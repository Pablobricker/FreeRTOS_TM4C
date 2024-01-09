/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
 * This project provides two demo applications.  A simple blinky style project,
 * and a more comprehensive test and demo application.  The
 * configCREATE_SIMPLE_TICKLESS_DEMO setting (defined in FreeRTOSConfig.h) is
 * used to select between the two.  The simply blinky demo is implemented and
 * described in main_blinky.c.  The more comprehensive test and demo application
 * is implemented and described in main_full.c.
 *
 * The comprehensive demo uses FreeRTOS+CLI to create a simple command line
 * interface through a UART.
 *
 * The blinky demo uses FreeRTOS's tickless idle mode to reduce power
 * consumption.  See the notes on the web page below regarding the difference
 * in power saving that can be achieved between using the generic tickless
 * implementation (as used by the blinky demo) and a tickless implementation
 * that is tailored specifically to the MSP432.
 *
 * This file implements the code that is not demo specific.
 *
 * See http://www.FreeRTOS.org/TI_MSP432_Free_RTOS_Demo.html for instructions.
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdbool.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Hardware includes */
#include "inc/tm4c1294ncpdt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"


static void prvSetupHardware( void );

extern void Task1 (void*);
extern void Task2 (void*);
extern void Task3 (void*);
extern void Task4 (void*);
extern void Task5 (void*);

SemaphoreHandle_t Semaphore1;
SemaphoreHandle_t Semaphore2;

/*-----------------------------------------------------------*/

int main( void )
{
	/* Prepare the hardware to run this demo. */
	prvSetupHardware();

	xTaskCreate(&Task1, "Tarea 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(&Task2, "Tarea 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(&Task3, "Tarea 3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(&Task4, "Tarea 4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(&Task5, "Tarea 5", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	Semaphore1 = xSemaphoreCreateBinary();
	Semaphore2 = xSemaphoreCreateBinary();
	vTaskStartScheduler();
	return 0;
}
/*-----------------------------------------------------------*/
void GPIO_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x00001338; // (a) activa el reloj para el puerto D,F,J,E,N,K(p.p382)
       while((SYSCTL_PRGPIO_R & 0x00001619) == 0){};//Espera a que el reloj se estabilice

    //Puerto E entrada analogica de potenciometro
    GPIO_PORTE_AHB_DIR_R = 0x00;    // 2) PE4 entrada (analogica)
    GPIO_PORTE_AHB_AFSEL_R |= 0x10; // 3) Habilita Funcion Alterna de PE4
    GPIO_PORTE_AHB_DEN_R = 0x00;    // 4) Deshabilita Funcion Digital de PE4
    GPIO_PORTE_AHB_AMSEL_R |= 0x10; // 5) Habilita Funcion Analogica de PE4

    //Puerto K para fila de LEDS que indica el valor analogico
    GPIO_PORTK_DIR_R  = 0xFF; // puerto K todos los bits salidas
    GPIO_PORTK_DEN_R  = 0xFF;   // todos digitales.
    GPIO_PORTK_DATA_R = 0x00;


    //Puerto F para salida PWM por software
      // Bits 4 y 0 del PTO F como salidas, los demas bits entradas
      GPIO_PORTF_AHB_DIR_R |= 0x01;       // (b)

      // Habilita el modo Digital de los pines 4 y 0
      GPIO_PORTF_AHB_DEN_R |= 0x01;           // (c)

  //Configuracion del puerto D para PWM por hardware 4 y software 0
  GPIO_PORTD_AHB_DEN_R |= 0x11; //BIT 4,0 DIGITAL
  GPIO_PORTD_AHB_DIR_R |= 0x11; //bit 4,0 SALIDA
  GPIO_PORTD_AHB_DATA_R |= 0x00; // SALIDA A 0
  GPIO_PORTD_AHB_AFSEL_R |= 0x10; //FUNCION ALTERNA EN BIT 4
  GPIO_PORTD_AHB_PCTL_R |= 0x00030000; //DIRIGIDO A T3CCP0


//Puerto J para interrupciones por flanco de bajada
  GPIO_PORTJ_AHB_DIR_R &= ~0x01;       // (c) PJ0 dirección entrada - boton SW1
  GPIO_PORTJ_AHB_DEN_R |= 0x01;        //     PJ0 se habilita
  GPIO_PORTJ_AHB_PUR_R |= 0x01;        //     habilita weak pull-up on PJ1
  GPIO_PORTJ_AHB_IS_R &= ~0x01;        // (d) PJ1 es sensible por flanco
  GPIO_PORTJ_AHB_IBE_R &= ~0x01;       //     PJ1 no es sensible a dos flancos
  GPIO_PORTJ_AHB_IEV_R &= ~0x01;       //     PJ1 detecta eventos de flanco de bajada
  GPIO_PORTJ_AHB_ICR_R = 0x01;         // (e) limpia la bandera 0
  GPIO_PORTJ_AHB_IM_R |= 0x01;         // (f) Se desenmascara la interrupcion PJ0 y se envia al controlador de interrupciones
  NVIC_PRI12_R = (NVIC_PRI12_R&0x00FFFFFF)|0x00000000; // (g) prioridad 0  (pag 159)
  NVIC_EN1_R= 0x180000;          //(h) habilita la interrupción 51 y 52 en NVIC (Pag. 154) DATO DIRECTO EN HEXA

  //Puerto N para rutina de LEDS soldados a la tarjeta
  GPIO_PORTN_DIR_R |= 0x0F;    // puerto N de salida
  GPIO_PORTN_DEN_R |= 0x0F;    // puerto N habilitado
  GPIO_PORTN_DATA_R = 0x02;

}




  void PWM_init(){
      //Configuración del Timer 3
      SYSCTL_RCGCTIMER_R |= 0X08; //HABILITA TIMER 4
      while ((SYSCTL_PRGPIO_R & 0X0008) == 0){};  // reloj listo?

      TIMER3_CTL_R=0X00000000; //DESHABILITA TIMER EN LA CONFIGURACION
      TIMER3_CFG_R= 0X0000004; //CONFIGURAR PARA 16 BITS
      TIMER3_TAMR_R= 0X0000000A; //CONFIGURAR PARA MODO PWM, MODO PERIODICO CUENTA HACIA ABAJO

      TIMER3_TAILR_R= 45000; // VALOR DE RECARGA 1 KHz
      TIMER3_TAMATCHR_R =40000; // 88 %
      TIMER3_TAPR_R= 0X00; // RELOJ 16 MHZ / 16 = 1 MHz
      TIMER3_CTL_R |= 0X00000001; //HABILITA TIMER A
  }
void ADC0_init(void){
    SYSCTL_RCGCADC_R  = 0x01;   // 6) Habilita reloj para logica de ADC0
    while((SYSCTL_PRADC_R & 0x01)==0);

    ADC0_PC_R = 0x01;       //  7) Configura para 129Ksamp/s --> Fconv/8
    ADC0_SSPRI_R = 0x3210;  //  8) SS3 con la mas baja prioridad
    ADC0_ACTSS_R = 0x0000;  //  9) Deshabilita SS3 antes de cambiar configuracion de registros
    ADC0_EMUX_R = 0x5000;   // 10) iniciar muestreo sel SS3 por Timer
    ADC0_SSEMUX3_R = 0x00;    // 11) posibles Entradas AIN(15:0)
    ADC0_SSMUX3_R = 0x09;    //     en especifico canal AIN9-> PE4
    ADC0_SSCTL3_R = 0x0006; // 12) no TS0, D0 , yes IE0 END0
    ADC0_IM_R = 0x0008;     // 13) Habilita interrupciones de SS3
    ADC0_ACTSS_R |= 0x0008; // 14) Habilita SS3

    NVIC_EN0_R = 1 << 17;  // Habilita Interrupcion del ADC0 Secuenciador 3

    SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_PLLPWR;  // encender PLL
    while((SYSCTL_PLLSTAT_R&0x01)==0);    // espera a que el PLL fije su frecuencia
    SYSCTL_PLLFREQ0_R &= ~SYSCTL_PLLFREQ0_PLLPWR; // apagar PLL

    ADC0_ISC_R = 0x0008;      // Se recomienda Limpia la bandera RIS del ADC0

    /*Configuracion del Timer*/

    SYSCTL_RCGCTIMER_R = 0x01;                // _Activa TIMER0
    while((SYSCTL_PRTIMER_R & 0x01)==0);

    TIMER0_CTL_R = 0X00000000;            //_disable timer0A during setup
    TIMER0_CFG_R = TIMER_CFG_16_BIT;          // _configure TIMER0 for 16-bit timer mode

    TIMER0_CTL_R |= TIMER_CTL_TAOTE;     // _enable timer0A trigger to ADC
    TIMER0_ADCEV_R = 0x1;                // el trigger de ADC es con Time Out de Timer

    TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;   // _configure for periodic mode
    TIMER0_TAPR_R = 0;                        // prescale value for trigger
    TIMER0_TAILR_R = 4000;                    // start value for trigger
    TIMER0_IMR_R &= ~TIMER_IMR_TATOIM;        // _disable timeout (rollover) interrupt
    TIMER0_CTL_R |= TIMER_CTL_TAEN;           // _enable timer0A 16-b, periodic, no interrupts

}

static void prvSetupHardware( void )
{
    DisableInterrupts();
    GPIO_Init();  // Inicializa puertos
    ADC0_init();  // Inicializa ADC e interrupciones
    PWM_init();   // Inicializa timer para PWM
    EnableInterrupts();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	IntMasterDisable();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	IntMasterDisable();
	for( ;; );
}
/*-----------------------------------------------------------*/

void *malloc( size_t xSize )
{
	/* There should not be a heap defined, so trap any attempts to call
	malloc. */
	IntMasterDisable();
	for( ;; );
}
/*-----------------------------------------------------------*/

void DisableInterrupts(void){
    __asm(  " CPSID I ");
}

void EnableInterrupts(void){
    __asm( "  CPSIE  I  ");
}

