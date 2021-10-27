/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/

#include "project.h"
#include <stdio.h>

// block_size: how many bytes in our source block
const int BLOCK_SIZE = 4096;
uint8 src[4096];
uint8 dst[4096];
int tx_counter = 0;
int rx_counter = 0;

int transfer_complete = 0;

int tx_fifo_empty_int_count = 0;
int rx_fifo_overflow_count = 0;

void readISR(void) {
    //Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 0);
    uint32_t interrupts = Cy_TCPWM_GetInterruptStatusMasked(half_milli_HW, 0);
    Cy_TCPWM_ClearInterrupt(half_milli_HW, 0, interrupts);
    uint32_t rxStatus = Cy_SCB_UART_GetRxFifoStatus(UART_1_HW);
    
    if(rxStatus & CY_SCB_UART_RX_OVERFLOW) {
        rx_fifo_overflow_count++;
    }
        
    if(rxStatus & CY_SCB_UART_RX_NOT_EMPTY) {
        //read byte
        dst[rx_counter++] = Cy_SCB_UART_Get(UART_1_HW);
        
        Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_NOT_EMPTY);
    }
    
    if(rx_counter >= 4096) {
        half_milli_TriggerStop();
        half_milli_SetCounter(500);
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 1);
        transfer_complete = 1;
        UART_1_HW->INTR_TX_MASK = 0;
        UART_1_HW->INTR_RX_MASK = 0;
    }
}

void UART_1_ISR(void) {
    //Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
    uint32_t txStatus = Cy_SCB_UART_GetTxFifoStatus(UART_1_HW);
    
    if((txStatus & CY_SCB_UART_TX_EMPTY) && (UART_1_HW->INTR_TX_MASK)!=0) {
        if(!transfer_complete)
            tx_fifo_empty_int_count++;
        
        //add byte
        Cy_SCB_UART_Put(UART_1_HW, src[tx_counter++]);
        
        Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_EMPTY);
    } 
    if(tx_counter >= 4096) {
        //half_milli_TriggerStop();
        //half_milli_SetCounter(500);
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 1);
        //transfer_complete = 1;
        UART_1_HW->INTR_TX_MASK = 0;
        //UART_1_HW->INTR_RX_MASK = 0;
    }
    
}

//Set up LCD functions
void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);


int main(void)
{
    __enable_irq();
    
    //Wait 50ms before initializing LCD
    Cy_SysLib_Delay(50);
    lcd_init();
    
    //Reset error LED
    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
    Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 1);
    
    //Inizialize src to values and dst to all zeros
    for(int i = 0; i < BLOCK_SIZE; i++) {
        src[i] = i % 256;
        dst[i] = 0;
    }

    //UART_1 setup
    cy_en_scb_uart_status_t init_status = Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    if(init_status != CY_SCB_UART_SUCCESS) {
        //bad things
    }
    Cy_SCB_UART_Enable(UART_1_HW);
    
    //UART ISR setup
    Cy_SysInt_Init(&UART_1_INT_cfg, UART_1_ISR);
    NVIC_EnableIRQ(UART_1_INT_cfg.intrSrc);
    
    //0.5 millisecond timer
    half_milli_Start();
    Cy_SysInt_Init(&half_milli_int_cfg, readISR);
    NVIC_EnableIRQ(half_milli_int_cfg.intrSrc);
    
    for(;;)
    {
        if(transfer_complete == 1) { //All data recieved
            
            int error_count = 0;
            for(int i = 0; i < BLOCK_SIZE; i++) {
                if(dst[i] != src[i]) {
                    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
                    error_count++;
                }
            }
            
            
            lcd_cursor(0,0);
            char msg_rx[11];
            sprintf(msg_rx, "ovf: %05d", rx_fifo_overflow_count);
            lcd_write(msg_rx, sizeof(msg_rx));
            
            lcd_cursor(1,0);
            char msg_bps[] = "err:";
            lcd_write(msg_bps, sizeof(msg_bps));
            
            lcd_cursor(1,5);
            char msg_bps_num[6];
            sprintf(msg_bps_num, "%05d", error_count);
            lcd_write(msg_bps_num, sizeof(msg_bps_num));
            
            transfer_complete = 2;
        }
    }
}

/* [] END OF FILE */
