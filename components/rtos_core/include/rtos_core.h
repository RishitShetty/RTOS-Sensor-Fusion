#ifndef RTOS_CORE_H
#define RTOS_CORE_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
//define priorities

#define CONTROL_TASK_PRIORITY 10
#define SENSOR_A_TASK_PRIORITY 7
#define SENSOR_B_TASK_PRIORITY 7
#define FUSION_TASK_PRIORITY 6
#define LOGGER_TASK_PRIORITY 4
#define CLI_TASK_PRIORITY 2
#define HEALTH_TASK_PRIORITY 3

//DEFIINE stack size 

#define CONTROL_TASK_STACK_SIZE 4096
#define SENSOR_A_TASK_STACK_SIZE 2048
#define SENSOR_B_TASK_STACK_SIZE 2048
#define FUSION_TASK_STACK_SIZE 4096
#define LOGGER_TASK_STACK_SIZE 3072
#define CLI_TASK_STACK_SIZE 2048
#define HEALTH_TASK_STACK_SIZE 2048

//Define timing 
//Control: 1-10kHz
//Sensor acquisition: 100Hz-10kHz
//User interfaces: 10-60Hz
//Logging/telemetry: 1-10Hz
//Diagnostics: 0.1-1Hz

#define CONTROL_PERIOD_US 1000
#define SENSOR_A_PERIOD_US 1000
#define SENSOR_B_PERIOD_US 2500
#define FUSION_PERIOD_US 10000
#define LOGGER_PERIOD_US 100000
#define CLI_PERIOD_US 50000
#define HEALTH_PERIOD_US 100000

typedef struct {
    uint64_t timestamp_us;
    float value;
    uint8_t sensor_id;
} sensor_data_t;

typedef struct{
    uint64_t timestamp_us;
    float fusion_value;
    float control_value;
} fusion_data_t;

// initialize functions

esp_err_t rtos_core_init(void);

uint64_t rtos_get_time_us(void);

void delay_until_us(uint64_t *last_wake_time, uint64_t period_us);

#endif 