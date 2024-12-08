#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "unity_internals.h"
#include <stddef.h>
#include <string.h>
#include "freertos/timers.h"

// Espacio de memoria para las tareas
#define STACK_SIZE 3048

// Definiciones de los timers
#define TRUE 1
#define FALSE 0
#define LED_PIN 2
#define RT_MAX 180

// Configuración de los timers
TimerHandle_t xTimersDynamic;
TimerHandle_t xTimersFixed;
int interval_dynamic = 100;  // Intervalo dinámico en ms
int interval_fixed = 200;    // Intervalo fijo en ms
int timerid_dynamic = 1;
int timerid_fixed = 2;

char tag[5];
unsigned int led_state = FALSE;

// Definición de estados
#define ESTADO_OFF      1 // OFF
#define ESTADO_PRI      2 // 0.55 SEGUNDOS
#define ESTADO_SEGUN    3 // 0.15 SEGUNDOS
#define ESTADO_TRE      4 // 1 SEGUNDO
#define ESTADO_VARIABLE 5 // 0.1 A 1 SEGUNDO

// Variables de estados actuales y anteriores
int ESTADO_SIGUENTE = ESTADO_OFF;
int ESTADO_ACTUAL = ESTADO_OFF;
int ESTADO_ANTERIOR = ESTADO_OFF;

// Prototipos de funciones de los estados
int Func_OFF(void);
int Func_PRI(void);
int Func_SEGUN(void);
int Func_TRE(void);
int Func_VARIABLE(void);

// Función de inicialización de tareas
esp_err_t create_task(void);

// Callback del temporizador dinámico
void vTimerCallbackDynamic(TimerHandle_t pxTimer) {
    led_state = !led_state; // Alterna el estado del LED
    gpio_set_level(LED_PIN, led_state);

    // Cambiar el intervalo del temporizador dinámico según el estado de la máquina
    switch (ESTADO_ACTUAL) {
        case ESTADO_OFF:
            interval_dynamic = 100;
            break;
        case ESTADO_PRI:
            interval_dynamic = 50;
            break;
        case ESTADO_SEGUN:
            interval_dynamic = 500;
            break;
        case ESTADO_TRE:
            interval_dynamic = 100;
            break;
        case ESTADO_VARIABLE:
            interval_dynamic = 50;
            break;
    }
    // Cambiar el periodo del temporizador dinámico
    xTimerChangePeriod(xTimersDynamic, pdMS_TO_TICKS(interval_dynamic), 0);
}

// Callback del temporizador fijo
void vTimerCallbackFixed(TimerHandle_t pxTimer) {
    // Aquí puedes verificar el botón o realizar alguna acción
}

// Creación y configuración de los temporizadores
void SET_TIMER(void) {
    // Crear el temporizador dinámico
    xTimersDynamic = xTimerCreate("TimerDynamic", pdMS_TO_TICKS(interval_dynamic), pdTRUE, (void *)timerid_dynamic, vTimerCallbackDynamic);
    if (xTimersDynamic == NULL) {
        ESP_LOGE(tag, "No se pudo crear el temporizador dinámico");
    } else {
        if (xTimerStart(xTimersDynamic, 0) != pdPASS) {
            ESP_LOGE(tag, "No se pudo iniciar el temporizador dinámico");
        }
    }

    // Crear el temporizador fijo
    xTimersFixed = xTimerCreate("TimerFixed", pdMS_TO_TICKS(interval_fixed), pdTRUE, (void *)timerid_fixed, vTimerCallbackFixed);
    if (xTimersFixed == NULL) {
        ESP_LOGE(tag, "No se pudo crear el temporizador fijo");
    } else {
        if (xTimerStart(xTimersFixed, 0) != pdPASS) {
            ESP_LOGE(tag, "No se pudo iniciar el temporizador fijo");
        }
    }
}

// Funciones de los estados
int Func_OFF(void) {
    ESTADO_ANTERIOR = ESTADO_OFF;
    ESTADO_ACTUAL = ESTADO_OFF;
    return ESTADO_PRI;
}

int Func_PRI(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_PRI;
    return ESTADO_SEGUN;
}

int Func_SEGUN(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_SEGUN;
    return ESTADO_TRE;
}

int Func_TRE(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_TRE;
    return ESTADO_VARIABLE;
}

int Func_VARIABLE(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_VARIABLE;
    return ESTADO_OFF;
}

// Creación de tareas
esp_err_t create_task(void) {
    TaskHandle_t xHandleTin = NULL;
    TaskHandle_t xHandleTLed = NULL;
    TaskHandle_t xHandleTout = NULL;

    // Tarea de la máquina de estados
    xTaskCreate(vTaksTin, "vTaksTin", STACK_SIZE, NULL, 1, &xHandleTin);

    // Tarea del LED
    xTaskCreate(vTaksTLed, "vTaksTLed", STACK_SIZE, NULL, 1, &xHandleTLed);

    // Tarea de salida
    xTaskCreate(vTaksTout, "vTaksTout", STACK_SIZE, NULL, 1, &xHandleTout);

    return ESP_OK;
}

// Tarea de la máquina de estados
void vTaksTin(void * pvParameters) {
    while (1) {
        if (ESTADO_SIGUENTE != ESTADO_ACTUAL) {
            ESTADO_ACTUAL = ESTADO_SIGUENTE;
            printf("El estado actual es el: %d\n", ESTADO_ACTUAL);
        }

        if (ESTADO_SIGUENTE == ESTADO_OFF) {
            ESTADO_SIGUENTE = Func_OFF();
        } else if (ESTADO_SIGUENTE == ESTADO_PRI) {
            ESTADO_SIGUENTE = Func_PRI();
        } else if (ESTADO_SIGUENTE == ESTADO_SEGUN) {
            ESTADO_SIGUENTE = Func_SEGUN();
        } else if (ESTADO_SIGUENTE == ESTADO_TRE) {
            ESTADO_SIGUENTE = Func_TRE();
        } else if (ESTADO_SIGUENTE == ESTADO_VARIABLE) {
            ESTADO_SIGUENTE = Func_VARIABLE();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Retardo para liberar el CPU
    }
}

// Tarea del LED
void vTaksTLed(void * pvParameters) {
    gpio_set_level(LED_PIN, FALSE);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Configurar e iniciar los temporizadores
    SET_TIMER();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500)); // Esperar entre cambios
    }
}

// Tarea de salida (puedes agregar lo que necesites)
void vTaksTout(void * pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Tiempo de espera
    }
}

int main() {
    create_task();
    printf("Comienzo\n");
    vTaskStartScheduler(); // Inicia el scheduler de FreeRTOS
    return 0;
}
