#include "fusion.h"
#include "sensor.h"
#include "rtos_core.h"

static const char *TAG = "FUSION";

fusion_stats_t fusion_stats = {0};
QueueHandle_t fusion_output_queue = NULL;

void fusion_task(void *pvParameters){
    sensor_data_t sensor_a_data, sensor_b_data;
    fusion_data_t fusion_output_data;
    TickType_t timeout_ticks = pdMS_TO_TICKS(50);//convert ms to ticks as FREERTOS reads ticks only, timeout of 50ms
    ESP_LOGI(TAG, "Fusion Task commenced");
    uint64_t last_wake_time = rtos_get_time_us();
    
    while(1){
        uint64_t cycle_start = rtos_get_time_us();
        bool got_sensor_a = false, got_sensor_b = false;
        //try to receive data from sensor a
        if(xQueueReceive(sensor_a_queue, &sensor_a_data, timeout_ticks) == pdTRUE){
            got_sensor_a = true;
        }
        else{
            fusion_stats.sensor_a_timeouts++;
        }
        if(xQueueReceive(sensor_b_queue, &sensor_b_data, timeout_ticks) == pdTRUE){
            got_sensor_b = true;
        }
        else{
            fusion_stats.sensor_b_timeouts++;
        }

        //to perform fusion
        float fused_value = 0.0f;
        if(got_sensor_a && got_sensor_b){
            fused_value = (sensor_a_data.value*0.6f + 0.4f*sensor_b_data.value);
        }
        else if(got_sensor_a){
            fused_value = sensor_a_data.value;
            ESP_LOGI(TAG, "Only sensor a data");
        }
        else if(got_sensor_b){
            fused_value = sensor_b_data.value;
            ESP_LOGI(TAG, "Only sensor b data");
        }
        else{
            fused_value = fusion_stats.last_fusion_value;
            ESP_LOGI(TAG, "No sensor data gathered");
        }

        //output data
        fusion_output_data.timestamp_us = cycle_start;
        fusion_output_data.fusion_value = fused_value;
        fusion_output_data.control_value = 0.0f;
        //we need to send data to queue
        if(xQueueSend(fusion_output_queue, &fusion_output_data, 0) == pdTRUE){
            ESP_LOGW(TAG, "Warning Queue is full");
        }

        //statictics
        fusion_stats.fusion_cycles++;
        fusion_stats.last_fusion_value = fused_value;
        uint64_t execution_time = (rtos_get_time_us() - cycle_start);
        if(execution_time > fusion_stats.maximum_execution_time){
            fusion_stats.maximum_execution_time = execution_time;
        }
        //log every 100 cycles
        if(fusion_stats.fusion_cycles % 100 == 0){
            LOGI(TAG, " Cycles = %lu, Sensor A timeouts = %lu, Sensor B timeouts = %lu, Max execution time = %llu, last fused value = %lu", fusion_stats.fusion_cycles,fusion_stats.sensor_a_timeouts, fusion_stats.sensor_b_timeouts, fusion_stats.maximum_execution_time, fusion_stats.last_fusion_value);
        }
        delay_until_us(&last_wake_time, FUSION_PERIOD_US);
    }
}

esp_err_t fusion_init(void){
    ESP_LOGI(TAG, "Initializing the fusion task");
    //create queue
    fusion_output_queue = xQueueCreate(5, sizeof(fusion_data_t));
    if(!(fusion_output_queue)){
        ESP_LOGE(TAG, "failed to create queue");
        return ESP_FAIL;
    }
    //create the Task now 
    xTaskCreate(fusion_task, "FUSION_TASK", FUSION_TASK_STACK_SIZE, NULL, FUSION_TASK_PRIORITY, NULL, &fusion_task_handle);
    ESP_LOGI(TAG, "Fusion task is initialized");
    return ESP_OK;
}

