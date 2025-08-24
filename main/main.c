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
TaskHandle_t sensor_a_task_handle = NULL;
TaskHandle_t sensor_b_task_handle = NULL;
TaskHandle_t fusion_task_handle = NULL;
TaskHandle_t cli_task_handle = NULL;

static const char *TAG = "MAIN";

void heart_task(void *pvParameters){
    const char *task_tag = "heartbeat task";
    uint32_t counter = 0;
    while(1){
        ESP_LOGI(task_tag, "Heartbeat %lu", counter++);
        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 1 second
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
        UBaseType_t sensor_a_hwm = uxTaskGetHighWaterMark(sensor_a_task_handle);
        UBaseType_t sensor_b_hwm = uxTaskGetHighWaterMark(sensor_b_task_handle);
        UBaseType_t fusion_hwm = uxTaskGetHighWaterMark(fusion_task_handle);
        ESP_LOGI(TASK_tag, "Sensor A task high water mark: %lu, Sensor B task high water mark: %lu, Fusion task high water mark: %lu",
                 sensor_a_hwm, sensor_b_hwm, fusion_hwm);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void){
    ESP_LOGI(TAG, "RTOS sensor fusion");
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
    xTaskCreate(heart_task, "heartbeat_task", 2048, NULL, 1, NULL);
    ESP_LOGI(TAG, "HEARTBEAT TASK INITIALIZED");
    vTaskDelete(NULL);// removes app_main task as everything is already initialized
}


