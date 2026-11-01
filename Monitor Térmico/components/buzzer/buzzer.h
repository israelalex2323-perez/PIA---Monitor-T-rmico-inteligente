
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>

void Buzzer_Init(void);
void Actualizar_Buzzer(void *estado, uint32_t tiempo);

#endif
