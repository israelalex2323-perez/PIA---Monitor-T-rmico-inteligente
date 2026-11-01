
#ifndef PANTALLA_OLED_H
#define PANTALLA_OLED_H

#include <stdbool.h>
#include <stdint.h>

/* Iniciar la pantalla OLED */
void Pantalla_OLED_Init(void);

/* Revisar si corresponde actualizar la pantalla */
bool Corresponde_Actualizar_OLED(uint32_t tiempo);

/* Mostrar la vista seleccionada */
void Controlar_Pantalla(void *estado);

#endif
