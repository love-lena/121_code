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



int main(void)
{
    init_uart_printf();
    
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        char sw2_on[2];
        sprintf(sw2_on, "%u", Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM));
        uart_printf(sw2_on);
        uart_printf("\n\r");
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
