/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/

#include "project.h"
#include <stdio.h>

//Set up LCD functions
void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

int main(void)
{
    //Wait 50ms before initializing LCD
    Cy_SysLib_Delay(50);
    lcd_init();
    
    //UART_1 setup
    Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    Cy_SCB_UART_Enable(UART_1_HW);
    
    int rx_fifo_framing_int_count = 0;
    int rx_fifo_parity_int_count = 0;
    
    for(;;)
    {
        
        //Poll RX status
        uint32_t rxStatus = Cy_SCB_UART_GetRxFifoStatus(UART_1_HW);
    
        //Record FRAME errors
        if(rxStatus & CY_SCB_UART_RX_ERR_FRAME) {
            rx_fifo_framing_int_count++;
            Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_ERR_FRAME);
        }
        
        //Record PARITY errors
        if(rxStatus & CY_SCB_UART_RX_ERR_PARITY) {
            rx_fifo_framing_int_count++;
            Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_ERR_PARITY);
        }

        //Read from RX
        if(rxStatus & CY_SCB_UART_RX_NOT_EMPTY) {

            uint32_t recieve_bit = Cy_SCB_UART_Get(UART_1_HW);
            
            Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_NOT_EMPTY);
            
            //Print read byte to LCD
            lcd_cursor(0,14);
            char msg_rec[3];
            sprintf(msg_rec, "%02x", recieve_bit);
            lcd_write(msg_rec, sizeof(msg_rec));
            
        }

        //Print to LCD
        lcd_cursor(0,0);
        char msg_tx[13];
        sprintf(msg_tx,  "frame:  %04d", rx_fifo_framing_int_count);
        lcd_write(msg_tx, sizeof(msg_tx));
        
        lcd_cursor(1,0);
        char msg_rx[13];
        sprintf(msg_rx, "parity: %04d", rx_fifo_parity_int_count);
        lcd_write(msg_rx, sizeof(msg_rx));
        
        
    }
}

/* [] END OF FILE */
