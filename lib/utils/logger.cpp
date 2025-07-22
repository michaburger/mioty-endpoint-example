/**
 * @file logger.cpp
 * @brief Logger implementation
 * 
 * Copyright (c) 2025 mioty Alliance e.V.
 * Author: Micha Burger <micha.burger@mioty-alliance.com>
 * SPDX-License-Identifier: MIT
 * 
 * NOTE: This project incorporates the Fraunhofer TS-UNB-Lib, which has 
 * additional licensing restrictions. See LICENSE file for complete terms.
 */

#include "logger.hpp"
#include "pico/time.h"

LogLevel Logger::s_current_level = LogLevel::INFO;

void Logger::setLogLevel(LogLevel level) {
    s_current_level = level;
}

void Logger::debug(const char* format, ...) {
    if (s_current_level <= LogLevel::DEBUG) {
        va_list args;
        va_start(args, format);
        log(LogLevel::DEBUG, "[DEBUG]", format, args);
        va_end(args);
    }
}

void Logger::info(const char* format, ...) {
    if (s_current_level <= LogLevel::INFO) {
        va_list args;
        va_start(args, format);
        log(LogLevel::INFO, "[INFO] ", format, args);
        va_end(args);
    }
}

void Logger::warning(const char* format, ...) {
    if (s_current_level <= LogLevel::WARNING) {
        va_list args;
        va_start(args, format);
        log(LogLevel::WARNING, "[WARN] ", format, args);
        va_end(args);
    }
}

void Logger::error(const char* format, ...) {
    if (s_current_level <= LogLevel::ERROR) {
        va_list args;
        va_start(args, format);
        log(LogLevel::ERROR, "[ERROR]", format, args);
        va_end(args);
    }
}

void Logger::log(LogLevel level, const char* prefix, const char* format, va_list args) {
    // Get timestamp in milliseconds
    uint64_t time_ms = time_us_64() / 1000;
    
    // Print timestamp and prefix
    printf("[%llu] %s ", time_ms, prefix);
    
    // Print formatted message
    vprintf(format, args);
    printf("\r\n");
}
