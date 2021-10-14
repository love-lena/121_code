/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

void init_uart_printf(void);
void uart_printf(char *print_string);

void Echo_timer_ISR(void) {
    uart_printf("Echo int\n\r");
    //what i want to do
    uint32_t clockedTime = Echo_timer_GetCounter();
    uint32_t baseTime = 65535;
    uint32_t time = baseTime - clockedTime;

    //constant based on d=vt/2 where v = 340m/s, d=1m
    uint32_t const timeTo1m = 5882;
    uint32_t const cc2m = 0xA21FE80;
    
    char str[32];
    sprintf(str, "%lu\n\r", time);
    uart_printf(str);
    
    if(time < timeTo1m) {
        //IN RANGE - turn on LED
        
        //constant based on 1MHz clock, 
        //d=1.7*10^-8*cc
        uint32_t const fiveSeconds = 39062;
        Cy_TCPWM_Counter_SetCounter(Five_second_HW, Five_second_CNT_NUM, 0);
        Cy_TCPWM_TriggerStart(Five_second_HW, Five_second_CNT_NUM);
    }
    
    Cy_TCPWM_TriggerStopOrKill(Echo_timer_HW, Echo_timer_CNT_MASK);
    //Echo_timer_SetCounter(65535);
    uint32_t interrupts = Cy_TCPWM_GetInterruptStatusMasked(Echo_timer_HW, 0);
    Cy_TCPWM_ClearInterrupt(Echo_timer_HW, 0, interrupts);
}

int main(void)
{
    init_uart_printf();
    
    Cy_SysInt_Init(&Echo_timer_int_cfg, Echo_timer_ISR);
    NVIC_EnableIRQ(Echo_timer_int_cfg.intrSrc);
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    (void)Cy_TCPWM_Counter_Init(Trigger_pulse_HW, Trigger_pulse_CNT_NUM, &Trigger_pulse_config);
    Cy_TCPWM_Enable_Multiple(Trigger_pulse_HW, Trigger_pulse_CNT_MASK);
    
    (void)Cy_TCPWM_Counter_Init(Echo_timer_HW, Echo_timer_CNT_NUM, &Echo_timer_config);
    Cy_TCPWM_Enable_Multiple(Echo_timer_HW, Echo_timer_CNT_MASK);
    
    (void)Cy_TCPWM_Counter_Init(Half_second_HW, Half_second_CNT_NUM, &Half_second_config);
    Cy_TCPWM_Enable_Multiple(Half_second_HW, Half_second_CNT_MASK);
    
    (void)Cy_TCPWM_Counter_Init(Five_second_HW, Five_second_CNT_NUM, &Five_second_config);
    Cy_TCPWM_Enable_Multiple(Five_second_HW, Five_second_CNT_MASK);
    
    
    for(;;)
    {
        
        
        if(Echo_timer_GetCounter() != 65535) {
            int capture = Echo_timer_GetCounter();
            char str2[32];
            sprintf(str2, "%d\n\r", capture);
            uart_printf(str2);
        }
        /*
        if(Trigger_Reg_Read() != 0) {
            int thingy = Trigger_Reg_Read();
            char str[32];
            sprintf(str, "%d\n\r", thingy);
            uart_printf(str);
        }
        */
        if(Half_second_GetCounter() == 0) {
            uart_printf("trigger pulse\n\r");
            Trigger_Reg_Write(1);
            Trigger_pulse_SetCounter(0);
            Cy_TCPWM_TriggerStart(Trigger_pulse_HW, Trigger_pulse_CNT_MASK); //Start timer to turn off pulse
            Echo_timer_SetCounter(65535);
            Cy_TCPWM_TriggerStart(Echo_timer_HW, Echo_timer_CNT_MASK);
            
            Half_second_SetCounter(1);
            Cy_TCPWM_TriggerStart(Half_second_HW, Half_second_CNT_MASK);
        }
        
        
        if(Five_second_GetCompare0() == 0) {
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        } else {
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
        }
        
        
    }
}






/* [] END OF FILE */
