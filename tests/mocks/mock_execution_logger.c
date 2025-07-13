/**
 * @file mock_execution_logger.c
 * @brief Mock implementation for execution logger functions
 * @author Test System
 * @date 2025
 * 
 * This file provides mock implementations for the execution logger functions
 * that are called by the main code but not needed for testing.
 */

#include <stdio.h>
#include <stdarg.h>
#include "mock_execution_logger.h"

/**
 * @brief Mock implementation for exec_logger_log_task_assigned
 * @param task_id ID of the task
 * @param elevator_id ID of the elevator
 * @param target_floor Target floor
 */
void exec_logger_log_task_assigned(const char* task_id, const char* elevator_id, int target_floor) {
    // Mock implementation - just print to console for debugging if needed
    printf("[MOCK] Task assigned: %s to elevator %s, target floor %d\n", 
           task_id ? task_id : "NULL", 
           elevator_id ? elevator_id : "NULL", 
           target_floor);
}

/**
 * @brief Mock implementation for exec_logger_log_elevator_moved
 * @param elevator_id ID of the elevator
 * @param from_floor Floor moved from
 * @param to_floor Floor moved to
 */
void exec_logger_log_elevator_moved(const char* elevator_id, int from_floor, int to_floor) {
    printf("[MOCK] Elevator moved: %s from floor %d to floor %d\n", 
           elevator_id ? elevator_id : "NULL", 
           from_floor, 
           to_floor);
}

/**
 * @brief Mock implementation for exec_logger_log_task_completed
 * @param task_id ID of the completed task
 * @param elevator_id ID of the elevator
 * @param floor Final floor
 */
void exec_logger_log_task_completed(const char* task_id, const char* elevator_id, int floor) {
    printf("[MOCK] Task completed: %s by elevator %s at floor %d\n", 
           task_id ? task_id : "NULL", 
           elevator_id ? elevator_id : "NULL", 
           floor);
}

/**
 * @brief Mock implementation for exec_logger_log_coap_sent
 * @param message Message sent
 */
void exec_logger_log_coap_sent(const char* message) {
    printf("[MOCK] CoAP sent: %s\n", message ? message : "NULL");
}

/**
 * @brief Mock implementation for exec_logger_log_coap_received
 * @param message Message received
 */
void exec_logger_log_coap_received(const char* message) {
    printf("[MOCK] CoAP received: %s\n", message ? message : "NULL");
}

/**
 * @brief Mock implementation for exec_logger_log_floor_call
 * @param floor Floor number
 * @param direction Direction (UP/DOWN)
 */
void exec_logger_log_floor_call(int floor, const char* direction) {
    printf("[MOCK] Floor call: floor %d, direction %s\n", 
           floor, 
           direction ? direction : "NULL");
}

/**
 * @brief Mock implementation for exec_logger_log_cabin_request
 * @param elevator_id ID of the elevator
 * @param target_floor Target floor
 */
void exec_logger_log_cabin_request(const char* elevator_id, int target_floor) {
    printf("[MOCK] Cabin request: elevator %s to floor %d\n", 
           elevator_id ? elevator_id : "NULL", 
           target_floor);
}

/**
 * @brief Mock implementation for generic logging function
 * @param format Format string
 * @param ... Variable arguments
 */
void exec_logger_log(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("[MOCK] ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
} 