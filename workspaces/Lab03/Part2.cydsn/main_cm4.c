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

int transfer_complete = 0;

int UART_INT_count = 0;

uint32_t end_time;

void TxDMA_done_ISR(void) {
    Cy_DMA_Channel_ClearInterrupt(TxDMA_HW, TxDMA_DW_CHANNEL);
}

void RxDMA_done_ISR(void) {
    end_time = Throughput_timer_GetCounter();
    Throughput_timer_TriggerStop();
    transfer_complete++;
    Cy_DMA_Channel_ClearInterrupt(RxDMA_HW, RxDMA_DW_CHANNEL);
}


void UART_1_ISR(void) {
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
    cy_en_scb_uart_status_t init_status = Cy_SCB_UART_Init(UART_1_HW, &UART_1_config, &UART_1_context);
    if(init_status != CY_SCB_UART_SUCCESS) {
        //bad things
    }
    
    
    //UART ISR setup
    Cy_SysInt_Init(&UART_1_INT_cfg, UART_1_ISR);
    NVIC_EnableIRQ(UART_1_INT_cfg.intrSrc);
    
    Cy_GPIO_Write(LED_1_PORT, LED_1_NUM, 1);
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
    
    //ISR
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
    
    //ISR
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
            //produce error:
            //dst[4079] = 101;
            
            for(int i = 0; i < BLOCK_SIZE; i++) {
                if(dst[i] != src[i]) {
                    Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
                    break;
                }
            }
            
            lcd_cursor(0,0);
            char msg_uart_err[14];
            sprintf(msg_uart_err, "uart err: %04d", UART_INT_count);
            lcd_write(msg_uart_err, sizeof(msg_uart_err));
            
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
