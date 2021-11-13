/* ========================================
 * UART Traffic Generator for Lab 4
 * Core 0 is not used in this design.
*/
#include "project.h"

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 

    for(;;)
    {
    }
}

/* [] END OF FILE */
