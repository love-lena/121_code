/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/

#include "project.h"

const char TRANSMIT_CHAR = 0x5a;

void UART_1_ISR(void) {
    uint32_t txStatus = Cy_SCB_UART_GetTxFifoStatus(UART_1_HW);
    
    if(txStatus & CY_SCB_UART_TX_NOT_FULL) {
        //add byte
        Cy_SCB_UART_Put(UART_1_HW, TRANSMIT_CHAR);
    }
    
    //Clear FIFO status
    Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_NOT_FULL);
}


int main(void)
{
    
    //UART_1 setup
    cy_en_scb_uart_status_t init_status = Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    if(init_status != CY_SCB_UART_SUCCESS) {
        //bad things
    }
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
