
#include "boton.h"
#include "driver/gpio.h"
#include "app_config.h"
#include "estadisticas.h"
#include "potenciometro.h"

static int boton_anterior = 0;
static int boton_estable = 0;

static uint32_t tiempo_cambio = 0;
static uint32_t inicio_pulsacion = 0;

static bool pulsacion_larga_detectada = false;

/* Inicializar el botón */
void Boton_Init(void)
{
    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);
}

/* Leer botón y potenciómetro */
void Leer_Entradas(
    entradas_monitor_t *entradas,
    uint32_t tiempo)
{
    int lectura = gpio_get_level(PIN_BUTTON);

    entradas->pulsacion_corta = false;
    entradas->pulsacion_larga = false;

    /* Detectar cambios en la entrada */
    if (lectura != boton_anterior)
    {
        boton_anterior = lectura;
        tiempo_cambio = tiempo;
    }

    /* Antirrebote de 30 ms */
    if (tiempo - tiempo_cambio >= 30)
    {
        if (lectura != boton_estable)
        {
            boton_estable = lectura;

            if (boton_estable == 1)
            {
                inicio_pulsacion = tiempo;
                pulsacion_larga_detectada = false;
            }
            else if (!pulsacion_larga_detectada)
            {
                entradas->pulsacion_corta = true;
            }
        }
    }

    /* Detectar pulsación larga de 1 segundo */
    if (boton_estable == 1 &&
        !pulsacion_larga_detectada &&
        tiempo - inicio_pulsacion >= 1000)
    {
        entradas->pulsacion_larga = true;
        pulsacion_larga_detectada = true;
    }

    entradas->boton_pulsado = boton_estable;

    /* Leer potenciómetro */
    entradas->potenciometro = Potenciometro_Leer();
}

/* Procesar las acciones del botón */
void Procesar_Eventos_Boton(
    void *estado,
    entradas_monitor_t *entradas,
    uint32_t tiempo)
{
    estado_monitor_t *monitor = (estado_monitor_t *)estado;

    (void)tiempo;

    monitor->potenciometro = entradas->potenciometro;

    /* Ajustar el intervalo mediante el potenciómetro */
    if (monitor->vista == 3)
    {
        monitor->intervalo_configuracion_ms =
            500 +
            ((uint32_t)entradas->potenciometro * 4500) / 4095;
    }

    /* PULSACION CORTA */
    if (entradas->pulsacion_corta)
    {
        /* Confirmar borrado de estadísticas */
        if (monitor->vista == 4)
        {
            Reiniciar_Estadisticas(monitor);

            monitor->confirmar_reinicio = false;
            monitor->vista = 0;
        }

        /* Mostrar estadísticas */
        else if (monitor->vista == 0)
        {
            monitor->vista = 1;
        }

        /* Regresar al monitoreo */
        else if (monitor->vista == 1)
        {
            monitor->vista = 0;
        }
    }

    /* PULSACION LARGA */
    if (entradas->pulsacion_larga)
    {
        /* Guardar intervalo configurado */
        if (monitor->vista == 3)
        {
            monitor->intervalo_muestreo_ms =
                monitor->intervalo_configuracion_ms;

            monitor->vista = 0;
        }

        /* Cancelar borrado */
        else if (monitor->vista == 4)
        {
            monitor->confirmar_reinicio = false;
            monitor->vista = 1;
        }

        /* Entrar a confirmación de borrado */
        else if (monitor->vista == 1)
        {
            monitor->confirmar_reinicio = true;
            monitor->vista = 4;
        }

        /* Entrar a configuración */
        else if (monitor->vista == 0)
        {
            monitor->intervalo_configuracion_ms =
                monitor->intervalo_muestreo_ms;

            if (monitor->intervalo_configuracion_ms < 500 ||
                monitor->intervalo_configuracion_ms > 5000)
            {
                monitor->intervalo_configuracion_ms = 1000;
            }

            monitor->vista = 3;
        }
    }
}
