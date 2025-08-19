idf_component_register(
    SRCS "rtos_core.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos esp_timer esp_hw_support
    )