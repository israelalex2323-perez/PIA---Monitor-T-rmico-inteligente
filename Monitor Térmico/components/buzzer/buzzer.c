
#include "buzzer.h"
#include "driver/ledc.h"
#include "app_config.h"
#include "estadisticas.h"

#define BUZZER_CANAL LEDC_CHANNEL_3

static uint32_t frecuencia_actual = 0;
static bool tono_activo = false;

/* Activar o desactivar el sonido */
static void Controlar_Tono(bool activar, uint32_t frecuencia)
{
    if (activar)
    {
        if (frecuencia != frecuencia_actual)
        {
            ledc_set_freq(
                LEDC_LOW_SPEED_MODE,
                LEDC_TIMER_1,
                frecuencia
            );

            frecuencia_actual = frecuencia;
        }

        if (!tono_activo)
        {
            ledc_set_duty(
                LEDC_LOW_SPEED_MODE,
                BUZZER_CANAL,
                512
            );

            ledc_update_duty(
                LEDC_LOW_SPEED_MODE,
                BUZZER_CANAL
            );

            tono_activo = true;
        }
    }
    else
    {
        if (tono_activo)
        {
            ledc_set_duty(
                LEDC_LOW_SPEED_MODE,
                BUZZER_CANAL,
                0
            );

            ledc_update_duty(
                LEDC_LOW_SPEED_MODE,
                BUZZER_CANAL
            );

            tono_activo = false;
        }
    }
}

/* Inicializar buzzer mediante PWM */
void Buzzer_Init(void)
{
    ledc_timer_config_t temporizador = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&temporizador);

    ledc_channel_config_t canal = {
        .gpio_num = PIN_BUZZER,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BUZZER_CANAL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = 0,
        .hpoint = 0
    };

    ledc_channel_config(&canal);

    frecuencia_actual = 1000;
    tono_activo = false;
}

/* Actualizar buzzer según el modo de operación */
void Actualizar_Buzzer(void *estado, uint32_t tiempo)
{
    estado_monitor_t *st = (estado_monitor_t *)estado;

    uint32_t posicion;
    bool activar = false;
    uint32_t frecuencia = 1000;

    /* Modo 3: prueba de patrones de sonido */
    if (st->modo == 3)
    {
        posicion = tiempo % 4000;

        /* ETAPA 1: AZUL
           Dos pitidos cortos de 700 Hz */
        if (posicion < 1000)
        {
            frecuencia = 700;

            if (posicion < 150 ||
                (posicion >= 300 && posicion < 450))
            {
                activar = true;
            }
        }

        /* ETAPA 2: AMARILLO
           Buzzer apagado */
        else if (posicion < 2000)
        {
            activar = false;
        }

        /* ETAPA 3: NARANJA
           Buzzer apagado */
        else if (posicion < 3000)
        {
            activar = false;
        }

        /* ETAPA 4: ROJO
           Un pitido largo y dos cortos de 2200 Hz */
        else
        {
            frecuencia = 2200;

            uint32_t tiempo_rojo = posicion - 3000;

            if (tiempo_rojo < 450 ||
                (tiempo_rojo >= 550 && tiempo_rojo < 700) ||
                (tiempo_rojo >= 800 && tiempo_rojo < 950))
            {
                activar = true;
            }
        }
    }

    /* Alarma de temperatura baja */
    else if (st->alarma == 1)
    {
        posicion = tiempo % 1000;

        frecuencia = 700;

        if (posicion < 250 ||
            (posicion >= 500 && posicion < 750))
        {
            activar = true;
        }
    }

    /* Alarma de temperatura alta */
    else if (st->alarma == 2)
    {
        posicion = tiempo % 500;

        frecuencia = 2200;

        if (posicion < 150 ||
            (posicion >= 250 && posicion < 400))
        {
            activar = true;
        }
    }

    /* Aplicar el estado del buzzer */
    Controlar_Tono(activar, frecuencia);

    st->buzzer_activo = activar;
}
