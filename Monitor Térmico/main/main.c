
/*
En este archivo se encuentra la función principal del programa. Desde aquí
se inicializan los componentes del monitor térmico y se controla la forma
en que trabajan durante la ejecución.

El programa está dividido en diferentes componentes para que cada uno
se encargue de una función específica.

El main funciona como el punto donde se relacionan todos estos componentes,
ya que obtiene la información de cada uno y determina cuándo deben
realizarse las diferentes operaciones.
*/

/* 1. LIBRERIAS */

/*
En esta primera sección se incluyen las librerías necesarias para el
funcionamiento del programa.

Las librerías generales son stdio.h, que permite utilizar funciones de
entrada y salida, principalmente printf(), para mostrar información en
el monitor serie; stdint.h, que permite utilizar datos enteros con un
tamaño definido, como uint8_t y uint32_t; y stdbool.h, que permite
trabajar con variables booleanas (true y false), utilizadas para
indicar la validez de una lectura.

Después tenemos las librerías de FreeRTOS, tales como FreeRTOS.h,
que contiene las definiciones principales del sistema para la ejecución
del programa, y task.h, que proporciona las funciones necesarias para
trabajar con tareas y controlar los tiempos de espera.

Por último, tenemos las librerías creadas para cada uno de los
componentes utilizados en el proyecto:

- sensor_bmp280.h contiene las funciones para inicializar el sensor,
obtener el bus I2C compartido y realizar las lecturas de temperatura.

- pantalla_oled.h inicializa la pantalla OLED, controla sus tiempos
de actualización y muestra la información del monitor de acuerdo
con la vista seleccionada.

- boton.h contiene las funciones para iniciar y leer el botón,
identifica las pulsaciones cortas y largas y ejecuta las acciones
correspondientes.

- potenciometro.h inicia el ADC del ESP32 y obtiene la lectura del
potenciómetro utilizada para modificar el intervalo de muestreo.

- dip_switch.h contiene las funciones de configuración y lectura de
los DIP switches, los cuales permiten seleccionar los diferentes
modos de funcionamiento del sistema.

- led_rgb.h inicia y controla el LED RGB, seleccionando colores de
acuerdo con la temperatura o con la secuencia establecida para el
modo de prueba.

- buzzer.h contiene las funciones para inicializar y controlar el
buzzer, generando los sonidos correspondientes a las alarmas y a
la prueba de periféricos.

- estadisticas.h contiene la estructura principal del monitor y las
funciones utilizadas para administrar las mediciones, los valores
mínimo y máximo, el historial de temperaturas, los tiempos de
muestreo y las condiciones de alarma.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sensor_bmp280.h"
#include "pantalla_oled.h"
#include "boton.h"
#include "potenciometro.h"
#include "dip_switch.h"
#include "led_rgb.h"
#include "buzzer.h"
#include "estadisticas.h"


/* 2. VARIABLES PRINCIPALES DEL MONITOR */

/*
En este bloque se declaran las variables principales que permiten
almacenar y controlar la información del monitor térmico durante su
funcionamiento.

Se utilizan dos estructuras para guardar los datos generales del
sistema y las lecturas de sus entradas, mientras que las demás
variables permiten llevar el control del tiempo, identificar los
cambios en el modo de operación y ejecutar correctamente las
secuencias del modo de prueba.

Primero se establece la estructura Estado, donde se almacena la
información general del monitor, como la temperatura actual, los
valores mínimo y máximo, el modo de operación, el estado de las
alarmas, la vista de la pantalla, el intervalo de muestreo y el
historial de temperatura.

Después se establece la estructura Entradas, donde se guardan los
valores del botón y el potenciómetro y sus indicaciones.

En la siguiente variable se almacena el tiempo actual del microcontrolador
en milisegundos. Se utiliza para controlar cuándo deben realizarse las
mediciones, actualizar la pantalla y ejecutar las secuencias de los
actuadores.

Luego, se guarda el tiempo en que se realizó el último mensaje de
diagnóstico en el monitor serie.

Posteriormente, se guarda el modo de funcionamiento que se encontraba
seleccionado anteriormente. Se inicializa en 255 porque los modos
disponibles van del 0 al 3, permitiendo detectar el primer modo
seleccionado al iniciar el programa.

Por último, se guarda el tiempo en que se ingresa al modo de prueba.
Se utiliza para calcular el tiempo transcurrido desde que comenzó
el modo y controlar sus secuencias.
*/

static estado_monitor_t Estado;
static entradas_monitor_t Entradas;

static uint32_t Tiempo_Actual = 0;
static uint32_t ultimo_diag = 0;

/* Control del cambio de modo */
static uint8_t modo_anterior = 255;

/* Inicio de la secuencia de prueba */
static uint32_t inicio_modo3 = 0;


/* 3. FUNCIÓN PRINCIPAL APP_MAIN */

/*
Es la función desde donde comienza la ejecución del programa.

Primero se inicializan los parámetros y componentes del monitor térmico.
Después se ejecuta un ciclo continuo en el que se leen las entradas,
se revisa el modo seleccionado, se realizan las mediciones y se
actualizan los dispositivos de salida.
*/

void app_main(void)
{
    /* 3.1. INICIALIZACIÓN DE LOS COMPONENTES */

    /*
    1. Se llama a Parametros_Init(), función del componente de
    estadísticas, para preparar los valores iniciales utilizados
    por el monitor.

    2. En Estado.intervalo_muestreo_ms = 1000; se establece un
    intervalo inicial de 1000 milisegundos para realizar las
    mediciones de temperatura. Este valor puede modificarse
    posteriormente mediante el potenciómetro.

    3. En Estado.intervalo_configuracion_ms = 1000; se establece
    el valor inicial del intervalo que se utilizará durante el
    proceso de configuración.

    4. Después se selecciona la vista 0, que corresponde a la
    pantalla principal del monitor térmico, en Estado.vista.

    5. Sensor_BMP280_Init(); inicia el sensor BMP280 y configura
    la comunicación I2C necesaria para obtener las lecturas
    de temperatura.

    6. Pantalla_OLED_Init(); inicializa la pantalla OLED y prepara
    la comunicación I2C para mostrar la información del monitor.

    7. Boton_Init(); configura el pin del botón como entrada
    digital para detectar las pulsaciones realizadas.

    8. Potenciometro_Init(); configura el convertidor analógico-digital
    del ESP32 para obtener las lecturas del potenciómetro.

    9. DIP_Switch_Init(); configura los pines de los DIP switches
    como entradas digitales para seleccionar los modos de
    funcionamiento.

    10. LED_RGB_Init(); configura los canales PWM necesarios para
    controlar los colores del LED RGB.

    11. Buzzer_Init(); configura el PWM del buzzer para generar
    los diferentes sonidos de las alarmas y del modo de prueba.

    12. Estadisticas_Init(); inicializa las variables internas del
    componente de estadísticas, utilizadas principalmente para
    llevar el control de los tiempos.
    */

    Parametros_Init();
    Estado.intervalo_muestreo_ms = 1000;
    Estado.intervalo_configuracion_ms = 1000;
    Estado.vista = 0;

    Sensor_BMP280_Init();
    Pantalla_OLED_Init();
    Boton_Init();
    Potenciometro_Init();
    DIP_Switch_Init();
    LED_RGB_Init();
    Buzzer_Init();
    Estadisticas_Init();


    /* 3.2. CICLO PRINCIPAL WHILE */

    /*
    El while(1) permite que el programa se mantenga ejecutándose
    mientras el ESP32 permanezca encendido.

    Dentro de este ciclo se realizan continuamente las lecturas,
    comprobaciones y actualizaciones necesarias para el
    funcionamiento del monitor.
    */

    while (1)
    {
        /* 1. Obtener el tiempo actual */
        /*
        Se llama a Obtener_Tiempo_ms(), función del componente de
        estadísticas, que obtiene el tiempo de funcionamiento a
        partir de los ticks de FreeRTOS y lo convierte a milisegundos.
        Este valor se utiliza como referencia para controlar los
        diferentes intervalos del programa.
        */
        Tiempo_Actual = Obtener_Tiempo_ms();


        /* 2. Leer las entradas */
        /*
        Se llama a Leer_Entradas(), función del componente del botón.
        Esta función obtiene el estado del botón y también solicita
        la lectura del potenciómetro.

        Los valores se guardan en la estructura Entradas para
        utilizarlos posteriormente. También se envía el tiempo actual
        para llevar el control de las pulsaciones y del antirrebote
        del botón.
        */
        Leer_Entradas(&Entradas, Tiempo_Actual);


        /* 3. Leer los DIP switches */
        /*
        Se obtiene la combinación seleccionada mediante la función
        Leer_DipSwitch(). Esta función lee los estados de SW1 y SW2
        y devuelve un valor numérico que representa el modo seleccionado.

        Las combinaciones posibles son:
        0 = SW1 apagado y SW2 apagado.
        1 = SW1 encendido y SW2 apagado.
        2 = SW1 apagado y SW2 encendido.
        3 = SW1 encendido y SW2 encendido.
        */
        uint8_t modo_actual = Leer_DipSwitch();


        /* 4. Seleccionar el modo de operación */
        /*
        Se utiliza switch para revisar el valor obtenido de los
        DIP switches y asignarlo a Estado.modo. Esta variable permite
        que los demás componentes conozcan el modo en que se encuentra
        trabajando el monitor.

        case 0: Se aplica para mostrar el funcionamiento normal
        del monitor térmico. En él se muestra la primera vista,
        donde se pueden apreciar datos como la temperatura actual,
        el modo actual y el estado del LED RGB de acuerdo con la
        temperatura registrada por el sensor.

        case 1: Se ejecuta cuando el valor de modo_actual es 1.
        Se asigna el valor 1 a Estado.modo, seleccionando el modo
        de muestreo rápido, en el que se realizan mediciones con
        mayor frecuencia.

        case 2: Se ejecuta cuando el valor de modo_actual es 2.
        Se asigna el valor 2 a Estado.modo, indicando que el monitor
        trabajará en el modo de historial de temperaturas.

        case 3: Se ejecuta cuando el valor de modo_actual es 3.
        Se asigna el valor 3 a Estado.modo, seleccionando el modo
        de prueba de periféricos.

        default: Se ejecuta cuando el valor de modo_actual no
        coincide con ninguno de los casos anteriores. Se asigna
        el valor 0 para establecer el modo normal como opción
        predeterminada.
        */
        switch (modo_actual)
        {
            case 0:
                Estado.modo = 0;
                break;

            case 1:
                Estado.modo = 1;
                break;

            case 2:
                Estado.modo = 2;
                break;

            case 3:
                Estado.modo = 3;
                break;

            default:
                Estado.modo = 0;
                break;
        }


        /* 5. Detectar cambios de modo */
        /*
        Este bloque utiliza la instrucción switch para evaluar
        el valor de Estado.modo y modificar la variable Estado.vista,
        que indica qué información debe mostrarse en la pantalla OLED.

        Además, cuando se selecciona el modo de prueba de periféricos,
        se registra el tiempo en el que comenzó dicha prueba.

        case 0: Se ejecuta cuando el monitor se encuentra en modo
        normal. Se asigna el valor 0 a Estado.vista para mostrar
        la pantalla correspondiente a este modo.

        case 1: Se ejecuta cuando está seleccionado el modo de
        muestreo rápido. Se asigna el valor 6 a Estado.vista para
        mostrar la información correspondiente a este modo.

        case 2: Se ejecuta cuando está seleccionado el modo de
        historial. Se asigna el valor 5 a Estado.vista para mostrar
        la vista relacionada con el historial de temperaturas.

        case 3: Se ejecuta cuando está seleccionado el modo de
        prueba de periféricos. Se asigna el valor 2 a Estado.vista
        para mostrar la pantalla de prueba. También se guarda el
        valor de Tiempo_Actual en inicio_modo3, con el propósito
        de registrar el momento en que inicia la prueba y calcular
        posteriormente el tiempo transcurrido.

        default: Se ejecuta cuando el valor de Estado.modo no
        coincide con ninguno de los casos establecidos. Se asigna
        el valor 0 a Estado.vista para regresar a la pantalla inicial.
        */
        if (Estado.modo != modo_anterior)
        {
            switch (Estado.modo)
            {
                case 0:
                    Estado.vista = 0;
                    break;

                case 1:
                    Estado.vista = 6;
                    break;

                case 2:
                    Estado.vista = 5;
                    break;

                case 3:
                    Estado.vista = 2;
                    inicio_modo3 = Tiempo_Actual;
                    break;

                default:
                    Estado.vista = 0;
                    break;
            }

            modo_anterior = Estado.modo;
        }


        /* 6. Procesar los eventos del botón */
        /*
        Este bloque comprueba si el monitor se encuentra en el modo
        normal antes de permitir el uso del botón. De esta manera,
        las funciones del botón solamente estarán disponibles cuando
        Estado.modo tenga el valor 0.

        if (Estado.modo == 0): Comprueba si el modo de operación
        actual es el modo normal.

        Procesar_Eventos_Boton(&Estado, &Entradas, Tiempo_Actual):
        llama a la función encargada de procesar las acciones
        realizadas con el botón. Recibe la dirección de las
        estructuras Estado y Entradas para consultar o modificar
        su información, además del tiempo actual para identificar
        y procesar los eventos del botón.
        */
        if (Estado.modo == 0)
        {
            Procesar_Eventos_Boton(
                &Estado,
                &Entradas,
                Tiempo_Actual
            );
        }


        /* 7. Comprobar si corresponde realizar una medición */
        /*
        Este bloque verifica si ya se cumplió el tiempo necesario
        para realizar una nueva medición de temperatura.

        Corresponde_Muestrear revisa las condiciones de muestreo
        según el modo de operación y los intervalos establecidos.

        Cuando se cumple la condición, el programa obtiene una
        lectura del sensor BMP280, actualiza los datos y comprueba
        si existe alguna condición de alarma.

        resultado_sensor_t Resultado = Adquirir_Temperatura():
        obtiene la lectura de temperatura del sensor y su estado
        de validez.

        Actualizar_Estado_Monitor(&Estado, &Resultado, Tiempo_Actual):
        actualiza la temperatura actual, los valores mínimo y máximo
        y el historial de mediciones.

        Actualizar_Estado_Alarma(&Estado): evalúa la temperatura
        y el estado del sensor para actualizar la condición de alarma.
        */
        if (Corresponde_Muestrear(
                &Estado,
                &Entradas,
                Tiempo_Actual))
        {
            resultado_sensor_t Resultado = Adquirir_Temperatura();

            Actualizar_Estado_Monitor(
                &Estado,
                &Resultado,
                Tiempo_Actual
            );

            Actualizar_Estado_Alarma(&Estado);
        }


        /* 8. Actualizar LED RGB y buzzer */
        /* En este bloque se actualiza el funcionamiento del LED RGB
        y del buzzer de acuerdo con el modo de operación seleccionado.

        Cuando el monitor se encuentra en el modo 3, utiliza el
        tiempo transcurrido desde que comenzó la prueba para controlar
        la secuencia de los periféricos.

        if (Estado.modo == 3): Comprueba si el monitor está en modo
        de prueba de periféricos.

        tiempo_prueba: Calcula el tiempo transcurrido desde el inicio
        del modo 3.

        Actualizar_LED_RGB(&Estado, tiempo_prueba): actualiza el LED
        RGB según la secuencia de prueba.

        Actualizar_Buzzer(&Estado, tiempo_prueba): controla el buzzer
        de acuerdo con el tiempo de la prueba.

        else: Se ejecuta cuando el monitor está en un modo diferente
        al de prueba.

        Actualizar_LED_RGB(&Estado, Tiempo_Actual): actualiza el LED
        RGB según la temperatura y las condiciones de alarma.

        Actualizar_Buzzer(&Estado, Tiempo_Actual): controla el buzzer
        de acuerdo con las condiciones de alarma.
        */
        if (Estado.modo == 3)
        {
            uint32_t tiempo_prueba =
                Tiempo_Actual - inicio_modo3;

            Actualizar_LED_RGB(&Estado, tiempo_prueba);
            Actualizar_Buzzer(&Estado, tiempo_prueba);
        }
        else
        {
            Actualizar_LED_RGB(&Estado, Tiempo_Actual);
            Actualizar_Buzzer(&Estado, Tiempo_Actual);
        }


        /* 9. Actualizar pantalla OLED */
        /* Este bloque comprueba si ya transcurrió el intervalo
        establecido para actualizar la pantalla OLED.

        Corresponde_Actualizar_OLED determina si corresponde
        realizar una actualización.

        Controlar_Pantalla(&Estado) llama a la función encargada
        de controlar la pantalla OLED. Recibe la dirección de
        Estado para consultar la información del monitor y
        mostrar los datos de acuerdo con el valor de Estado.vista.
        */
        if (Corresponde_Actualizar_OLED(Tiempo_Actual))
        {
            Controlar_Pantalla(&Estado);
        }


        /* 10. Diagnóstico por monitor serie */
        /* Este bloque permite visualizar información del funcionamiento
        del monitor a través del monitor serie.

        La condición establece que los mensajes se muestren cada
        1000 milisegundos, es decir, cada segundo.

        Se presentan datos como el tiempo de funcionamiento,
        la temperatura actual, los valores mínimo y máximo,
        el modo seleccionado, la vista de la pantalla, el estado
        de la alarma, el periodo de muestreo y la activación del buzzer.

        if (Tiempo_Actual - ultimo_diag >= 1000):
        comprueba si ha transcurrido al menos un segundo desde
        el último diagnóstico.

        ultimo_diag = Tiempo_Actual:
        actualiza el tiempo del último diagnóstico mostrado.

        Los printf() siguientes muestran el tiempo de funcionamiento,
        las temperaturas, el modo de operación, la vista OLED,
        el estado de la alarma, el periodo de muestreo y el estado
        del buzzer.
        */
        if (Tiempo_Actual - ultimo_diag >= 1000)
        {
            ultimo_diag = Tiempo_Actual;

            printf("\n--- DIAGNOSTICO ESP32 (%lu ms) ---\n",
                   (unsigned long)Tiempo_Actual);

            printf("Temp: %.2f C | Min: %.2f C | Max: %.2f C\n",
                   Estado.temperatura,
                   Estado.temp_min,
                   Estado.temp_max);

            printf("Modo DIP: %u | Vista OLED: %u | Alarma: %u\n",
                   (unsigned int)Estado.modo,
                   (unsigned int)Estado.vista,
                   (unsigned int)Estado.alarma);

            printf("Periodo de muestreo: %lu ms\n",
                   (unsigned long)Estado.intervalo_muestreo_ms);

            printf("Buzzer Activo: %s\n",
                   Estado.buzzer_activo ? "SI" : "NO");
        }


        /* 11. Esperar antes de repetir el ciclo */
        /* Este último bloque establece una pequeña pausa antes de que
        el programa vuelva a ejecutar las instrucciones del ciclo while.

        Esto permite controlar la frecuencia con la que se repiten
        las operaciones y evita que el ciclo se ejecute continuamente
        sin ninguna espera.
        */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
