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

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    Cy_TCPWM_PWM_Init(PWM_2_HW, PWM_2_CNT_NUM, &PWM_2_config);
    Cy_TCPWM_Enable_Multiple(PWM_2_HW, PWM_2_CNT_MASK);
    Cy_TCPWM_TriggerStart(PWM_2_HW, PWM_2_CNT_MASK);
    
    ADC_1_Start();
    ADC_1_StartConvert();
    

    for(;;)
    {
        /* Place your application code here. */
        uint16 output = ADC_1_GetResult16(0);
        uint16 mask = 2047;
        uint16 value = (output < 2048) ? 0 : output & mask;
        
        Cy_TCPWM_PWM_SetCompare0(PWM_2_HW, PWM_2_CNT_NUM, value);
    }
}

/* [] END OF FILE */
