
#include "led_rgb.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "estadisticas.h"

#define PIN_ROJO   14
#define PIN_VERDE  27
#define PIN_AZUL   26

#define LED_PWM_ROJO   LEDC_CHANNEL_0
#define LED_PWM_VERDE  LEDC_CHANNEL_1
#define LED_PWM_AZUL   LEDC_CHANNEL_2

/* Controlar la intensidad de cada color del LED RGB.
   Como el LED es de catodo comun, se utilizan directamente
   los valores PWM para encender y apagar cada color. */
static void Escribir_Color(
    uint32_t rojo,
    uint32_t verde,
    uint32_t azul)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_ROJO, rojo);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_ROJO);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_VERDE, verde);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_VERDE);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_PWM_AZUL, azul);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_PWM_AZUL);
}

/* Inicializar el temporizador y los tres canales PWM
   correspondientes a los colores del LED RGB. */
void LED_RGB_Init(void)
{
    ledc_timer_config_t temporizador = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&temporizador);

    ledc_channel_config_t canal_rojo = {
        .gpio_num = PIN_ROJO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_PWM_ROJO,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config_t canal_verde = {
        .gpio_num = PIN_VERDE,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_PWM_VERDE,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config_t canal_azul = {
        .gpio_num = PIN_AZUL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_PWM_AZUL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&canal_rojo);
    ledc_channel_config(&canal_verde);
    ledc_channel_config(&canal_azul);

    /* Iniciar con todos los colores apagados. */
    Escribir_Color(0, 0, 0);
}

/* Seleccionar el color del LED RGB dependiendo de la
   temperatura o del modo de prueba de actuadores. */
void Actualizar_LED_RGB(void *estado, uint32_t tiempo)
{
    estado_monitor_t *st = (estado_monitor_t *)estado;

    /* Modo 3: secuencia de prueba de actuadores. */
    if (st->modo == 3)
    {
        uint32_t paso = (tiempo / 1000) % 4;

        if (paso == 0)
        {
            Escribir_Color(0, 0, 255);       // Azul
        }
        else if (paso == 1)
        {
            Escribir_Color(255, 255, 0);     // Amarillo
        }
        else if (paso == 2)
        {
            Escribir_Color(255, 35, 0);      // Naranja
        }
        else
        {
            Escribir_Color(255, 0, 0);       // Rojo
        }
    }

    /* Modo normal: colores segun la temperatura. */
    else if (st->temperatura < 20.0f)
    {
        Escribir_Color(0, 0, 255);           // Azul
    }
    else if (st->temperatura < 30.0f)
    {
        Escribir_Color(255, 255, 0);         // Amarillo
    }
    else if (st->temperatura <= 40.0f)
    {
        Escribir_Color(255, 35, 0);          // Naranja
    }
    else
    {
        Escribir_Color(255, 0, 0);           // Rojo
    }
}
