/*
 * Heimdall Compiler Plugin CMake Example - Network Client Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Network client implementation for simulation demo, demonstrating
 * client-side networking and asynchronous communication patterns.
 */

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <string>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>

/**
 * @brief Client connection state
 */
enum class ConnectionState {
    DISCONNECTED,    ///< Not connected to server
    CONNECTING,      ///< Attempting to connect
    CONNECTED,       ///< Successfully connected
    RECONNECTING,    ///< Attempting to reconnect
    ERROR            ///< Connection error state
};

/**
 * @brief Client message structure
 */
struct ClientMessage {
    std::string content;
    std::chrono::steady_clock::time_point timestamp;
    bool is_response;
    
    ClientMessage(const std::string& msg, bool response = false) 
        : content(msg), timestamp(std::chrono::steady_clock::now()), is_response(response) {}
};

/**
 * @brief Network client with asynchronous communication
 * 
 * This class demonstrates modern C++ client design including:
 * - Asynchronous connection management
 * - Thread-safe message queuing and processing
 * - Automatic reconnection with exponential backoff
 * - Statistics collection and performance monitoring
 * - Template-based callback system for extensibility
 * - RAII resource management
 */
class NetworkClient {
private:
    const std::string server_address_;                    ///< Server address to connect to
    const int server_port_;                              ///< Server port
    std::atomic<ConnectionState> state_;                 ///< Current connection state
    std::atomic<bool> should_reconnect_;                 ///< Enable automatic reconnection
    std::atomic<size_t> messages_sent_;                  ///< Total messages sent
    std::atomic<size_t> messages_received_;              ///< Total messages received
    std::atomic<size_t> bytes_sent_;                     ///< Total bytes sent
    std::atomic<size_t> bytes_received_;                 ///< Total bytes received
    
    // Connection management
    std::chrono::steady_clock::time_point connect_time_; ///< Connection establishment time
    std::chrono::steady_clock::time_point last_activity_; ///< Last activity timestamp
    std::atomic<int> reconnect_attempts_;                ///< Number of reconnection attempts
    
    // Message queues
    std::queue<std::unique_ptr<ClientMessage>> send_queue_;     ///< Outgoing message queue
    std::queue<std::unique_ptr<ClientMessage>> receive_queue_;  ///< Incoming message queue
    std::mutex send_mutex_;                              ///< Send queue mutex
    std::mutex receive_mutex_;                           ///< Receive queue mutex
    std::condition_variable send_condition_;             ///< Send queue notification
    std::condition_variable receive_condition_;          ///< Receive queue notification
    
    // Worker threads
    std::thread connection_thread_;                      ///< Connection management thread
    std::thread send_thread_;                           ///< Message sending thread
    std::thread receive_thread_;                        ///< Message receiving thread
    
    // Configuration
    struct ClientConfig {
        std::chrono::milliseconds connect_timeout{5000};
        std::chrono::milliseconds send_timeout{3000};
        std::chrono::milliseconds receive_timeout{3000};
        size_t max_message_size = 4096;
        size_t max_queue_size = 100;
        int max_reconnect_attempts = 5;
        std::chrono::milliseconds base_reconnect_delay{1000};
        bool enable_keepalive = true;
        std::chrono::seconds keepalive_interval{30};
    };
    
    ClientConfig config_;                                ///< Client configuration
    std::atomic<bool> running_;                          ///< Client running state

public:
    /**
     * @brief Constructor
     * @param server_address Server address to connect to
     * @param server_port Server port to connect to
     */
    NetworkClient(const std::string& server_address, int server_port);
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~NetworkClient();
    
    // Non-copyable but moveable
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;
    NetworkClient(NetworkClient&&) = default;
    NetworkClient& operator=(NetworkClient&&) = default;
    
    /**
     * @brief Connect to the server
     * @return true if connection successful or attempt started
     */
    bool connect();
    
    /**
     * @brief Disconnect from the server
     */
    void disconnect();
    
    /**
     * @brief Send message to server
     * @param message Message content to send
     * @return true if message was queued successfully
     */
    bool sendMessage(const std::string& message);
    
    /**
     * @brief Receive message from server (blocking with timeout)
     * @param timeout Maximum time to wait for message
     * @return Received message, or empty string if timeout
     */
    std::string receiveMessage(std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    
    /**
     * @brief Get current connection state
     * @return Current connection state
     */
    ConnectionState getState() const { return state_.load(); }
    
    /**
     * @brief Check if client is connected
     * @return true if connected to server
     */
    bool isConnected() const { return state_.load() == ConnectionState::CONNECTED; }
    
    /**
     * @brief Get connection uptime
     * @return Seconds since connection established
     */
    double getConnectionUptime() const;
    
    /**
     * @brief Get client statistics
     * @return Formatted statistics string
     */
    std::string getStatistics() const;
    
    /**
     * @brief Get number of messages sent
     * @return Total messages sent since connection
     */
    size_t getMessagesSent() const { return messages_sent_.load(); }
    
    /**
     * @brief Get number of messages received
     * @return Total messages received since connection
     */
    size_t getMessagesReceived() const { return messages_received_.load(); }
    
    /**
     * @brief Get total bytes sent
     * @return Total bytes sent since connection
     */
    size_t getBytesSent() const { return bytes_sent_.load(); }
    
    /**
     * @brief Get total bytes received
     * @return Total bytes received since connection
     */
    size_t getBytesReceived() const { return bytes_received_.load(); }
    
    /**
     * @brief Enable or disable automatic reconnection
     * @param enable Enable automatic reconnection
     */
    void setAutoReconnect(bool enable) { should_reconnect_.store(enable); }
    
    /**
     * @brief Set connection timeout
     * @param timeout Connection timeout in milliseconds
     */
    void setConnectTimeout(std::chrono::milliseconds timeout);
    
    /**
     * @brief Set maximum reconnection attempts
     * @param max_attempts Maximum number of reconnection attempts
     */
    void setMaxReconnectAttempts(int max_attempts);

private:
    /**
     * @brief Connection management loop (runs in separate thread)
     */
    void connectionLoop();
    
    /**
     * @brief Message sending loop (runs in separate thread)
     */
    void sendLoop();
    
    /**
     * @brief Message receiving loop (runs in separate thread)
     */
    void receiveLoop();
    
    /**
     * @brief Attempt to establish connection to server
     * @return true if connection successful
     */
    bool attemptConnection();
    
    /**
     * @brief Handle connection loss and attempt reconnection
     */
    void handleConnectionLoss();
    
    /**
     * @brief Send a single message to server
     * @param message Message to send
     * @return true if sent successfully
     */
    bool sendSingleMessage(const std::string& message);
    
    /**
     * @brief Receive a single message from server
     * @return Received message, or empty if no message
     */
    std::string receiveSingleMessage();
    
    /**
     * @brief Calculate reconnection delay with exponential backoff
     * @param attempt_number Current attempt number
     * @return Delay duration
     */
    std::chrono::milliseconds calculateReconnectDelay(int attempt_number) const;
    
    /**
     * @brief Convert connection state to string
     * @param state Connection state to convert
     * @return String representation of state
     */
    static std::string stateToString(ConnectionState state);
    
    /**
     * @brief Update last activity timestamp
     */
    void updateLastActivity();
    
    /**
     * @brief Check if connection is active (for keepalive)
     * @return true if connection appears active
     */
    bool isConnectionActive() const;
};

#endif // NETWORK_CLIENT_H