
#include "sensor_bmp280.h"
#include "app_config.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#define DIRECCION_BMP280  0x77
#define REGISTRO_ID       0xD0
#define REGISTRO_RESET    0xE0
#define REGISTRO_CONTROL  0xF4
#define REGISTRO_CONFIG   0xF5
#define REGISTRO_DATOS    0xF7
#define REGISTRO_CALIB    0x88

static i2c_master_bus_handle_t bus_i2c;
static i2c_master_dev_handle_t sensor_i2c;

static uint16_t T1;
static int16_t T2, T3;
static int32_t ajuste_temperatura;

/* Leer registros del sensor */
static void Leer_Registros(uint8_t registro, uint8_t *datos, size_t cantidad)
{
    i2c_master_transmit_receive(
        sensor_i2c, &registro, 1, datos, cantidad, 1000
    );
}

/* Escribir un registro del sensor */
static void Escribir_Registro(uint8_t registro, uint8_t valor)
{
    uint8_t datos[2] = {registro, valor};

    i2c_master_transmit(sensor_i2c, datos, sizeof(datos), 1000);
}

/* Convertir dos bytes en un número de 16 bits */
static uint16_t Leer_Valor_16(uint8_t *datos)
{
    return (uint16_t)datos[0] | ((uint16_t)datos[1] << 8);
}

/* Leer los valores de calibración de temperatura */
static void Leer_Calibracion(void)
{
    uint8_t datos[6];

    Leer_Registros(REGISTRO_CALIB, datos, sizeof(datos));

    T1 = Leer_Valor_16(&datos[0]);
    T2 = (int16_t)Leer_Valor_16(&datos[2]);
    T3 = (int16_t)Leer_Valor_16(&datos[4]);
}

/* Calcular la temperatura en grados Celsius */
static float Calcular_Temperatura(int32_t dato)
{
    int32_t var1, var2;

    var1 = ((((dato >> 3) - ((int32_t)T1 << 1))) * T2) >> 11;

    var2 = (((((dato >> 4) - (int32_t)T1) *
              ((dato >> 4) - (int32_t)T1)) >> 12) * T3) >> 14;

    ajuste_temperatura = var1 + var2;

    return (float)((ajuste_temperatura * 5 + 128) >> 8) / 100.0f;
}

/* Iniciar la comunicación y el sensor */
void Sensor_BMP280_Init(void)
{
    i2c_master_bus_config_t configuracion_bus = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = PIN_I2C_SDA,
        .scl_io_num = PIN_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };

    i2c_new_master_bus(&configuracion_bus, &bus_i2c);

    i2c_device_config_t configuracion_sensor = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DIRECCION_BMP280,
        .scl_speed_hz = 100000
    };

    i2c_master_bus_add_device(
        bus_i2c, &configuracion_sensor, &sensor_i2c
    );

    uint8_t identificacion = 0;

    Leer_Registros(REGISTRO_ID, &identificacion, 1);

    if (identificacion == 0x58) {
        Escribir_Registro(REGISTRO_RESET, 0xB6);

        vTaskDelay(pdMS_TO_TICKS(10));

        Leer_Calibracion();

        Escribir_Registro(REGISTRO_CONFIG, 0xA0);
        Escribir_Registro(REGISTRO_CONTROL, 0x27);

        printf("BMP280 iniciado correctamente\n");
    } else {
        printf("No se detecto el BMP280. ID: 0x%02X\n", identificacion);
    }
}

/* Obtener el bus I2C compartido */
i2c_master_bus_handle_t Sensor_BMP280_Obtener_Bus(void)
{
    return bus_i2c;
}

/* Obtener una lectura de temperatura */
resultado_sensor_t Adquirir_Temperatura(void)
{
    resultado_sensor_t resultado = {0.0f, false};
    uint8_t datos[6] = {0};

    Leer_Registros(REGISTRO_DATOS, datos, sizeof(datos));

    int32_t dato_temperatura =
        ((int32_t)datos[3] << 12) |
        ((int32_t)datos[4] << 4) |
        ((int32_t)datos[5] >> 4);

    if (T1 != 0 && dato_temperatura != 0 &&
        dato_temperatura != 0x80000) {
        resultado.temperatura =
            Calcular_Temperatura(dato_temperatura);

        resultado.valida = true;
    }

    return resultado;
}
