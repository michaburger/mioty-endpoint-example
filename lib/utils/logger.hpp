/**
 * @file logger.hpp
 * @brief Simple logging utility for debugging and monitoring
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "pico/stdlib.h"
#include <stdio.h>
#include <cstdarg>

/**
 * @brief Logging levels
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

/**
 * @brief Simple logger class for debugging output
 */
class Logger {
public:
    /**
     * @brief Set the minimum log level to display
     */
    static void setLogLevel(LogLevel level);
    
    /**
     * @brief Log a debug message
     */
    static void debug(const char* format, ...);
    
    /**
     * @brief Log an info message
     */
    static void info(const char* format, ...);
    
    /**
     * @brief Log a warning message
     */
    static void warning(const char* format, ...);
    
    /**
     * @brief Log an error message
     */
    static void error(const char* format, ...);

private:
    static LogLevel s_current_level;
    
    /**
     * @brief Internal logging function
     */
    static void log(LogLevel level, const char* prefix, const char* format, va_list args);
};
