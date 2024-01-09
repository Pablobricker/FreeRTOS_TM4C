#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#include "inc/hw_memmap.h"
#include "inc/tm4c1294ncpdt.h"

extern SemaphoreHandle_t Semaphore1;
extern SemaphoreHandle_t Semaphore2;

uint32_t result;
unsigned int Delay;

//Interrupcion del boton en la tarjeta
//Pide la creacion de la tarea 4
void GPIOPortJ_Handler(void)
{
    BaseType_t priorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(Semaphore2, &priorityTaskWoken);
    portYIELD_FROM_ISR(priorityTaskWoken);
    GPIO_PORTJ_AHB_ICR_R = 0x01;      // bandera0 de confirmación

   }

//Interrupcion del ADC conectado al potenciometro
//Pide la creacion de la tarea 1
void ADC0_HandleSS3(void)
{
    BaseType_t priorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(Semaphore1, &priorityTaskWoken);
    portYIELD_FROM_ISR(priorityTaskWoken);
    result = (ADC0_SSFIFO3_R & 0xFFF);  // Resultado en FIFO3 se asigna a variable "result"
    ADC0_ISC_R = 0x0008;                // Limpia la bandera RIS del ADC0

}

//Tarea 1: Leer el valor del voltaje en el potenciometro
void Task1 (void* args){

    while(1){
        xSemaphoreTake(Semaphore1, portMAX_DELAY);

        result = (ADC0_SSFIFO3_R & 0xFFF);  // Resultado en FIFO3 se asigna a "result"
    }
}

//Tarea 2:Despliega en 8 bits resultado ADC en PTO K
void Task2 (void* args){

    while(1){
        GPIO_PORTK_DATA_R = (result>>4);
    }
}

//Tarea 3: Ajusta el angulo de un servomotor conectado a D0
//segun el valor del potenciometro
void Task3 (void* args){

    while(1){
        Delay = (uint32_t)(result);
        unsigned int i;
        if(result<3200){//Tope del CT=3200
        //PWM generado por retardos en software
        GPIO_PORTD_AHB_DATA_R &= ~0x01;
        for(i=0; i<5000-Delay; ++i){}

        GPIO_PORTD_AHB_DATA_R |= 0x01;
        for(i=0; i<Delay; ++i){}

        }
    }

}

//Tarea 4: rutina de switcheo de LEDs de la tarjeta N0,N1
void Task4 (void* args){

    while(1){
        xSemaphoreTake(Semaphore2, portMAX_DELAY);
        int i=0;
        int j=0;
        for(i=0; i<5; i++){
        vTaskDelay(pdMS_TO_TICKS(800));
        GPIO_PORTN_DATA_R = GPIO_PORTN_DATA_R^0x03; // conmuta encendido de leds
        }
    }
}

//Tarea 5: Ajusta el brillo de un LED conectado a D4
//segun el valor del potenciometro
void Task5 (void* args){

    while(1){
        TIMER3_CTL_R=0X00000000; //DESHABILITA TIMER EN LA CONFIGURACION
        TIMER3_TAMATCHR_R =(10*result); // CT del PWM generado por timer
        TIMER3_CTL_R |= 0X00000001; //HABILITA TIMER A

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
