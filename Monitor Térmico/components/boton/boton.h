
#ifndef BOTON_H
#define BOTON_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int boton_pulsado;
    int potenciometro;
    bool pulsacion_corta;
    bool pulsacion_larga;
} entradas_monitor_t;

void Boton_Init(void);

void Leer_Entradas(
    entradas_monitor_t *entradas,
    uint32_t tiempo
);

void Procesar_Eventos_Boton(
    void *estado,
    entradas_monitor_t *entradas,
    uint32_t tiempo
);

#endif
