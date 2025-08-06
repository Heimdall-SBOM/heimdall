/*
 * Heimdall Compiler Plugin CMake Example - Network Server Implementation
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Complete implementation of the multi-threaded network server with
 * connection management, message processing, and statistics collection.
 */

#include "Server.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>
#include <random>

NetworkServer::NetworkServer(int port, int max_clients) 
    : port_(port), max_clients_(max_clients), running_(false), 
      message_count_(0), total_bytes_received_(0) {
    // Reserve space for connections to avoid frequent reallocations
    connections_.reserve(max_clients);
}

NetworkServer::~NetworkServer() {
    if (running_.load()) {
        stop();
    }
}

bool NetworkServer::start() {
    if (running_.load()) {
        return true; // Already running
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::INFO, "Starting server on port " + std::to_string(port_));
    
    try {
        running_.store(true);
        
        // Start worker threads for message processing
        for (size_t i = 0; i < config_.worker_thread_count; ++i) {
            worker_threads_.emplace_back(&NetworkServer::processMessages, this);
        }
        
        // Start connection acceptance thread
        accept_thread_ = std::thread(&NetworkServer::acceptConnections, this);
        
        // Start cleanup thread
        cleanup_thread_ = std::thread(&NetworkServer::cleanupConnections, this);
        
        logger.log(LogLevel::INFO, "Server started with " + std::to_string(config_.worker_thread_count) + " worker threads");
        return true;
        
    } catch (const std::exception& e) {
        logger.log(LogLevel::ERROR, "Failed to start server: " + std::string(e.what()));
        running_.store(false);
        return false;
    }
}

void NetworkServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::INFO, "Stopping server...");
    
    // Signal shutdown
    running_.store(false);
    
    // Notify all waiting threads
    queue_condition_.notify_all();
    
    // Join worker threads
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    // Join other threads
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
    
    // Clear connections
    {
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_.clear();
    }
    
    // Clear message queue
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!message_queue_.empty()) {
            message_queue_.pop();
        }
    }
    
    logger.log(LogLevel::INFO, "Server stopped successfully");
}

size_t NetworkServer::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    return connections_.size();
}

std::string NetworkServer::getStatistics() const {
    std::ostringstream stats;
    stats << "Server Statistics:\n";
    stats << "  Port: " << port_ << "\n";
    stats << "  Active Connections: " << getActiveConnections() << "/" << max_clients_ << "\n";
    stats << "  Messages Processed: " << message_count_.load() << "\n";
    stats << "  Total Bytes Received: " << total_bytes_received_.load() << "\n";
    stats << "  Worker Threads: " << config_.worker_thread_count << "\n";
    stats << "  Running: " << (running_.load() ? "Yes" : "No");
    
    return stats.str();
}

bool NetworkServer::sendMessage(int client_id, const std::string& message) {
    auto& logger = Logger::getInstance();
    
    // Simulate sending message to client
    logger.log(LogLevel::DEBUG, "Sending message to client " + std::to_string(client_id) + ": " + message.substr(0, 50) + "...");
    
    // In a real implementation, this would send data over a socket
    // For the demo, we just log the action
    return true;
}

size_t NetworkServer::broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Broadcasting message to " + std::to_string(connections_.size()) + " clients");
    
    for (const auto& connection : connections_) {
        sendMessage(connection->client_id, message);
    }
    
    return connections_.size();
}

std::shared_ptr<const ConnectionInfo> NetworkServer::getConnectionInfo(int client_id) const {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = std::find_if(connections_.begin(), connections_.end(),
        [client_id](const std::unique_ptr<ConnectionInfo>& conn) {
            return conn->client_id == client_id;
        });
    
    if (it != connections_.end()) {
        return std::shared_ptr<const ConnectionInfo>(it->get(), [](const ConnectionInfo*) {});
    }
    
    return nullptr;
}

void NetworkServer::setWorkerThreadCount(size_t worker_threads) {
    config_.worker_thread_count = worker_threads;
}

void NetworkServer::setMaxMessageSize(size_t max_size) {
    config_.max_message_size = max_size;
}

void NetworkServer::acceptConnections() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Connection acceptance thread started");
    
    int client_counter = 0;
    
    while (running_.load()) {
        // Simulate accepting a new connection every 200ms
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        if (!running_.load()) break;
        
        // Check if we can accept more connections
        if (getActiveConnections() >= static_cast<size_t>(max_clients_)) {
            continue;
        }
        
        // Simulate incoming connection (in real implementation, this would be accept())
        int client_id = generateClientId();
        std::string client_address = "192.168.1." + std::to_string(100 + (client_counter % 50));
        
        if (addConnection(client_id, client_address)) {
            logger.log(LogLevel::DEBUG, "Accepted connection from " + client_address + " (ID: " + std::to_string(client_id) + ")");
            
            // Handle client in separate thread (simplified for demo)
            std::thread client_thread(&NetworkServer::handleClient, this, client_id, client_address);
            client_thread.detach();
        }
        
        client_counter++;
        
        // Limit simulation rate
        if (client_counter > 3) break; // For demo, only accept a few connections
    }
    
    logger.log(LogLevel::DEBUG, "Connection acceptance thread finished");
}

void NetworkServer::processMessages() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Message processing thread started");
    
    while (running_.load()) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Wait for messages or shutdown signal
        queue_condition_.wait(lock, [this] { 
            return !message_queue_.empty() || !running_.load(); 
        });
        
        if (!running_.load()) break;
        
        // Process all available messages
        while (!message_queue_.empty()) {
            auto message = std::move(message_queue_.front());
            message_queue_.pop();
            
            lock.unlock();
            handleMessage(std::move(message));
            lock.lock();
        }
    }
    
    logger.log(LogLevel::DEBUG, "Message processing thread finished");
}

void NetworkServer::cleanupConnections() {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Connection cleanup thread started");
    
    while (running_.load()) {
        std::this_thread::sleep_for(config_.cleanup_interval);
        
        if (!running_.load()) break;
        
        // Clean up expired connections (simplified for demo)
        std::lock_guard<std::mutex> lock(connections_mutex_);
        auto now = std::chrono::steady_clock::now();
        
        auto it = std::remove_if(connections_.begin(), connections_.end(),
            [now, this](const std::unique_ptr<ConnectionInfo>& conn) {
                auto age = now - conn->connect_time;
                return age > std::chrono::seconds(10); // Remove connections older than 10s for demo
            });
        
        if (it != connections_.end()) {
            size_t removed = connections_.end() - it;
            connections_.erase(it, connections_.end());
            if (removed > 0) {
                logger.log(LogLevel::DEBUG, "Cleaned up " + std::to_string(removed) + " expired connections");
            }
        }
    }
    
    logger.log(LogLevel::DEBUG, "Connection cleanup thread finished");
}

void NetworkServer::handleClient(int client_id, const std::string& client_address) {
    auto& logger = Logger::getInstance();
    logger.log(LogLevel::DEBUG, "Handling client " + std::to_string(client_id) + " from " + client_address);
    
    // Simulate client sending messages
    std::vector<std::string> sample_messages = {
        "Hello server!",
        "Can you process this request?",
        "Here is some data: " + std::to_string(rand() % 10000),
        "Testing connection stability",
        "Goodbye!"
    };
    
    for (const auto& msg : sample_messages) {
        if (!running_.load()) break;
        
        // Create message and add to queue
        auto message = std::make_unique<ServerMessage>(client_id, msg);
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            message_queue_.push(std::move(message));
        }
        queue_condition_.notify_one();
        
        // Simulate message sending interval
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 200));
    }
}

bool NetworkServer::addConnection(int client_id, const std::string& client_address) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    if (connections_.size() >= static_cast<size_t>(max_clients_)) {
        return false;
    }
    
    connections_.push_back(std::make_unique<ConnectionInfo>(client_id, client_address));
    return true;
}

void NetworkServer::removeConnection(int client_id) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    
    auto it = std::find_if(connections_.begin(), connections_.end(),
        [client_id](const std::unique_ptr<ConnectionInfo>& conn) {
            return conn->client_id == client_id;
        });
    
    if (it != connections_.end()) {
        connections_.erase(it);
    }
}

void NetworkServer::handleMessage(std::unique_ptr<ServerMessage> message) {
    auto& logger = Logger::getInstance();
    
    // Update statistics
    message_count_.fetch_add(1);
    total_bytes_received_.fetch_add(message->content.size());
    
    // Process the message
    std::string response = simulateNetworkCommunication(message->client_id, message->content);
    
    // Log the interaction
    logger.log(LogLevel::DEBUG, "Client " + std::to_string(message->client_id) + 
               " sent: " + message->content.substr(0, 30) + "...");
    
    // Send response back to client
    sendMessage(message->client_id, response);
}

int NetworkServer::generateClientId() {
    static std::atomic<int> counter{1000};
    return counter.fetch_add(1);
}

std::string NetworkServer::simulateNetworkCommunication(int client_id, const std::string& message) {
    // Simulate processing and generate response
    std::ostringstream response;
    response << "Response to client " << client_id << ": ";
    
    if (message.find("Hello") != std::string::npos) {
        response << "Welcome to the server!";
    } else if (message.find("data") != std::string::npos) {
        response << "Data received and processed successfully";
    } else if (message.find("Goodbye") != std::string::npos) {
        response << "Connection closing gracefully";
    } else {
        response << "Message acknowledged: " << message.size() << " bytes";
    }
    
    return response.str();
}