
#ifndef SENSOR_BMP280_H
#define SENSOR_BMP280_H

#include <stdbool.h>
#include "driver/i2c_master.h"

typedef struct {
    float temperatura;
    bool valida;
} resultado_sensor_t;

/* Iniciar el sensor BMP280 */
void Sensor_BMP280_Init(void);

/* Obtener el bus I2C compartido */
i2c_master_bus_handle_t Sensor_BMP280_Obtener_Bus(void);

/* Leer la temperatura del sensor */
resultado_sensor_t Adquirir_Temperatura(void);

#endif
