#include "../include/ulog.hpp"

int main() {
    // Configure the logging system
    ulog::Log::enable_colors(true);
    ulog::Log::enable_timestamps(true);
    ulog::Log::set_level(ulog::Level::debug);

    // Simple static logging
    ulog::Log::info("Hello from static ulog interface!");
    ulog::Log::debug("Debug value: %d", 42);
    ulog::Log::warn("Warning: %s", "Something might be wrong");
    ulog::Log::error("Error code: %d", 404);

    // Topic-based logging
    ulog::Log::add_topic("NETWORK");
    ulog::Log::add_topic("DATABASE");
    
    ulog::Log::info("NETWORK", "Connected to server");
    ulog::Log::debug("DATABASE", "Query completed");

    // Instance-based logger with topic
    ulog::Logger network_logger("NETWORK", ulog::Level::info);
    ulog::Logger general_logger; // No topic
    
    network_logger.info("Connection established");
    network_logger.debug("This won't show (level too low)");
    
    general_logger.trace("Application trace");
    general_logger.fatal("Critical error");

    return 0;
}
