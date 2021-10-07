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

int regToggleComponentLEDs() {
    
    char value;
    // Red
    value = 0x6;
    
    // Green
    //value = 0x5;

    // Blue
    //value = 0x3;
    
    for(;;)
    {
        
        
        LED_Reg_Write(value);
        LED_Reg_Write(0xf);
        
    }
}
/* [] END OF FILE */
