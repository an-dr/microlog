#include <iostream>
#include <fstream>
#include "../include/ulog.hpp"

int main() {
    // Configure logging
    ulog::Log::enable_colors(true);
    ulog::Log::enable_timestamps(true);
    ulog::Log::enable_source_location(true);
    ulog::Log::set_level(ulog::Level::debug);

    // Basic logging without topics
    ulog::Log::info("Application started");
    ulog::Log::debug("Debug information: value = %d", 42);
    ulog::Log::warn("This is a warning message");
    ulog::Log::error("Error occurred: %s", "File not found");

    // Topic-based logging
    ulog::Log::add_topic("NETWORK", true);
    ulog::Log::add_topic("DATABASE", true);
    
    ulog::Log::info("NETWORK", "Connected to server %s:%d", "localhost", 8080);
    ulog::Log::debug("DATABASE", "Query executed in %d ms", 150);
    ulog::Log::warn("NETWORK", "Connection timeout, retrying...");
    
    // Disable a specific topic
    ulog::Log::disable_topic("DATABASE");
    ulog::Log::info("DATABASE", "This message won't be displayed");
    
    // Re-enable and set different level
    ulog::Log::enable_topic("DATABASE");
    ulog::Log::set_topic_level("DATABASE", ulog::Level::warn);
    ulog::Log::info("DATABASE", "This info message won't be displayed");
    ulog::Log::error("DATABASE", "But this error will be displayed");

    // Using instance-based logger
    ulog::Logger network_logger("NETWORK", ulog::Level::info);
    ulog::Logger app_logger; // No topic, default level
    
    network_logger.info("Instance logger: Connection established");
    network_logger.debug("This debug message won't show (level is info)");
    
    app_logger.trace("Application trace message");
    app_logger.fatal("Critical error in application");

    // Add file output
    FILE* log_file = fopen("application.log", "w");
    if (log_file) {
        int file_output = ulog::Log::add_file_output(log_file, ulog::Level::info);
        
        ulog::Log::info("This message goes to both console and file");
        ulog::Log::debug("This debug message only goes to console");
        
        // Remove file output when done
        ulog::Log::remove_output(file_output);
        fclose(log_file);
    }

    ulog::Log::info("Application finished");
    
    return 0;
}
