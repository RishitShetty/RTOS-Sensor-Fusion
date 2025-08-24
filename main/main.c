/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "rtos_core.h"
#include "sensor.h"
#include "fusion.h"

TaskHandle_t control_task_handle = NULL;
TaskHandle_t fusion_task_handle = NULL;
TaskHandle_t cli_task_handle = NULL;

static const char *TAG = "MAIN";


//test to verify timing 
void test_timing_task(void *pvParameters){
    uint64_t last_wake_time = rtos_get_time_us();
    uint32_t counter = 0;
    ESP_LOGI("TEST", "STarting test timer task ");
    while(1){
        uint64_t current_time = rtos_get_time_us();
        uint64_t jitter = current_time - (last_wake_time+10000);
        ESP_LOGI("TEST", "Counter : %lu, Jitter : %lld", counter++, jitter);
        delay_until_us(&last_wake_time, 10000); //consistent wake up time of 10ms
        if(counter >=10){
            ESP_LOGI("TEST", "Test passed successfully");
            vTaskDelete(NULL);
        }
    }
}


void heart_task(void *pvParameters){
    const char *task_tag = "heartbeat task";
    uint32_t counter = 0;
    while(1){
        ESP_LOGI(task_tag, "Heartbeat %lu", counter++);
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 1 second
    }
}

void debug_task(void *pvParameters) {
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // Every 5 seconds only
        
        uint64_t a_avg = sensor_a_stats.total_samples > 0 ? 
            sensor_a_stats.total_jitter_us / sensor_a_stats.total_samples : 0;
        uint64_t b_avg = sensor_b_stats.total_samples > 0 ? 
            sensor_b_stats.total_jitter_us / sensor_b_stats.total_samples : 0;
            
        printf("A:%lu(%lu,avg:%llu) B:%lu(%lu,avg:%llu) F:%lu\n",
               sensor_a_stats.total_samples, sensor_a_stats.missed_deadlines, a_avg,
               sensor_b_stats.total_samples, sensor_b_stats.missed_deadlines, b_avg,
               fusion_stats.fusion_cycles);
    }
}

void system_moniter_task(void *pvParameters){
    const char *TASK_tag = "System Moniter";
    while(1){
        //print system statistics
        ESP_LOGI(TASK_tag, "SYSTEM STATUS");
        ESP_LOGI(TASK_tag, "FREE HEAP: %lu ", esp_get_free_heap_size());
        ESP_LOGI(TASK_tag, "Min Free heap: %lu", esp_get_minimum_free_heap_size());
        //check task high water mark's
        UBaseType_t sensor_a_hwm = uxTaskGetStackHighWaterMark(sensor_a_task_handle);
        UBaseType_t sensor_b_hwm = uxTaskGetStackHighWaterMark(sensor_b_task_handle);
        UBaseType_t fusion_hwm = uxTaskGetStackHighWaterMark(fusion_task_handle);
        ESP_LOGI(TASK_tag, "Sensor A task high water mark: %lu, Sensor B task high water mark: %lu, Fusion task high water mark: %lu",
                 sensor_a_hwm, sensor_b_hwm, fusion_hwm);
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

void app_main(void){
    // ESP_LOGI(TAG, "RTOS sensor fusion");
    //initialize NVS
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    //rtos core init
    ESP_ERROR_CHECK(rtos_core_init());
    //sensor init
    ESP_ERROR_CHECK(sensors_init());
    //fusion init
    ESP_ERROR_CHECK(fusion_init());
    //create heartbeat task
    // xTaskCreate(system_moniter_task, "SYSTEM MONITER", 3072, NULL, 1, NULL);
    xTaskCreate(debug_task, "DEBUG Task", 2048, NULL, 1,NULL);

    ESP_LOGI(TAG, "ALL SYSTEMS INITIALIZED");
    vTaskDelete(NULL);// removes app_main task as everything is already initialized
}


