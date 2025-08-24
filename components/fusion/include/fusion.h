#ifndef FUSION_H
#define FUSION_H

#include "rtos_core.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "sensor.h"

typedef struct{
    uint32_t fusion_cycles;
    uint32_t sensor_a_timeouts;
    uint32_t sensor_b_timeouts;
    uint64_t maximum_execution_time;
    uint32_t last_execution_time;
    float last_fusion_value;
} fusion_stats_t;
extern fusion_stats_t fusion_stats;
extern QueueHandle_t fusion_output_queue;

esp_err_t fusion_init(void);
void fusion_task(void *pvParameters);

#endif
