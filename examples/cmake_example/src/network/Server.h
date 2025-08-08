/*
 * Heimdall Compiler Plugin CMake Example - Network Server Header
 * 
 * Copyright 2025 Heimdall Project
 * Licensed under Apache License 2.0
 * 
 * Network server implementation for the simulation demo, demonstrating
 * advanced C++ networking concepts and multi-threaded server design.
 */

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>

/**
 * @brief Network connection information
 */
struct ConnectionInfo {
    int client_id;
    std::string client_address;
    std::chrono::steady_clock::time_point connect_time;
    std::atomic<size_t> messages_received{0};
    std::atomic<size_t> bytes_received{0};
    
    ConnectionInfo(int id, const std::string& addr) 
        : client_id(id), client_address(addr), connect_time(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Message structure for server communication
 */
struct ServerMessage {
    int client_id;
    std::string content;
    std::chrono::steady_clock::time_point timestamp;
    
    ServerMessage(int id, const std::string& msg) 
        : client_id(id), content(msg), timestamp(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Multi-threaded network server
 * 
 * This class demonstrates advanced C++ server design including:
 * - Multi-threaded client handling
 * - Thread-safe message queuing and processing
 * - Connection management with automatic cleanup
 * - Statistics collection and performance monitoring
 * - Graceful shutdown with proper resource management
 * - Template-based message handling for extensibility
 */
class NetworkServer {
private:
    const int port_;                                          ///< Server port
    const int max_clients_;                                   ///< Maximum concurrent clients
    std::atomic<bool> running_;                               ///< Server running state
    std::atomic<size_t> message_count_;                       ///< Total messages processed
    std::atomic<size_t> total_bytes_received_;                ///< Total bytes received
    
    // Connection management
    std::vector<std::unique_ptr<ConnectionInfo>> connections_; ///< Active connections
    mutable std::mutex connections_mutex_;                    ///< Connections list mutex
    
    // Message queue system
    std::queue<std::unique_ptr<ServerMessage>> message_queue_; ///< Incoming message queue
    std::mutex queue_mutex_;                                  ///< Queue access mutex
    std::condition_variable queue_condition_;                 ///< Queue notification
    
    // Worker threads
    std::vector<std::thread> worker_threads_;                 ///< Message processing threads
    std::thread accept_thread_;                              ///< Connection acceptance thread
    std::thread cleanup_thread_;                             ///< Connection cleanup thread
    
    // Configuration
    struct ServerConfig {
        size_t worker_thread_count = 2;
        size_t max_message_size = 4096;
        std::chrono::seconds connection_timeout{300};
        std::chrono::milliseconds cleanup_interval{1000};
        bool enable_statistics = true;
    };
    
    ServerConfig config_;                                     ///< Server configuration

public:
    /**
     * @brief Constructor
     * @param port Server port to listen on
     * @param max_clients Maximum number of concurrent clients
     */
    NetworkServer(int port, int max_clients = 10);
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~NetworkServer();
    
    // Non-copyable but moveable
    NetworkServer(const NetworkServer&) = delete;
    NetworkServer& operator=(const NetworkServer&) = delete;
    NetworkServer(NetworkServer&&) = default;
    NetworkServer& operator=(NetworkServer&&) = default;
    
    /**
     * @brief Start the server
     * @return true if server started successfully
     */
    bool start();
    
    /**
     * @brief Stop the server gracefully
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if server is running
     */
    bool isRunning() const { return running_.load(); }
    
    /**
     * @brief Get number of active connections
     * @return Current number of active connections
     */
    size_t getActiveConnections() const;
    
    /**
     * @brief Get total message count
     * @return Total messages processed since server start
     */
    size_t getMessageCount() const { return message_count_.load(); }
    
    /**
     * @brief Get total bytes received
     * @return Total bytes received from all clients
     */
    size_t getTotalBytesReceived() const { return total_bytes_received_.load(); }
    
    /**
     * @brief Get server statistics
     * @return Formatted statistics string
     */
    std::string getStatistics() const;
    
    /**
     * @brief Send message to specific client
     * @param client_id Target client ID
     * @param message Message to send
     * @return true if message was queued for sending
     */
    bool sendMessage(int client_id, const std::string& message);
    
    /**
     * @brief Broadcast message to all connected clients
     * @param message Message to broadcast
     * @return Number of clients message was sent to
     */
    size_t broadcastMessage(const std::string& message);
    
    /**
     * @brief Get connection information for a client
     * @param client_id Client ID to look up
     * @return Pointer to connection info, or nullptr if not found
     */
    std::shared_ptr<const ConnectionInfo> getConnectionInfo(int client_id) const;
    
    /**
     * @brief Set server configuration
     * @param worker_threads Number of worker threads for message processing
     */
    void setWorkerThreadCount(size_t worker_threads);
    
    /**
     * @brief Set maximum message size
     * @param max_size Maximum message size in bytes
     */
    void setMaxMessageSize(size_t max_size);

private:
    /**
     * @brief Accept incoming connections (runs in separate thread)
     */
    void acceptConnections();
    
    /**
     * @brief Process messages from queue (runs in worker threads)
     */
    void processMessages();
    
    /**
     * @brief Clean up expired connections (runs in separate thread)
     */
    void cleanupConnections();
    
    /**
     * @brief Handle individual client connection
     * @param client_id Unique client identifier
     * @param client_address Client IP address
     */
    void handleClient(int client_id, const std::string& client_address);
    
    /**
     * @brief Add new connection to the server
     * @param client_id Client ID
     * @param client_address Client address
     * @return true if connection was added successfully
     */
    bool addConnection(int client_id, const std::string& client_address);
    
    /**
     * @brief Remove connection from the server
     * @param client_id Client ID to remove
     */
    void removeConnection(int client_id);
    
    /**
     * @brief Process a single message
     * @param message Message to process
     */
    void handleMessage(std::unique_ptr<ServerMessage> message);
    
    /**
     * @brief Generate unique client ID
     * @return New unique client ID
     */
    int generateClientId();
    
    /**
     * @brief Simulate network communication
     * @param client_id Client to simulate communication with
     * @param message Message content
     * @return Response message
     */
    std::string simulateNetworkCommunication(int client_id, const std::string& message);
};

#endif // NETWORK_SERVER_H