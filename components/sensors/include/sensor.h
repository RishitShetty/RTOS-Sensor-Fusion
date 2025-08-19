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

#define SENSOR_QUEUE_SIZE 10

#endif