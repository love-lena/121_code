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
#include <stdio.h>

void init_uart_printf(void);
void uart_printf(char *print_string);

int dma_1_done = false;
int dma_1_error = false;

void DMA_done_ISR(void) {
    //uart_printf("transfer done!n\r");
    
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);
    
    if (Cy_DMA_Channel_GetStatus(DMA_1_HW, DMA_1_DW_CHANNEL) ==
        CY_DMA_INTR_CAUSE_COMPLETION) {
            dma_1_done = 1;
        } else {
            /* DMA error, abnormal termination */
            dma_1_error = 1;
    }
}

int main(void)
{
    init_uart_printf();
    Cy_SysInt_Init(&DMA_1_INT_cfg, DMA_done_ISR);
    NVIC_EnableIRQ(DMA_1_INT_cfg.intrSrc);
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    int block_size = 4096;
    int dst_block_size = block_size*4;
    uint8 src[block_size];
    uint8 dst[dst_block_size];
    uint8 tst_dst[dst_block_size];
    
    for(int i = 0; i < block_size; i++) {
        src[i] = i % 256;
        for(int j = 0; j < 4; j++) {
            dst[i*4+j] = 0;
            tst_dst[i*4+j] = i % 256;
        }
    }
    
    
    /* Allocate descriptor */
    cy_stc_dma_channel_config_t channelConfig;
    /* Set parameters based on settings of DMA component */
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    /* Start of descriptor chain */
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    { //added to make collapsable in IDE
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,src+256*0);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,dst+1024*0);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2,&DMA_1_Descriptor_2_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2,src+256*1);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2,dst+1024*1);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_3,&DMA_1_Descriptor_3_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_3,src+256*2);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_3,dst+1024*2);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_4,&DMA_1_Descriptor_4_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_4,src+256*3);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_4,dst+1024*3);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_5,&DMA_1_Descriptor_5_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_5,src+256*4);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_5,dst+1024*4);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_6,&DMA_1_Descriptor_6_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_6,src+256*5);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_6,dst+1024*5);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_7,&DMA_1_Descriptor_7_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_7,src+256*6);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_7,dst+1024*6);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_8,&DMA_1_Descriptor_8_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_8,src+256*7);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_8,dst+1024*7);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_9,&DMA_1_Descriptor_9_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_9,src+256*8);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_9,dst+1024*8);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_10,&DMA_1_Descriptor_10_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_10,src+256*9);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_10,dst+1024*9);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_11,&DMA_1_Descriptor_11_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_11,src+256*10);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_11,dst+1024*10);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_12,&DMA_1_Descriptor_12_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_12,src+256*11);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_12,dst+1024*11);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_13,&DMA_1_Descriptor_13_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_13,src+256*12);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_13,dst+1024*12);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_14,&DMA_1_Descriptor_14_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_14,src+256*13);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_14,dst+1024*13);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_15,&DMA_1_Descriptor_15_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_15,src+256*14);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_15,dst+1024*14);
        
        Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_16,&DMA_1_Descriptor_16_config);
        Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_16,src+256*15);
        Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_16,dst+1024*15);
    }
    
    Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
    
    Cy_DMA_Enable(DMA_1_HW);
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    
    Cy_DMA_Enable(DMA_1_HW);
    
    Cy_DMA_Channel_SetInterruptMask(DMA_1_HW, DMA_1_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    Cy_TrigMux_SwTrigger(TRIG0_OUT_CPUSS_DW0_TR_IN0,CY_TRIGGER_TWO_CYCLES);
    
    bool print_output = true;
    
    /*
    for(int i = 0; i < block_size/16; i++) {
        char str[8];
        sprintf(str, "%02x: ", i);
        uart_printf(str);
        
        for(int j = 0; j < 16; j++) {
            unsigned int elem = tst_dst[16*i + j];
            char str[8];
            sprintf(str, "%02x ", elem);
            uart_printf(str);
        }
        uart_printf("\n\r");
    }
    */
    
    for(;;)
    {
        if(dma_1_done) {
            //produce error:
            dst[100] = 101;
            
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
            //uart_printf("dma 1 done in loop\n\r");
            bool mismatch = false;
            for(int i = 0; i < dst_block_size/16; i++) {
                char str[8];
                sprintf(str, "%02x0: ", i);
                if(print_output)
                    uart_printf(str);
                
                bool section_mismatch = false;
                for(int j = 0; j < 16; j++) {
                    if(dst[16*i + j] != tst_dst[16*i + j]) {
                        section_mismatch = true;
                        mismatch = true;
                    }
                    unsigned int elem = dst[16*i + j];
                    char str[8];
                    sprintf(str, "%02x ", elem);
                    if(print_output)
                        uart_printf(str);
                }
                if(print_output) {
                    if(section_mismatch)
                        uart_printf(" <-- MISMATCH HERE");
                    uart_printf("\n\r");
                }
            }
            if(mismatch) {
                uart_printf("ERROR: some bytes were not copied correctly\n\r");
                Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
            }

            dma_1_done = 0;
            dma_1_error = 0;
        }
        
    }
}

/* [] END OF FILE */
