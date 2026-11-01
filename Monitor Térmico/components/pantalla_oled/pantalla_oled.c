
#include "pantalla_oled.h"
#include "sensor_bmp280.h"
#include "estadisticas.h"
#include "driver/i2c_master.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DIRECCION_OLED 0x3C
#define ANCHO_OLED 128
#define TAMANO_BUFFER 1024

static i2c_master_bus_handle_t bus_oled;
static i2c_master_dev_handle_t dispositivo_oled;
static uint8_t buffer_oled[TAMANO_BUFFER];
static uint32_t ultima_oled = 0;

/* Enviar un comando a la pantalla OLED */
static void Enviar_Comando(uint8_t comando)
{
    uint8_t datos[2] = {0x00, comando};
    i2c_master_transmit(dispositivo_oled, datos, sizeof(datos), 1000);
}

/* Enviar una pagina del buffer a la pantalla */
static void Enviar_Pagina(uint8_t pagina)
{
    uint8_t datos[129];
    datos[0] = 0x40;

    for (int x = 0; x < ANCHO_OLED; x++) {
        datos[x + 1] = buffer_oled[pagina * ANCHO_OLED + x];
    }

    Enviar_Comando(0xB0 + pagina);
    Enviar_Comando(0x00);
    Enviar_Comando(0x10);

    i2c_master_transmit(dispositivo_oled, datos, sizeof(datos), 1000);
}

/* Obtener el dibujo correspondiente a cada caracter */
static void Obtener_Letra(char letra, uint8_t dibujo[5])
{
    for (int i = 0; i < 5; i++) {
        dibujo[i] = 0;
    }

    switch (letra) {
        case 'A': dibujo[0]=0x7E; dibujo[1]=0x11; dibujo[2]=0x11; dibujo[3]=0x11; dibujo[4]=0x7E; break;
        case 'B': dibujo[0]=0x7F; dibujo[1]=0x49; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x36; break;
        case 'C': dibujo[0]=0x3E; dibujo[1]=0x41; dibujo[2]=0x41; dibujo[3]=0x41; dibujo[4]=0x22; break;
        case 'D': dibujo[0]=0x7F; dibujo[1]=0x41; dibujo[2]=0x41; dibujo[3]=0x22; dibujo[4]=0x1C; break;
        case 'E': dibujo[0]=0x7F; dibujo[1]=0x49; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x41; break;
        case 'F': dibujo[0]=0x7F; dibujo[1]=0x09; dibujo[2]=0x09; dibujo[3]=0x09; dibujo[4]=0x01; break;
        case 'G': dibujo[0]=0x3E; dibujo[1]=0x41; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x7A; break;
        case 'H': dibujo[0]=0x7F; dibujo[1]=0x08; dibujo[2]=0x08; dibujo[3]=0x08; dibujo[4]=0x7F; break;
        case 'I': dibujo[0]=0x00; dibujo[1]=0x41; dibujo[2]=0x7F; dibujo[3]=0x41; dibujo[4]=0x00; break;
        case 'J': dibujo[0]=0x20; dibujo[1]=0x40; dibujo[2]=0x41; dibujo[3]=0x3F; dibujo[4]=0x01; break;
        case 'K': dibujo[0]=0x7F; dibujo[1]=0x08; dibujo[2]=0x14; dibujo[3]=0x22; dibujo[4]=0x41; break;
        case 'L': dibujo[0]=0x7F; dibujo[1]=0x40; dibujo[2]=0x40; dibujo[3]=0x40; dibujo[4]=0x40; break;
        case 'M': dibujo[0]=0x7F; dibujo[1]=0x02; dibujo[2]=0x0C; dibujo[3]=0x02; dibujo[4]=0x7F; break;
        case 'N': dibujo[0]=0x7F; dibujo[1]=0x04; dibujo[2]=0x08; dibujo[3]=0x10; dibujo[4]=0x7F; break;
        case 'O': dibujo[0]=0x3E; dibujo[1]=0x41; dibujo[2]=0x41; dibujo[3]=0x41; dibujo[4]=0x3E; break;
        case 'P': dibujo[0]=0x7F; dibujo[1]=0x09; dibujo[2]=0x09; dibujo[3]=0x09; dibujo[4]=0x06; break;
        case 'Q': dibujo[0]=0x3E; dibujo[1]=0x41; dibujo[2]=0x51; dibujo[3]=0x21; dibujo[4]=0x5E; break;
        case 'R': dibujo[0]=0x7F; dibujo[1]=0x09; dibujo[2]=0x19; dibujo[3]=0x29; dibujo[4]=0x46; break;
        case 'S': dibujo[0]=0x46; dibujo[1]=0x49; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x31; break;
        case 'T': dibujo[0]=0x01; dibujo[1]=0x01; dibujo[2]=0x7F; dibujo[3]=0x01; dibujo[4]=0x01; break;
        case 'U': dibujo[0]=0x3F; dibujo[1]=0x40; dibujo[2]=0x40; dibujo[3]=0x40; dibujo[4]=0x3F; break;
        case 'V': dibujo[0]=0x1F; dibujo[1]=0x20; dibujo[2]=0x40; dibujo[3]=0x20; dibujo[4]=0x1F; break;
        case 'W': dibujo[0]=0x3F; dibujo[1]=0x40; dibujo[2]=0x38; dibujo[3]=0x40; dibujo[4]=0x3F; break;
        case 'X': dibujo[0]=0x63; dibujo[1]=0x14; dibujo[2]=0x08; dibujo[3]=0x14; dibujo[4]=0x63; break;
        case 'Y': dibujo[0]=0x07; dibujo[1]=0x08; dibujo[2]=0x70; dibujo[3]=0x08; dibujo[4]=0x07; break;
        case 'Z': dibujo[0]=0x61; dibujo[1]=0x51; dibujo[2]=0x49; dibujo[3]=0x45; dibujo[4]=0x43; break;

        case '0': dibujo[0]=0x3E; dibujo[1]=0x51; dibujo[2]=0x49; dibujo[3]=0x45; dibujo[4]=0x3E; break;
        case '1': dibujo[0]=0x00; dibujo[1]=0x42; dibujo[2]=0x7F; dibujo[3]=0x40; dibujo[4]=0x00; break;
        case '2': dibujo[0]=0x42; dibujo[1]=0x61; dibujo[2]=0x51; dibujo[3]=0x49; dibujo[4]=0x46; break;
        case '3': dibujo[0]=0x21; dibujo[1]=0x41; dibujo[2]=0x45; dibujo[3]=0x4B; dibujo[4]=0x31; break;
        case '4': dibujo[0]=0x18; dibujo[1]=0x14; dibujo[2]=0x12; dibujo[3]=0x7F; dibujo[4]=0x10; break;
        case '5': dibujo[0]=0x27; dibujo[1]=0x45; dibujo[2]=0x45; dibujo[3]=0x45; dibujo[4]=0x39; break;
        case '6': dibujo[0]=0x3C; dibujo[1]=0x4A; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x30; break;
        case '7': dibujo[0]=0x01; dibujo[1]=0x71; dibujo[2]=0x09; dibujo[3]=0x05; dibujo[4]=0x03; break;
        case '8': dibujo[0]=0x36; dibujo[1]=0x49; dibujo[2]=0x49; dibujo[3]=0x49; dibujo[4]=0x36; break;
        case '9': dibujo[0]=0x06; dibujo[1]=0x49; dibujo[2]=0x49; dibujo[3]=0x29; dibujo[4]=0x1E; break;

        case ':': dibujo[1]=0x36; dibujo[2]=0x36; break;
        case '.': dibujo[1]=0x60; dibujo[2]=0x60; break;
        case '-': dibujo[0]=0x08; dibujo[1]=0x08; dibujo[2]=0x08; dibujo[3]=0x08; dibujo[4]=0x08; break;
        case '?': dibujo[0]=0x02; dibujo[1]=0x01; dibujo[2]=0x51; dibujo[3]=0x09; dibujo[4]=0x06; break;
        case '=': dibujo[0]=0x14; dibujo[1]=0x14; dibujo[2]=0x14; dibujo[3]=0x14; dibujo[4]=0x14; break;
        case '/': dibujo[0]=0x40; dibujo[1]=0x20; dibujo[2]=0x10; dibujo[3]=0x08; dibujo[4]=0x04; break;
        case ' ': break;

        default: break;
    }
}

/* Escribir una linea en una pagina */
static void Escribir_Linea(uint8_t pagina, const char *texto)
{
    int posicion = 0;

    while (*texto != '\0' && posicion < 21) {
        uint8_t dibujo[5];
        Obtener_Letra(*texto, dibujo);

        for (int columna = 0; columna < 5; columna++) {
            buffer_oled[pagina * ANCHO_OLED + posicion * 6 + columna] = dibujo[columna];
        }

        buffer_oled[pagina * ANCHO_OLED + posicion * 6 + 5] = 0;

        posicion++;
        texto++;
    }
}

/* Actualizar todas las paginas de la pantalla */
static void Actualizar_Pantalla(void)
{
    for (uint8_t pagina = 0; pagina < 8; pagina++) {
        Enviar_Pagina(pagina);
    }
}

/* Limpiar el contenido del buffer */
static void Limpiar_Buffer(void)
{
    for (int i = 0; i < TAMANO_BUFFER; i++) {
        buffer_oled[i] = 0;
    }
}

/* Inicializar la pantalla OLED */
void Pantalla_OLED_Init(void)
{
    bus_oled = Sensor_BMP280_Obtener_Bus();

    i2c_device_config_t configuracion_oled = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = DIRECCION_OLED,
        .scl_speed_hz = 100000
    };

    i2c_master_bus_add_device(
        bus_oled,
        &configuracion_oled,
        &dispositivo_oled
    );

    Enviar_Comando(0xAE);
    Enviar_Comando(0xD5);
    Enviar_Comando(0x80);
    Enviar_Comando(0xA8);
    Enviar_Comando(0x3F);
    Enviar_Comando(0xD3);
    Enviar_Comando(0x00);
    Enviar_Comando(0x40);
    Enviar_Comando(0x8D);
    Enviar_Comando(0x14);
    Enviar_Comando(0x20);
    Enviar_Comando(0x00);
    Enviar_Comando(0xA1);
    Enviar_Comando(0xC8);
    Enviar_Comando(0xDA);
    Enviar_Comando(0x12);
    Enviar_Comando(0x81);
    Enviar_Comando(0x7F);
    Enviar_Comando(0xD9);
    Enviar_Comando(0xF1);
    Enviar_Comando(0xDB);
    Enviar_Comando(0x40);
    Enviar_Comando(0xA4);
    Enviar_Comando(0xA6);
    Enviar_Comando(0xAF);

    Limpiar_Buffer();
    Actualizar_Pantalla();

    printf("Pantalla OLED iniciada\n");
}

/* Determinar si corresponde actualizar la pantalla */
bool Corresponde_Actualizar_OLED(uint32_t tiempo)
{
    if (tiempo - ultima_oled >= 250) {
        ultima_oled = tiempo;
        return true;
    }

    return false;
}

/* Mostrar el color actual del LED RGB */
static void Mostrar_Color_LED(const estado_monitor_t *datos)
{
    if (datos->error_sensor) {
        Escribir_Linea(5, "LED: SIN DATO");
    }
    else if (datos->temperatura < 20.0f) {
        Escribir_Linea(5, "LED: AZUL");
    }
    else if (datos->temperatura < 30.0f) {
        Escribir_Linea(5, "LED: AMARILLO");
    }
    else if (datos->temperatura <= 40.0f) {
        Escribir_Linea(5, "LED: NARANJA");
    }
    else {
        Escribir_Linea(5, "LED: ROJO");
    }
}

/* Controlar la informacion mostrada en la pantalla OLED */
void Controlar_Pantalla(void *estado)
{
    estado_monitor_t *datos = (estado_monitor_t *)estado;
    char linea[22];

    Limpiar_Buffer();

    /* Vista 0: temperatura actual */
    if (datos->vista == 0) {
        Escribir_Linea(0, "MONITOR TERMICO");

        snprintf(linea, sizeof(linea), "MODO: %02u",
                 (unsigned int)datos->modo);
        Escribir_Linea(1, linea);

        if (datos->error_sensor) {
            Escribir_Linea(3, "TEMP: ERROR");
            Escribir_Linea(6, "ERROR SENSOR");
        }
        else {
            snprintf(linea, sizeof(linea), "TEMP: %.2f C",
                     (double)datos->temperatura);
            Escribir_Linea(3, linea);

            if (datos->alarma == 1) {
                Escribir_Linea(6, "ALERTA: FRIO");
            }
            else if (datos->alarma == 2) {
                Escribir_Linea(6, "ALERTA: CALOR");
            }
            else {
                Escribir_Linea(6, "ESTADO: NORMAL");
            }
        }
    }

    /* Vista 1: estadisticas de temperatura */
    else if (datos->vista == 1) {
        Escribir_Linea(0, "ESTADISTICAS");

        if (datos->error_sensor) {
            Escribir_Linea(2, "ACTUAL: ERROR");
        }
        else {
            snprintf(linea, sizeof(linea), "ACTUAL: %.2f C",
                     (double)datos->temperatura);
            Escribir_Linea(2, linea);
        }

        if (datos->primera_lectura) {
            snprintf(linea, sizeof(linea), "MINIMA: %.2f C",
                     (double)datos->temp_min);
            Escribir_Linea(4, linea);

            snprintf(linea, sizeof(linea), "MAXIMA: %.2f C",
                     (double)datos->temp_max);
            Escribir_Linea(6, linea);
        }
        else {
            Escribir_Linea(4, "MINIMA: SIN DATOS");
            Escribir_Linea(6, "MAXIMA: SIN DATOS");
        }
    }

    /* Vista 2: perifericos o prueba de actuadores */
    else if (datos->vista == 2) {

        if (datos->modo == 3) {
            Escribir_Linea(0, "PRUEBA ACTUADORES");

            if (datos->buzzer_activo) {
                Escribir_Linea(2, "BUZZER: ACTIVO");
            }
            else {
                Escribir_Linea(2, "BUZZER: APAGADO");
            }

            Escribir_Linea(4, "LED: SECUENCIA");
            Escribir_Linea(6, "AZUL Y ROJO: ALARMA");
        }
        else {
            Escribir_Linea(0, "ESTADO PERIFERICOS");

            if (datos->buzzer_activo) {
                Escribir_Linea(2, "BUZZER: ACTIVO");
            }
            else {
                Escribir_Linea(2, "BUZZER: APAGADO");
            }

            Mostrar_Color_LED(datos);

            snprintf(linea, sizeof(linea), "MUESTREO: %lu MS",
                     (unsigned long)datos->intervalo_muestreo_ms);
            Escribir_Linea(7, linea);
        }
    }

    /* Vista 5: historial de temperaturas */
    else if (datos->vista == 5) {
        Escribir_Linea(0, "HISTORIAL TEMP.");

        if (datos->cantidad_historial == 0) {
            Escribir_Linea(3, "SIN MEDICIONES");
        }
        else {
            uint32_t cantidad = datos->cantidad_historial;

            uint32_t inicio =
                (datos->indice_historial +
                 TAMANO_HISTORIAL - cantidad) %
                TAMANO_HISTORIAL;

            uint32_t mostrar = cantidad;

            if (mostrar > 4) {
                mostrar = 4;
            }

            for (uint32_t i = 0; i < mostrar; i++) {
                uint32_t indice =
                    (inicio + cantidad - mostrar + i) %
                    TAMANO_HISTORIAL;

                char texto[22];

                snprintf(
                    texto,
                    sizeof(texto),
                    "%lu: %.2f C",
                    (unsigned long)(i + 1),
                    (double)datos->historial[indice]
                );

                Escribir_Linea(i + 2, texto);
            }
        }
    }

    /* Vista 3: configuracion del intervalo */
    else if (datos->vista == 3) {
        Escribir_Linea(0, "CONFIGURACION");
        Escribir_Linea(2, "GIRE POTENCIOMETRO");

        snprintf(linea, sizeof(linea), "PERIODO: %.1f S",
                 (double)datos->intervalo_configuracion_ms / 1000.0);
        Escribir_Linea(4, linea);

        Escribir_Linea(6, "MANT: GUARDAR");
    }

    /* Vista 4: confirmar reinicio de estadisticas */
    else if (datos->vista == 4) {
        Escribir_Linea(0, "REINICIAR DATOS");
        Escribir_Linea(3, "BORRAR MIN/MAX?");
        Escribir_Linea(5, "CLICK: CONFIRMAR");
        Escribir_Linea(6, "MANT: CANCELAR");
    }

    /* Vista 6: muestreo rapido */
    else if (datos->vista == 6) {
        Escribir_Linea(0, "MUESTREO RAPIDO");
        Escribir_Linea(1, "MODO: ACTIVO");

        if (datos->error_sensor) {
            Escribir_Linea(3, "TEMP: ERROR");
        }
        else {
            snprintf(linea, sizeof(linea), "TEMP: %.2f C",
                     (double)datos->temperatura);

            Escribir_Linea(3, linea);
        }

        Escribir_Linea(5, "INTERVALO: 500 MS");

        if (datos->alarma == 1) {
            Escribir_Linea(7, "ALERTA: FRIO");
        }
        else if (datos->alarma == 2) {
            Escribir_Linea(7, "ALERTA: CALOR");
        }
        else {
            Escribir_Linea(7, "ESTADO: NORMAL");
        }
    }

    /* Enviar el contenido actualizado a la pantalla */
    Actualizar_Pantalla();
}
