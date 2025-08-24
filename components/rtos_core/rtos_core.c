#include "rtos_core.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_clk_tree.h" 

static const char *TAG = "RTOS_CORE";

esp_err_t rtos_core_init(void){
    // ESP_LOGI(TAG, "Initialized RTOS Core");
    // ESP_LOGI(TAG, "RTOS Core is running %d", tskKERNEL_VERSION_NUMBER);
    uint32_t cpu_freq_mhz = 0;
    esp_err_t re = esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_APPROX, &cpu_freq_mhz);
    if(re == ESP_OK){
        // ESP_LOGI(TAG,"CPU FREQUENCY: %lu Mhz", cpu_freq_mhz / 1000000);
    }
    else{
        // ESP_LOGI(TAG, "Failed to get CPU frequency");
    }

    return ESP_OK; // Return success
}

uint64_t rtos_get_time_us(void){
    return esp_timer_get_time();
}

void delay_until_us(uint64_t *last_wake_time, uint64_t period_us){
    uint64_t current_time_us = rtos_get_time_us();
    uint64_t next_wake_time = *last_wake_time + period_us;

    if(current_time_us < next_wake_time){
        uint64_t delay_us = next_wake_time - current_time_us;
        if(delay_us >= 1000){
        uint64_t delay_ms = delay_us/1000;//converting to ms
        vTaskDelay(pdMS_TO_TICKS(delay_ms));//vTaskDelay(pdMS_TO_TICKS(ms))
        }
        else{
            while(rtos_get_time_us() < next_wake_time){

            }
        }
    }
    *last_wake_time = next_wake_time;
}