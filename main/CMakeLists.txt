idf_component_register(SRCS "main.c" 
                       INCLUDE_DIRS "." 
                       PRIV_REQUIRES rtos_core nvs_flash freertos sensors fusion)
