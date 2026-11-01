#ifndef POTENCIOMETRO_H
#define POTENCIOMETRO_H

#include <stdint.h>

void Potenciometro_Init(void);

int Potenciometro_Leer(void);

uint32_t Potenciometro_Obtener_Intervalo(void);

#endif