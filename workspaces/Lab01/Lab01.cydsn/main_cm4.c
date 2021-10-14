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
#include "compAPI.h"
#include "regLED.h"

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */

    // Need to switch TopDesign layout to switch between these
    //compToggleComponentLEDs();
    regToggleComponentLEDs();
}

/* [] END OF FILE */
