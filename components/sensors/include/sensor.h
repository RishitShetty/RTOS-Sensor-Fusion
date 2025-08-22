#ifndef SENSOR_H
#define SENSOR_H

#include "rtos_core.h"
#include "freertos/semphr.h"

#define SENSOR_A_AMPLITUDE      100.0f
#define SENSOR_A_FREQUENCY      0.5f    // Hz
#define SENSOR_A_NOISE_LEVEL    5.0f

#define SENSOR_B_AMPLITUDE      200.0f
#define SENSOR_B_FREQUENCY      0.3f    // Hz
#define SENSOR_B_NOISE_LEVEL    8.0f
//queue size
#define SENSOR_QUEUE_SIZE 10
//define shared handles
extern QueueHandle_t sensor_a_queue;
extern QueueHandle_t sensor_b_queue;
//Timer handler

extern esp_timer_handle_t sensor_a_timer;
extern esp_timer_handle_t sensor_b_timer;

//semaphores
extern SemaphoreHandle_t sensor_a_sem;
extern SemaphoreHandle_t sensor_b_sem;

//structure

typedef struct{
    uint32_t total_samples;
    uint64_t min_jitter_us;
    uint64_t max_jitter_us;
    uint64_t total_jitter_us;
    uint32_t missed_deadlines;
    uint64_t last_execution_time_us;
} sensor_stats_t;

extern sensor_stats_t sensor_a_stats;
extern sensor_stats_t sensor_b_stats;
//declare functions
esp_err_t sensors_init(void);
void sensor_a_task(void *pvParameters);
void sensor_b_task(void *pvParameters);
void sensor_a_timer_callback(void* arg);
void sensor_b_timer_callback(void* arg);
float generate_noise(void);

#endif