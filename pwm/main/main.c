#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_PIN 2             // Pin GPIO para la salida PWM
#define PWM_FREQ 10         // Frecuencia del PWM en Hz
#define PWM_RES LEDC_TIMER_8_BIT // Resolución del PWM (8 bits = 256 niveles)
#define PWM_CHANNEL LEDC_CHANNEL_0 // Canal LEDC (puedes elegir entre 0 y 7)

void app_main() {
    // Configurar el temporizador para el PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE, // Modo de velocidad (baja o alta)
        .timer_num        = LEDC_TIMER_0,       // Temporizador utilizado (0 a 3)
        .duty_resolution  = PWM_RES,            // Resolución del PWM
        .freq_hz          = PWM_FREQ,           // Frecuencia en Hz
        .clk_cfg          = LEDC_AUTO_CLK       // Selección de reloj (automática)
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configurar el canal para el PWM
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = PWM_PIN,             // GPIO asignado al canal
        .speed_mode     = LEDC_LOW_SPEED_MODE, // Modo de velocidad
        .channel        = PWM_CHANNEL,         // Canal asignado
        .intr_type      = LEDC_INTR_DISABLE,   // Interrupciones deshabilitadas
        .timer_sel      = LEDC_TIMER_0,        // Temporizador utilizado
        .duty           = 0,                   // Ciclo de trabajo inicial (0%)
        .hpoint         = 0                    // Punto de inicio del pulso (0)
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // Cambiar el ciclo de trabajo en un bucle (efecto de aumento/disminución de brillo)
    while (1) {
        for (int duty = 0; duty <= 10; duty++) { // Incrementar el duty cycle
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, duty));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL));
            vTaskDelay(pdMS_TO_TICKS(10)); // Esperar 10ms
        }

        for (int duty = 255; duty >= 0; duty--) { // Decrementar el duty cycle
            ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, duty));
            ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL));
            vTaskDelay(pdMS_TO_TICKS(10)); // Esperar 10ms
        }
    }
}
