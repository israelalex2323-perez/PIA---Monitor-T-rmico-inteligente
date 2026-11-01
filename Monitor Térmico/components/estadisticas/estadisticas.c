
#include "estadisticas.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

/* Variables para controlar los periodos de ejecución */
static uint32_t ultima_muestra = 0;
static uint32_t ultimo_diagnostico = 0;

/* Inicializar los contadores de tiempo */
void Estadisticas_Init(void)
{
    ultima_muestra = 0;
    ultimo_diagnostico = 0;
}

/* Inicializar los parámetros generales */
void Parametros_Init(void)
{
    /* Los valores iniciales del estado se conservan en cero */
}

/* Obtener el tiempo transcurrido desde el inicio */
uint32_t Obtener_Tiempo_ms(void)
{
    return (uint32_t)(
        xTaskGetTickCount() * portTICK_PERIOD_MS
    );
}

/* Comprobar si corresponde realizar una medición */
bool Corresponde_Muestrear(
    const estado_monitor_t *estado,
    const void *entradas,
    uint32_t tiempo)
{
    uint32_t intervalo;

    /* No medir durante la prueba de actuadores */
    if (estado->modo == 3) {
        return false;
    }

    /* Seleccionar el intervalo de medición */
    if (estado->modo == 1) {
        intervalo = 500;
    } else {
        intervalo = estado->intervalo_muestreo_ms;

        if (intervalo == 0) {
            intervalo = 1000;
        }
    }

    /* Comprobar si transcurrió el intervalo */
    if (tiempo - ultima_muestra >= intervalo) {
        ultima_muestra = tiempo;
        return true;
    }

    return false;
}

/* Comprobar si corresponde mostrar el diagnóstico */
bool Corresponde_Diagnostico(uint32_t tiempo)
{
    if (tiempo - ultimo_diagnostico >= 1000) {
        ultimo_diagnostico = tiempo;
        return true;
    }

    return false;
}

/* Mostrar los datos actuales por el puerto serie */
void Registrar_Diagnostico(
    const estado_monitor_t *estado,
    uint32_t tiempo)
{
    printf(
        "Temperatura: %.2f C | Min: %.2f C | Max: %.2f C | "
        "Muestras: %u | Modo: %u | Alarma: %u | Buzzer: %s\n",
        (double)estado->temperatura,
        (double)estado->temp_min,
        (double)estado->temp_max,
        (unsigned int)estado->cantidad_historial,
        (unsigned int)estado->modo,
        (unsigned int)estado->alarma,
        estado->buzzer_activo ? "ACTIVO" : "APAGADO"
    );
}

/* Actualizar temperatura, extremos e historial */
void Actualizar_Estado_Monitor(
    estado_monitor_t *estado,
    resultado_sensor_t *resultado,
    uint32_t tiempo)
{
    /* Procesar únicamente las lecturas válidas */
    if (resultado->valida) {

        estado->temperatura = resultado->temperatura;
        estado->error_sensor = false;

        /* Inicializar mínimo y máximo con la primera lectura */
        if (!estado->primera_lectura) {
            estado->temp_min = resultado->temperatura;
            estado->temp_max = resultado->temperatura;
            estado->primera_lectura = true;
        } else {

            /* Actualizar el mínimo */
            if (resultado->temperatura < estado->temp_min) {
                estado->temp_min = resultado->temperatura;
            }

            /* Actualizar el máximo */
            if (resultado->temperatura > estado->temp_max) {
                estado->temp_max = resultado->temperatura;
            }
        }

        /* Guardar la lectura en el historial */
        estado->historial[estado->indice_historial] =
            resultado->temperatura;

        estado->indice_historial++;

        /* Volver al inicio al completar el historial */
        if (estado->indice_historial >= TAMANO_HISTORIAL) {
            estado->indice_historial = 0;
        }

        /* Incrementar el contador de muestras válidas */
        if (estado->cantidad_historial < TAMANO_HISTORIAL) {
            estado->cantidad_historial++;
        }

    } else {

        /* Indicar un error de lectura */
        estado->error_sensor = true;
    }
}

/* Determinar el estado de la alarma */
void Actualizar_Estado_Alarma(estado_monitor_t *estado)
{
    /* Priorizar los errores del sensor */
    if (estado->error_sensor) {
        estado->alarma = 3;
    } else if (estado->temperatura < 20.0f) {
        estado->alarma = 1;
    } else if (estado->temperatura > 40.0f) {
        estado->alarma = 2;
    } else {
        estado->alarma = 0;
    }
}

/* Actualizar el modo seleccionado por los DIP switches */
void Actualizar_Modo_Operacion(
    estado_monitor_t *estado,
    uint8_t modo_dip)
{
    estado->modo = modo_dip;
}

/* Reiniciar las estadísticas */
void Reiniciar_Estadisticas(estado_monitor_t *estado)
{
    estado->temp_min = 0;
    estado->temp_max = 0;
    estado->primera_lectura = false;

    estado->indice_historial = 0;
    estado->cantidad_historial = 0;

    /* Borrar las muestras anteriores */
    for (int i = 0; i < TAMANO_HISTORIAL; i++) {
        estado->historial[i] = 0.0f;
    }
}
