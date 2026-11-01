
#ifndef ESTADISTICAS_H
#define ESTADISTICAS_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor_bmp280.h"

#define TAMANO_HISTORIAL 100

typedef struct {
    float temperatura;
    float temp_min;
    float temp_max;

    uint8_t modo;
    uint8_t alarma;
    uint8_t vista;

    bool primera_lectura;
    bool error_sensor;
    bool confirmar_reinicio;
    bool buzzer_activo;

    int potenciometro;

    uint32_t intervalo_muestreo_ms;
    uint32_t intervalo_configuracion_ms;

    float historial[TAMANO_HISTORIAL];
    uint16_t indice_historial;
    uint16_t cantidad_historial;

} estado_monitor_t;

void Estadisticas_Init(void);
void Parametros_Init(void);
uint32_t Obtener_Tiempo_ms(void);

bool Corresponde_Muestrear(
    const estado_monitor_t *estado,
    const void *entradas,
    uint32_t tiempo
);

bool Corresponde_Diagnostico(uint32_t tiempo);

void Registrar_Diagnostico(
    const estado_monitor_t *estado,
    uint32_t tiempo
);

void Actualizar_Estado_Monitor(
    estado_monitor_t *estado,
    resultado_sensor_t *resultado,
    uint32_t tiempo
);

void Actualizar_Estado_Alarma(estado_monitor_t *estado);

void Actualizar_Modo_Operacion(
    estado_monitor_t *estado,
    uint8_t modo_dip
);

void Reiniciar_Estadisticas(estado_monitor_t *estado);

#endif
