/*
 * Heimdall Compiler Plugin CMake Example - Network Client Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Complete implementation of the network client with asynchronous
 * communication, reconnection logic, and statistics collection.
 */

#include "Client.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>
#include <random>

NetworkClient::NetworkClient(const std::string& server_address, int server_port)
    : server_address_(server_address), server_port_(server_port), 
      state_(ConnectionState::DISCONNECTED), should_reconnect_(true),
      messages_sent_(0), messages_received_(0), bytes_sent_(0), bytes_received_(0),
      reconnect_attempts_(0), running_(false) {
    
    connect_time_ = std::chrono::steady_clock::now();
    last_activity_ = connect_time_;
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect() {
    if (state_.load() == ConnectionState::CONNECTED) {
        return true; // Already connected
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::INFO, "Connecting to " + server_address_ + ":" + std::to_string(server_port_));
    
    running_.store(true);
    state_.store(ConnectionState::CONNECTING);
    
    // Start worker threads
    connection_thread_ = std::thread(&NetworkClient::connectionLoop, this);
    send_thread_ = std::thread(&NetworkClient::sendLoop, this);
    receive_thread_ = std::thread(&NetworkClient::receiveLoop, this);
    
    // Wait a moment for connection attempt
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    return state_.load() != ConnectionState::ERROR;
}

void NetworkClient::disconnect() {
    if (state_.load() == ConnectionState::DISCONNECTED) {
        return;
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::INFO, "Disconnecting from server");
    
    // Signal shutdown
    running_.store(false);
    should_reconnect_.store(false);
    state_.store(ConnectionState::DISCONNECTED);
    
    // Notify all waiting threads
    send_condition_.notify_all();
    receive_condition_.notify_all();
    
    // Join threads
    if (connection_thread_.joinable()) {
        connection_thread_.join();
    }
    if (send_thread_.joinable()) {
        send_thread_.join();
    }
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    
    // Clear queues
    {
        std::lock_guard<std::mutex> lock(send_mutex_);
        while (!send_queue_.empty()) {
            send_queue_.pop();
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(receive_mutex_);
        while (!receive_queue_.empty()) {
            receive_queue_.pop();
        }
    }
    
    logger.log(LogLevel::INFO, "Client disconnected");
}

bool NetworkClient::sendMessage(const std::string& message) {
    if (state_.load() != ConnectionState::CONNECTED) {
        return false;
    }
    
    if (message.size() > config_.max_message_size) {
        auto& logger = Logger::getInstance();
        logger.log(LogLevel::WARNING, "Message too large: " + std::to_string(message.size()) + " bytes");
        return false;
    }
    
    {
        std::lock_guard<std::mutex> lock(send_mutex_);
        
        if (send_queue_.size() >= config_.max_queue_size) {
            return false; // Queue full
        }
        
        send_queue_.push(std::make_unique<ClientMessage>(message));
    }
    
    send_condition_.notify_one();
    return true;
}

std::string NetworkClient::receiveMessage(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(receive_mutex_);
    
    if (receive_condition_.wait_for(lock, timeout, [this] { return !receive_queue_.empty(); })) {
        auto message = std::move(receive_queue_.front());
        receive_queue_.pop();
        return message->content;
    }
    
    return ""; // Timeout
}

double NetworkClient::getConnectionUptime() const {
    if (state_.load() != ConnectionState::CONNECTED) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = now - connect_time_;
    return std::chrono::duration<double>(duration).count();
}

std::string NetworkClient::getStatistics() const {
    std::ostringstream stats;
    stats << "Client Statistics:\n";
    stats << "  Server: " << server_address_ << ":" << server_port_ << "\n";
    stats << "  State: " << stateToString(state_.load()) << "\n";
    stats << "  Uptime: " << getConnectionUptime() << " seconds\n";
    stats << "  Messages Sent: " << messages_sent_.load() << "\n";
    stats << "  Messages Received: " << messages_received_.load() << "\n";
    stats << "  Bytes Sent: " << bytes_sent_.load() << "\n";
    stats << "  Bytes Received: " << bytes_received_.load() << "\n";
    stats << "  Reconnect Attempts: " << reconnect_attempts_.load();
    
    return stats.str();
}

void NetworkClient::setConnectTimeout(std::chrono::milliseconds timeout) {
    config_.connect_timeout = timeout;
}

void NetworkClient::setMaxReconnectAttempts(int max_attempts) {
    config_.max_reconnect_attempts = max_attempts;
}

void NetworkClient::connectionLoop() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Connection loop started");
    
    while (running_.load()) {
        ConnectionState current_state = state_.load();
        
        switch (current_state) {
            case ConnectionState::CONNECTING:
                if (attemptConnection()) {
                    state_.store(ConnectionState::CONNECTED);
                    connect_time_ = std::chrono::steady_clock::now();
                    updateLastActivity();
                    reconnect_attempts_.store(0);
                    logger.log(LogLevel::INFO, "Connected to server successfully");
                } else {
                    if (should_reconnect_.load()) {
                        handleConnectionLoss();
                    } else {
                        state_.store(ConnectionState::ERROR);
                    }
                }
                break;
                
            case ConnectionState::CONNECTED:
                // Check connection health
                if (!isConnectionActive()) {
                    logger.log(LogLevel::WARNING, "Connection appears inactive");
                    handleConnectionLoss();
                }
                std::this_thread::sleep_for(config_.keepalive_interval);
                break;
                
            case ConnectionState::RECONNECTING:
                if (attemptConnection()) {
                    state_.store(ConnectionState::CONNECTED);
                    connect_time_ = std::chrono::steady_clock::now();
                    updateLastActivity();
                    logger.log(LogLevel::INFO, "Reconnected to server");
                } else {
                    handleConnectionLoss();
                }
                break;
                
            case ConnectionState::DISCONNECTED:
            case ConnectionState::ERROR:
                // Wait for external connect() call
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
        }
    }
    
    logger.log(LogLevel::DEBUG, "Connection loop finished");
}

void NetworkClient::sendLoop() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Send loop started");
    
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(send_mutex_);
        
        send_condition_.wait(lock, [this] {
            return !send_queue_.empty() || !running_.load();
        });
        
        if (!running_.load()) break;
        
        while (!send_queue_.empty() && state_.load() == ConnectionState::CONNECTED) {
            auto message = std::move(send_queue_.front());
            send_queue_.pop();
            
            lock.unlock();
            
            if (sendSingleMessage(message->content)) {
                messages_sent_.fetch_add(1);
                bytes_sent_.fetch_add(message->content.size());
                updateLastActivity();
            }
            
            lock.lock();
        }
    }
    
    logger.log(LogLevel::DEBUG, "Send loop finished");
}

void NetworkClient::receiveLoop() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Receive loop started");
    
    while (running_.load()) {
        if (state_.load() != ConnectionState::CONNECTED) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        std::string received = receiveSingleMessage();
        if (!received.empty()) {
            messages_received_.fetch_add(1);
            bytes_received_.fetch_add(received.size());
            updateLastActivity();
            
            {
                std::lock_guard<std::mutex> lock(receive_mutex_);
                if (receive_queue_.size() < config_.max_queue_size) {
                    receive_queue_.push(std::make_unique<ClientMessage>(received, true));
                }
            }
            receive_condition_.notify_one();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    logger.log(LogLevel::DEBUG, "Receive loop finished");
}

bool NetworkClient::attemptConnection() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Attempting connection to " + server_address_ + ":" + std::to_string(server_port_));
    
    // Simulate connection attempt
    std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 200));
    
    // For demo purposes, simulate occasional connection failures
    if (rand() % 10 == 0) { // 10% chance of connection failure
        logger.log(LogLevel::DEBUG, "Connection attempt failed (simulated)");
        return false;
    }
    
    logger.log(LogLevel::DEBUG, "Connection established successfully");
    return true;
}

void NetworkClient::handleConnectionLoss() {
    auto& logger = Logger::getInstance();
    
    int current_attempts = reconnect_attempts_.fetch_add(1);
    
    if (current_attempts >= config_.max_reconnect_attempts) {
        logger.log(LogLevel::ERROR, "Max reconnection attempts reached");
        state_.store(ConnectionState::ERROR);
        return;
    }
    
    auto delay = calculateReconnectDelay(current_attempts);
    logger.log(LogLevel::INFO, "Reconnecting in " + std::to_string(delay.count()) + "ms (attempt " + 
               std::to_string(current_attempts + 1) + "/" + std::to_string(config_.max_reconnect_attempts) + ")");
    
    state_.store(ConnectionState::RECONNECTING);
    std::this_thread::sleep_for(delay);
}

bool NetworkClient::sendSingleMessage(const std::string& message) {
    auto& logger = Logger::getInstance();
    
    // Simulate sending message over network
    logger.log(LogLevel::DEBUG, "Sending message: " + message.substr(0, 30) + "...");
    
    // Simulate network delay and occasional failures
    std::this_thread::sleep_for(std::chrono::milliseconds(10 + rand() % 50));
    
    if (rand() % 50 == 0) { // 2% chance of send failure
        logger.log(LogLevel::DEBUG, "Message send failed (simulated network error)");
        return false;
    }
    
    return true;
}

std::string NetworkClient::receiveSingleMessage() {
    // Simulate receiving message from server
    static std::vector<std::string> responses = {
        "Response to your message",
        "Data processed successfully", 
        "Server acknowledgment",
        "Thank you for your request",
        "Processing complete"
    };
    
    // Simulate occasional incoming messages
    if (rand() % 20 == 0) { // 5% chance of receiving message
        return responses[rand() % responses.size()];
    }
    
    return "";
}

std::chrono::milliseconds NetworkClient::calculateReconnectDelay(int attempt_number) const {
    // Exponential backoff with jitter
    auto base_delay = config_.base_reconnect_delay.count();
    auto exponential_delay = base_delay * (1 << std::min(attempt_number, 6)); // Cap at 2^6
    auto jitter = rand() % (exponential_delay / 4 + 1); // Add up to 25% jitter
    
    return std::chrono::milliseconds(exponential_delay + jitter);
}

std::string NetworkClient::stateToString(ConnectionState state) {
    switch (state) {
        case ConnectionState::DISCONNECTED: return "DISCONNECTED";
        case ConnectionState::CONNECTING:   return "CONNECTING";
        case ConnectionState::CONNECTED:    return "CONNECTED";
        case ConnectionState::RECONNECTING: return "RECONNECTING";
        case ConnectionState::ERROR:        return "ERROR";
        default:                           return "UNKNOWN";
    }
}

void NetworkClient::updateLastActivity() {
    last_activity_ = std::chrono::steady_clock::now();
}

bool NetworkClient::isConnectionActive() const {
    auto now = std::chrono::steady_clock::now();
    auto inactive_duration = now - last_activity_;
    
    // Consider connection inactive if no activity for 2 minutes
    return inactive_duration < std::chrono::minutes(2);
}