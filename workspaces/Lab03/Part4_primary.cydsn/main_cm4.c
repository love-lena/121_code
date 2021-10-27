/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/

#include "project.h"
#include <stdio.h>

uint32_t data_counter = 0;


void UART_1_ISR(void) {
    
    uint32_t txStatus = Cy_SCB_UART_GetTxFifoStatus(UART_1_HW);
    
    //Send a byte if TX is empty
    if((txStatus & CY_SCB_UART_TX_EMPTY)) {
        Cy_SCB_UART_Put(UART_1_HW, data_counter++);
        
        //Generate data in repeating pattern
        //0x00 - 0xff
        if(data_counter>255)
            data_counter=0;
        
        Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_EMPTY);
    } 
 
}



int main(void)
{

    //UART_1 setup
    Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    Cy_SCB_UART_Enable(UART_1_HW);
    
    //UART ISR setup
    Cy_SysInt_Init(&UART_1_INT_cfg, UART_1_ISR);
    NVIC_EnableIRQ(UART_1_INT_cfg.intrSrc);
    __enable_irq();
    
    
    for(;;)
    {
        
    }
}

/* [] END OF FILE */
