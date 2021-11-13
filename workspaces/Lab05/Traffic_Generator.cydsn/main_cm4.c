/* ========================================
 * UART Traffic Generator for Lab 5
 * Generates 4 serial bitstreams to test soft UARTs.
 * Default data rate id 9600 baud.
 * Default data pattern is the sequence of bytes 00, 01, 02, ...
 * Configure baud rate and data pattern as needed.
 * ========================================
*/
#include "project.h"

int main(void)
{
    uint8_t uart_tx_data[4]; 
    
    __enable_irq(); /* Enable global interrupts. */
    /* Initialize UARTs */
    Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context); 
    Cy_SCB_UART_Init(UART_2_HW, &UART_2_config, &UART_2_context); 
    Cy_SCB_UART_Init(UART_3_HW, &UART_3_config, &UART_3_context); 
    Cy_SCB_UART_Init(UART_4_HW, &UART_4_config, &UART_4_context); 
    Cy_SCB_UART_Enable(UART_1_HW);
    Cy_SCB_UART_Enable(UART_2_HW);
    Cy_SCB_UART_Enable(UART_3_HW);
    Cy_SCB_UART_Enable(UART_4_HW);
    for(;;)
    {
      if (Cy_SCB_UART_GetTxFifoStatus(UART_1_HW) & CY_SCB_UART_TX_NOT_FULL){
        Cy_SCB_UART_Put(UART_1_HW, uart_tx_data[0]++);
        Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_NOT_FULL);  
      }
      if (Cy_SCB_UART_GetTxFifoStatus(UART_2_HW) & CY_SCB_UART_TX_NOT_FULL){
        Cy_SCB_UART_Put(UART_2_HW, uart_tx_data[1]++);
        Cy_SCB_UART_ClearTxFifoStatus(UART_2_HW, CY_SCB_UART_TX_NOT_FULL);  
      }
      if (Cy_SCB_UART_GetTxFifoStatus(UART_3_HW) & CY_SCB_UART_TX_NOT_FULL){
        Cy_SCB_UART_Put(UART_3_HW, uart_tx_data[2]++);
        Cy_SCB_UART_ClearTxFifoStatus(UART_3_HW, CY_SCB_UART_TX_NOT_FULL);  
      }  
          
      if (Cy_SCB_UART_GetTxFifoStatus(UART_4_HW) & CY_SCB_UART_TX_NOT_FULL){
        Cy_SCB_UART_Put(UART_4_HW, uart_tx_data[3]++);
        Cy_SCB_UART_ClearTxFifoStatus(UART_4_HW, CY_SCB_UART_TX_NOT_FULL);  
      }
        
    }
}

/* [] END OF FILE */
