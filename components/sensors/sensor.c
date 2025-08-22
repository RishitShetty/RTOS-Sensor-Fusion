#include "sensor.h"
#include <math.h>
#include <stdlib.h>

static const char *TAG = "SENSOR_A";
static const char *TAG = "SENSOR_B";

QueueHandle_t sensor_a_queue = NULL;
QueueHandle_t sensor_b_queue = NULL;
esp_timer_handle_t sensor_a_timer = NULL;
esp_timer_handle_t sensor_b_timer = NULL;
SemaphoreHandle_t sensor_a_sem = NULL;
SemaphoreHandle_t sensor_b_sem = NULL;
sensor_stats_t sensor_a_stats = {0};
sensor_stats_t sensor_b_stats = {0};

// Generate noise
float generate_noise(void) {
    // Simple pseudo-random noise generator
    return ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
}

void sensor_a_timer_callback(void* arg){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sensor_a_sem, &xHigherPriorityTaskWoken);
    port_YIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void sensor_b_timer_callback(void* arg){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(sensor_b_sem, &xHigherPriorityTaskWoken);
    port_YIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void sensor_a_task(void *pvParameters){
    uint64_t jitter_us = 0;
    uint64_t actual_wake_time_us;
    uint64_t expected_wake_time_us = rtos_get_time_us();
    while (1){
        if(xSemaphoreTake(sensor_a_sem, portMAX_DELAY) == pdTRUE){
            actual_wake_time_us = rtos_get_time_us();
            //calculte jitter 
            if(sensor_a_stats.total_samples > 0){
                //if condition to get value of jitter time
                jitter_us = (actual_wake_time_us > expected_wake_time_us)?
                (actual_wake_time_us - expected_wake_time_us) : (expected_wake_time_us - actual_wake_time_us);
                if(sensor_a_stats.total_samples == 1 || jitter_us < sensor_a_stats.min_jitter_us){
                    sensor_a_stats.min_jitter_us = jitter_us;
                }
                if(sensor_a_stats.max_jitter_us < jitter_us){
                    sensor_a_stats.max_jitter_us = jitter_us;
                }
                sensor_a_stats.total_jitter_us += jitter_us;
                //task runs every 1ms, therefor if jitter exceeds 1ms/2 we observe a missed deadline
                if(jitter_us > 500){
                    sensor_a_stats.missed_deadlines++;
                    ESP_LOGW(TAG, "Sensor A missed deadline Jitter: %llu us", jitter_us);
                }
            }
            // Generate simulated sensor data
            float time_sec = (float)actual_wake_time_us / 1000000.0f;
            float sensor_value = SENSOR_A_AMPLITUDE * sinf(2.0f * M_PI * SENSOR_A_FREQUENCY * time_sec) + SENSOR_A_NOISE_LEVEL * generate_noise();
            sensor_data_t data = {
                .timestamp_us = actual_wake_time_us,
                .value = sensor_value,
                .sensor_id = 1
            };

            //send data to queue
            if(xQueueHandler(sensor_a_queue, &data, 0) != pdTRUE){
                ESP_LOGW(TAG,"Queue is full, Dropping sample");
            }
            sensor_a_stats.total_samples++;
            sensor_a_stats.last_execution_time_us = rtos_get_time_us() - actual_wake_time_us;
            //log every 1000 samples
            if(sensor_a_stats.total_samples % 1000 == 0){
                uint64_t avg_jitter_us = sensor_a_stats.total_jitter_us / sensor_a_stats.total_samples;
                ESP_LOGI(TAG, "Samples = %lu, Avg Jitter = %llu us, Missed Deadlines = %lu", sensor_a_stats.total_samples, avg_jitter_us, sensor_a_stats.missed_deadlines);
            }
            // increment the ideal wake up time after every loop
            expected_wake_time_us += SENSOR_A_PERIOD_US;
        }
    }
}

void sensor_b_task(void *pvParameteres){
    uint64_t jitter_us = 0;
    uint64_t actual_wake_time_us;
    uint64_t expected_wake_time_us = rtos_get_time_us();
    while(1){
        if(xSemaphoreTake(sensor_b_sem, portMAX_DELAY) != pdTRUE){
            actual_wake_time_us = rtos_get_time_us();
            if(sensor_b_stats.total_samples > 0){
                jitter_us = (expected_wake_time_us > actual_wake_time_us)?
                (expected_wake_time_us - actual_wake_time_us) : (actual_wake_time_us - expected_wake_time_us);
                if(sensor_b_stats.total_samples == 1 || jitter_us < sensor_b_stats.min_jitter_us){
                    sensor_b_stats.min_jitter_us = jitter_us;

                }
                if(sensor_b_stats.max_jitter_us < jitter_us){
                    sensor_b_stats.max_jitter_us = jitter_us;
                }
                sensor_b_stats.total_jitter_us += jitter_us;
                //missed deadlines(task runs every 2500ms)
                if(jitter_us > 1250){
                    sensor_b_stats.missed_deadlines++;
                    ESP_LOGW(TAG, "Sensor B missed deadline: %llu us ", jitter_us);
                }
            }
            // Generate simulated sensor data
            float time_sec = (float)actual_wake_time / 1000000.0f;
            float sensor_value = SENSOR_B_AMPLITUDE * cosf(2.0f * M_PI * SENSOR_B_FREQUENCY * time_sec) +
                                SENSOR_B_NOISE_LEVEL * generate_sensor_noise();
            
            sensor_data_t data = {
                .timestamp_us = actual_wake_time,
                .value = sensor_value,
                .sensor_id = 2
            };

            //send the data to the sensor b queue
            if(xQueueSend(sensor_b_queue, &data, 0) != pdTRUE){
                ESP_LOGW(TAG, "sensor B queue is full");//check whether the queue is full
            }
            sensor_b_stats.total_samples++;
            //actual time taken to finish this task
            sensor_b_stats.last_execution_time_us = (rtos_get_time_us() - actual_wake_time_us);

            //logging task every 400 samples (400Hz)
            if(sensor_b_stats.total_samples % 400 =0){
                uint64_t avg_jitter = sensor_b_stats.total_jitter_us / sensor_b_stats.total_samples;
                ESP_LOGI(TAG, "Samples = %lu, Avg Jitter = %llu us, MIssed Deadlines = %lu", sensor_b_stats.total_samples, avg_jitter, sensor_b_stats.missed_deadlines);

            }
            //increment the expected time 
            expected_wake_time_us += SENSOR_B_PERIOD_US;
        }
    }
}

esp_err_t sensors_init(void){
    ESPI(TAG, "initializing sensors");

    //create queues
    sensor_a_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(sensor_data_t));
    sensor_b_queue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(sensor_data_t));
    if(!sensor_a_queue || !sensor_b_queue){
        ESP_LOGE(TAG, "Failed to initialize sensor queues");
    }
    //create semaphores
    sensor_a_sem = xSemaphoreCreateBinary();
    sensor_b_sem = xSemaphoreCreateBinary();
    if(!sensor_a_sem || !sensor_b_sem){
        ESP_LOGE(TAG,"Failed to initialze semaphores");
    }

    //create timer configs
    esp_timer_create_args_t timer_a_config = {
        .callback = &sensor_a_timer_callback,
        .arg = NULL,
        .name = "sensor_a_timer" 
    };
    esp_timer_create_args_t timer_b_config = {
        .callback = &sensor_b_timer_callback,
        .arg = NULL,
        .name = "sensor_b_timer"
    };

    //create timers, writes a va;ue in the address
    ESP_ERROR_CHECK(esp_timer_create(&timer_a_config, &sensor_a_timer));
    ESP_ERROR_CHECK(esp_timer_create(&timer_b_config, &sensor_b_timer));

    //start the timers, uses the value that is already written(no '&')
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_a_timer, SENSOR_A_PERIOD_US));
    ESP_ERROR_CHECK(esp_timer_start_periodic(sensor_b_timer, SENSOR_B_PERIOD_US));

    return ESP_OK;
}