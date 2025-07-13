/**
 * @file mock_execution_logger.h
 * @brief Mock declarations for execution logger functions
 * @author Test System
 * @date 2025
 * 
 * This file provides mock declarations for the execution logger functions
 * that are called by the main code but not needed for testing.
 */

#ifndef MOCK_EXECUTION_LOGGER_H
#define MOCK_EXECUTION_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mock implementation for exec_logger_log_task_assigned
 * @param task_id ID of the task
 * @param elevator_id ID of the elevator
 * @param target_floor Target floor
 */
void exec_logger_log_task_assigned(const char* task_id, const char* elevator_id, int target_floor);

/**
 * @brief Mock implementation for exec_logger_log_elevator_moved
 * @param elevator_id ID of the elevator
 * @param from_floor Floor moved from
 * @param to_floor Floor moved to
 */
void exec_logger_log_elevator_moved(const char* elevator_id, int from_floor, int to_floor);

/**
 * @brief Mock implementation for exec_logger_log_task_completed
 * @param task_id ID of the completed task
 * @param elevator_id ID of the elevator
 * @param floor Final floor
 */
void exec_logger_log_task_completed(const char* task_id, const char* elevator_id, int floor);

/**
 * @brief Mock implementation for exec_logger_log_coap_sent
 * @param message Message sent
 */
void exec_logger_log_coap_sent(const char* message);

/**
 * @brief Mock implementation for exec_logger_log_coap_received
 * @param message Message received
 */
void exec_logger_log_coap_received(const char* message);

/**
 * @brief Mock implementation for exec_logger_log_floor_call
 * @param floor Floor number
 * @param direction Direction (UP/DOWN)
 */
void exec_logger_log_floor_call(int floor, const char* direction);

/**
 * @brief Mock implementation for exec_logger_log_cabin_request
 * @param elevator_id ID of the elevator
 * @param target_floor Target floor
 */
void exec_logger_log_cabin_request(const char* elevator_id, int target_floor);

/**
 * @brief Mock implementation for generic logging function
 * @param format Format string
 * @param ... Variable arguments
 */
void exec_logger_log(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // MOCK_EXECUTION_LOGGER_H 