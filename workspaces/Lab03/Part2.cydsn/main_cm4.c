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

int transfer_complete = 0;

int UART_INT_count = 0;

uint32_t end_time;

void TxDMA_done_ISR(void) {
    UART_1_HW->INTR_TX_MASK = 0;
    Cy_DMA_Channel_ClearInterrupt(TxDMA_HW, TxDMA_DW_CHANNEL);
}

void RxDMA_done_ISR(void) {
    UART_1_HW->INTR_TX_MASK = 0;
    UART_1_HW->INTR_RX_MASK = 0;
    
    //Measure total time to transfer
    end_time = Throughput_timer_GetCounter();
    Throughput_timer_TriggerStop();
    transfer_complete++;
    Cy_DMA_Channel_ClearInterrupt(RxDMA_HW, RxDMA_DW_CHANNEL);
}


void UART_1_ISR(void) {
    //Record then clear interrupt reasons
    UART_INT_count++;
    Cy_SCB_UART_ClearTxFifoStatus(UART_1_HW, CY_SCB_UART_TX_OVERFLOW);
    
    Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_OVERFLOW);
    Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_UNDERFLOW);
    Cy_SCB_UART_ClearRxFifoStatus(UART_1_HW, CY_SCB_UART_RX_ERR_FRAME);
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
    Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 1);
    Cy_GPIO_Write(LED_2_PORT, LED_2_NUM, 1);
    
    
    //Inizialize src to values and dst to all zeros
    for(int i = 0; i < BLOCK_SIZE; i++) {
        src[i] = i % 256;
        dst[i] = 0;
    }

    //UART_1 setup
    Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    
    
    //UART ISR setup
    Cy_SysInt_Init(&UART_1_INT_cfg, UART_1_ISR);
    NVIC_EnableIRQ(UART_1_INT_cfg.intrSrc);
    
    
    //DMA Setup
    
    //TX
    cy_stc_dma_channel_config_t TxChannelConfig;
    TxChannelConfig.descriptor = &TxDMA_Descriptor_1;
    TxChannelConfig.preemptable = TxDMA_PREEMPTABLE;
    TxChannelConfig.priority = TxDMA_PRIORITY;
    TxChannelConfig.enable = false;
    TxChannelConfig.bufferable = TxDMA_BUFFERABLE;
    
    Cy_DMA_Descriptor_Init(&TxDMA_Descriptor_1,&TxDMA_Descriptor_1_config);
    
    Cy_DMA_Descriptor_SetSrcAddress(&TxDMA_Descriptor_1,src);
    Cy_DMA_Descriptor_SetDstAddress(&TxDMA_Descriptor_1,(uint32_t *) &UART_1_HW->TX_FIFO_WR);
    
    Cy_DMA_Channel_Init(TxDMA_HW, TxDMA_DW_CHANNEL,&TxChannelConfig);
    
    Cy_DMA_Channel_SetInterruptMask(TxDMA_HW, TxDMA_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    //TX ISR
    Cy_SysInt_Init(&TxDMA_INT_cfg, TxDMA_done_ISR);
    NVIC_EnableIRQ(TxDMA_INT_cfg.intrSrc);
    
    //RX
    cy_stc_dma_channel_config_t RxChannelConfig;
    RxChannelConfig.descriptor = &RxDMA_Descriptor_1;
    RxChannelConfig.preemptable = RxDMA_PREEMPTABLE;
    RxChannelConfig.priority = RxDMA_PRIORITY;
    RxChannelConfig.enable = false;
    RxChannelConfig.bufferable = RxDMA_BUFFERABLE;
    
    Cy_DMA_Descriptor_Init(&RxDMA_Descriptor_1,&RxDMA_Descriptor_1_config);
    
    Cy_DMA_Descriptor_SetSrcAddress(&RxDMA_Descriptor_1,(uint32_t *) &UART_1_HW->RX_FIFO_RD);
    Cy_DMA_Descriptor_SetDstAddress(&RxDMA_Descriptor_1,dst);
    
    Cy_DMA_Channel_Init(RxDMA_HW, RxDMA_DW_CHANNEL,&RxChannelConfig);

    Cy_DMA_Channel_SetInterruptMask(RxDMA_HW, RxDMA_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    //RX ISR
    Cy_SysInt_Init(&RxDMA_INT_cfg, RxDMA_done_ISR);
    NVIC_EnableIRQ(RxDMA_INT_cfg.intrSrc);
    __enable_irq();
    
    
    //Throughput timer setup
    Throughput_timer_Init(&Throughput_timer_config);
    Throughput_timer_Enable();
    
    
    //Start everything at once
    Cy_DMA_Enable(TxDMA_HW);
    Cy_DMA_Channel_Enable(TxDMA_HW, TxDMA_DW_CHANNEL);
    Cy_DMA_Enable(RxDMA_HW);
    Cy_DMA_Channel_Enable(RxDMA_HW, RxDMA_DW_CHANNEL);
    Cy_SCB_UART_Enable(UART_1_HW);
    Throughput_timer_TriggerStart();

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
            char msg_uart_err[14];
            sprintf(msg_uart_err, "uart err: %04d", UART_INT_count);
            lcd_write(msg_uart_err, sizeof(msg_uart_err));
            
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
