#include <stdio.h>
#include <stdint.h>

#include "inc/tm4c1294ncpdt.h"
//#define N 1000
//volatile uint32_t n;

uint32_t CT;

void GPIOInit(void)
{
    SYSCTL_RCGCGPIO_R |= 0x00000F19; //Activa el reloj de los puertos A, D, E, J, K, L y M (p. 382)
    while((SYSCTL_PRGPIO_R & 0x00001619) == 0){};//Espera a que el reloj se estabilice

    //Configuraci�n del puerto J
    GPIO_PORTJ_AHB_DIR_R |= 0B00000001; // Puerto PJ1 como entrada, PJ0 como salida.
    GPIO_PORTJ_AHB_DEN_R = 0x03; // 2 primeros bits habilitaci�n digital. (p.781)
    GPIO_PORTJ_AHB_DATA_R =0x00;

    //Configuraci�n del puerto D
    GPIO_PORTD_AHB_DIR_R = 0x0F; // Puerto D como salida. (p.760)
    GPIO_PORTD_AHB_DEN_R = 0x0F; // 4 primeros bits como salidas digitales.
    GPIO_PORTD_AHB_DATA_R =0x00;

    //Configuraci�n del puerto L
    GPIO_PORTL_DIR_R = 0x0F; // Puerto L como salida.
    GPIO_PORTL_DEN_R = 0x0F; // 4 primeros bits como salidas digitales.
    GPIO_PORTL_DATA_R =0x00;

    //Configuraci�n del puerto E
    GPIO_PORTE_AHB_DIR_R = 0x00;    // PE4 entrada (anal�gica)
    GPIO_PORTE_AHB_AFSEL_R |= 0x10; // Habilita Funci�n Alterna de PE4
    GPIO_PORTE_AHB_DEN_R = 0x00;    // Deshabilita Funci�n Digital de PE4
    GPIO_PORTE_AHB_AMSEL_R |= 0x10; // Habilita Funci�n Anal�gica de PE4

    //Configuraci�n para muestreo ADC
    SYSCTL_RCGCADC_R  = 0x01;   // Habilita reloj para l�gica de ADC0
    while((SYSCTL_PRADC_R&0x01)==0);

    ADC0_PC_R = 0x01;       // Configura para 125Ksamp/s
    ADC0_SSPRI_R = 0x0123;  // SS3 con la m�s alta prioridad
    ADC0_ACTSS_R = 0x0000;  // Deshabilita SS3 antes de cambiar configuraci�n de registros
    ADC0_EMUX_R = 0x0000;   // Se configura SS3 para iniciar muestreo por software
    ADC0_SSEMUX3_R = 0x00;  // Entradas AIN(15:0)
    ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 9; // canal AIN9 (PE4)
    ADC0_SSCTL3_R = 0x0006; // SI: AIN, Habilitaci�n de INR3, Fin de secuencia; No:muestra diferencial
    ADC0_IM_R = 0x0000;     // Deshabilita interrupciones de SS3
    ADC0_ACTSS_R |= 0x0008; // Habilita SS3

    SYSCTL_PLLFREQ0_R |= SYSCTL_PLLFREQ0_PLLPWR;    // encender PLL
    while((SYSCTL_PLLSTAT_R&0x01)==0);              // espera a que el PLL fije su frecuencia
    SYSCTL_PLLFREQ0_R &= ~SYSCTL_PLLFREQ0_PLLPWR;   // apagar PLL

    ADC0_ISC_R = 0x0008;                // Se recomienda Limpia la bandera RIS del ADC0

    //Configuraci�n del puerto K
    GPIO_PORTK_DIR_R  = 0xFF;   // Puerto K como salida.
    GPIO_PORTK_DEN_R  = 0xFF;   // Todas las salidas digitales.
    GPIO_PORTK_DATA_R = 0x00;

    //Configuraci�n del puerto D para PWM
    GPIO_PORTD_AHB_DEN_R |= 0x10; //BIT 4 DIGITAL
    GPIO_PORTD_AHB_DIR_R |= 0x10; //bit 4 SALIDA
    GPIO_PORTD_AHB_DATA_R |= 0x00; // SALIDA A 0
    GPIO_PORTD_AHB_AFSEL_R |= 0x10; //FUNCION ALTERNA EN BIT 4
    GPIO_PORTD_AHB_PCTL_R |= 0x00030000; //DIRIGIDO A T3CCP0

    //Configuraci�n del Timer 3
    SYSCTL_RCGCTIMER_R |= 0X08; //HABILITA TIMER 4
    while ((SYSCTL_PRGPIO_R & 0X0008) == 0){};  // reloj listo?

    TIMER3_CTL_R=0X00000000; //DESHABILITA TIMER EN LA CONFIGURACION
    TIMER3_CFG_R= 0X0000004; //CONFIGURAR PARA 16 BITS
    TIMER3_TAMR_R= 0X0000000A; //CONFIGURAR PARA MODO PWM, MODO PERIODICO CUENTA HACIA ABAJO

    TIMER3_TAILR_R= 16000; // VALOR DE RECARGA 1 KHz
    TIMER3_TAMATCHR_R =8000; // 50 %
    TIMER3_TAPR_R= 0X00; // RELOJ 16 MHZ / 16 = 1 MHz
    TIMER3_CTL_R |= 0X00000001; //HABILITA TIMER A
    CT=8000;

}
