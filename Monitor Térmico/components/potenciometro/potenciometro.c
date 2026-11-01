#include "potenciometro.h"
#include "app_config.h"
#include "esp_adc/adc_oneshot.h"

/* Variable para conservar la configuración del ADC */
static adc_oneshot_unit_handle_t manejador_adc;

/* Inicializar el potenciómetro */
void Potenciometro_Init(void)
{
    adc_oneshot_unit_init_cfg_t configuracion = {
        .unit_id = ADC_UNIT_1
    };

    adc_oneshot_new_unit(&configuracion, &manejador_adc);

    /* Configurar el canal de lectura del potenciómetro */
    adc_oneshot_chan_cfg_t canal = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12
    };

    adc_oneshot_config_channel(
        manejador_adc,
        POT_ADC_CHANNEL,
        &canal
    );
}

/* Obtener la lectura del potenciómetro */
int Potenciometro_Leer(void)
{
    int lectura_potenciometro = 0;

    adc_oneshot_read(
        manejador_adc,
        POT_ADC_CHANNEL,
        &lectura_potenciometro
    );

    return lectura_potenciometro;
}

/* Convertir la lectura en un intervalo de muestreo */
uint32_t Potenciometro_Obtener_Intervalo(void)
{
    int lectura_potenciometro;
    uint32_t intervalo_muestreo;

    lectura_potenciometro = Potenciometro_Leer();

    intervalo_muestreo = 500 +
        ((uint32_t)lectura_potenciometro * 4500) / 4095;

    return intervalo_muestreo;
}