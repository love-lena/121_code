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
int rx_fifo_not_empty_int_count = 0;

uint32_t end_time;

void UART_1_ISR(void) {
    uint32_t txStatus = Cy_SCB_UART_GetTxFifoStatus(UART_1_HW);
    uint32_t rxStatus = Cy_SCB_UART_GetRxFifoStatus(UART_1_HW);
    
    if(txStatus & CY_SCB_UART_TX_EMPTY) {
        if(!transfer_complete)
            tx_fifo_empty_int_count++;
        
        //add byte
        Cy_SCB_UART_Put(UART_1_HW, src[tx_counter++]);
        
        Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_EMPTY);
    } else if(rxStatus & CY_SCB_UART_RX_NOT_EMPTY) {
        if(rx_counter == 0) 
            Throughput_timer_TriggerStart();
        if(!transfer_complete)
            rx_fifo_not_empty_int_count++;
        
        //read byte
        dst[rx_counter++] = Cy_SCB_UART_Get(UART_1_HW);
        
        Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_NOT_EMPTY);
        
        if(rx_counter == 4096) {
            end_time = Throughput_timer_GetCounter();
            Throughput_timer_TriggerStop();
            transfer_complete++;
        }
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
    cy_en_scb_uart_status_t init_status = Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    if(init_status != CY_SCB_UART_SUCCESS) {
        //bad things
    }
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

            for(int i = 0; i < BLOCK_SIZE; i++) {
                if(dst[i] != src[i]) {
                    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
                    break;
                }
            }
            
            lcd_cursor(0,0);
            char msg_tx[8];
            sprintf(msg_tx, "tx:%04d", tx_fifo_empty_int_count);
            lcd_write(msg_tx, sizeof(msg_tx));
            
            lcd_cursor(0,8);
            char msg_rx[8];
            sprintf(msg_rx, "rx:%04d", rx_fifo_not_empty_int_count);
            lcd_write(msg_rx, sizeof(msg_rx));
            
            //bytes/sec = BLOCK_SIZE/total_time(in sec)
            //          = BLOCK_SIZE * 10^6 / total_time
            // using BLOCK_SIZE = 4096, bytes/sec = 4,096,000,000 / total_time
            
            uint32_t start_time = 65535;
            uint32_t total_time = start_time - end_time;

            int bytes_per_sec = 4096000000 / total_time;
            
            lcd_cursor(1,0);
            char msg_bps[] = "Bps:";
            lcd_write(msg_bps, sizeof(msg_bps));
            
            lcd_cursor(1,5);
            char msg_bps_num[6];
            sprintf(msg_bps_num, "%06d", bytes_per_sec);
            lcd_write(msg_bps_num, sizeof(msg_bps_num));
            
            transfer_complete = 2;
        }
    }
}

/* [] END OF FILE */
