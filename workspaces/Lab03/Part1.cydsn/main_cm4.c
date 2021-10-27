/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/

#include "project.h"
#include <stdio.h>

// block_size: how many bytes in our source block
#define BLOCK_SIZE 4096

uint8 src[BLOCK_SIZE];
uint8 dst[BLOCK_SIZE];
int tx_counter = 0;
int rx_counter = 0;

int transfer_complete = 0;

int tx_fifo_empty_int_count = 0;
int rx_fifo_not_empty_int_count = 0;

uint32_t end_time;

void UART_1_ISR(void) {
    uint32_t txStatus = Cy_SCB_UART_GetTxFifoStatus(UART_1_HW);
    uint32_t rxStatus = Cy_SCB_UART_GetRxFifoStatus(UART_1_HW);
    
    //Add byte to TX if it is empty
    if((txStatus & CY_SCB_UART_TX_EMPTY) && (UART_1_HW->INTR_TX_MASK)!=0) {
        if(!transfer_complete)
            tx_fifo_empty_int_count++;
        
        //add byte
        Cy_SCB_UART_Put(UART_1_HW, src[tx_counter++]);
        
        Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_EMPTY);
    }
    //Disable TX once all bytes have been sent
    if(tx_counter >= BLOCK_SIZE) {
        UART_1_HW->INTR_TX_MASK = 0;
    }
    
    //Read from RX if it is not empty
    if(rxStatus & CY_SCB_UART_RX_NOT_EMPTY) {
        //Start throughput timer on first element recieved
        if(rx_counter == 0) {
            Throughput_timer_TriggerStart();
            Throughput_timer_Start();   
        }
        if(!transfer_complete)
            rx_fifo_not_empty_int_count++;
        
        //read byte
        dst[rx_counter++] = Cy_SCB_UART_Get(UART_1_HW);
        
        Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_NOT_EMPTY);
        
        //Stop throughput timer on first element recieved
        if(rx_counter == BLOCK_SIZE) {
            end_time = Throughput_timer_GetCounter();
            Throughput_timer_TriggerStop();
            transfer_complete++;
        }
    }
    
    //Continue processing RX until all elements are recieved
    if(rx_counter >= BLOCK_SIZE) {
        UART_1_HW->INTR_TX_MASK = 0;
        UART_1_HW->INTR_RX_MASK = 0;
    }
    
}

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
    
    //Reset error LED
    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
    
    //Inizialize src to values and dst to all zeros
    for(int i = 0; i < BLOCK_SIZE; i++) {
        src[i] = i % 256;
        dst[i] = 0;
    }

    //UART_1 setup
    Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    Cy_SCB_UART_Enable(UART_1_HW);
    
    //UART ISR setup
    Cy_SysInt_Init(&UART_1_INT_cfg, UART_1_ISR);
    NVIC_EnableIRQ(UART_1_INT_cfg.intrSrc);
    __enable_irq();
    
    //Throughput timer setup
    Throughput_timer_Init(&Throughput_timer_config);
    Throughput_timer_Enable();
    
    
    for(;;)
    {
        if(transfer_complete == 1) { //All data recieved
            
            //Check for data mismatches
            for(int i = 0; i < BLOCK_SIZE; i++) {
                if(dst[i] != src[i]) {
                    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
                    break;
                }
            }
            
            // Print to LCD
            lcd_cursor(0,0);
            char msg_tx[8];
            sprintf(msg_tx, "tx:%04d", tx_fifo_empty_int_count);
            lcd_write(msg_tx, sizeof(msg_tx));
            
            lcd_cursor(0,8);
            char msg_rx[8];
            sprintf(msg_rx, "rx:%04d", rx_fifo_not_empty_int_count);
            lcd_write(msg_rx, sizeof(msg_rx));
            
            // bytes/sec = BLOCK_SIZE/total_time(in sec)
            // with a 1MHz clock:
            //           = BLOCK_SIZE * 10^6 / total_time
            // using BLOCK_SIZE = 4096, bytes/sec = 4,096,000,000 / total_time
            
            const uint32_t bps_constant = 4096000000;
            
            uint32_t bytes_per_sec = bps_constant / end_time;
            
            lcd_cursor(1,0);
            char msg_bps[] = "Bps:";
            lcd_write(msg_bps, sizeof(msg_bps));
            
            lcd_cursor(1,5);
            char msg_bps_num[6];
            sprintf(msg_bps_num, "%05u", bytes_per_sec);
            lcd_write(msg_bps_num, sizeof(msg_bps_num));
            
            transfer_complete = 2;
        }
    }
}

/* [] END OF FILE */
