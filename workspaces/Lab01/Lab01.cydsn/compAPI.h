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

int compToggleComponentLEDs() {
    for(;;)
    {
        // Red
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        
        // Green
        //Cy_GPIO_Write(LED_0_PORT, LED_1_NUM, 0);
        //Cy_GPIO_Write(LED_0_PORT, LED_1_NUM, 1);
        
        // Blue
        //Cy_GPIO_Write(LED_0_PORT, LED_2_NUM, 0);
        //Cy_GPIO_Write(LED_0_PORT, LED_2_NUM, 1);
    }
}
/* [] END OF FILE */
