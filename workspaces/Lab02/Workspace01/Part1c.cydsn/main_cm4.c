/* ========================================
 *
 * CSE 121 - Lab 2
 * Part 1c main
 * Lena Hickson Long
 *
 * Uses DMA to copy an array 4x
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

void init_uart_printf(void);
void uart_printf(char *print_string);

int dma_1_done = false;
int dma_1_error = false;

//Clears interrupt and sets flags
void DMA_done_ISR(void) {
    
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
    // Set up interrupts and print functions
    init_uart_printf();
    Cy_SysInt_Init(&DMA_1_INT_cfg, DMA_done_ISR);
    NVIC_EnableIRQ(DMA_1_INT_cfg.intrSrc);
    __enable_irq();

    // block_size: how many bytes in our source block
    // dst block size is 4x as large
    int block_size = 4096;
    int dst_block_size = block_size*4;
    uint8 src[block_size];
    uint8 dst[dst_block_size];
    uint8 tst_dst[dst_block_size];
    
    //Inizialize src to values and dst to all zeros
    for(int i = 0; i < block_size; i++) {
        src[i] = i % 256;
        for(int j = 0; j < 4; j++) {
            dst[i*4+j] = 0;
            tst_dst[i*4+j] = i % 256;
        }
    }
    
    
    // Set up DMA
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    //Large configuration block for descriptor src/dst
    //Each descriptor is responsible for 256 bytes of source
    //Includes multiplication that can be replaced with fixed values
    //  if the block size is assumed constant
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
    
    //Enable debug printing?
    bool print_output = true;
    
    for(;;)
    {
        if(dma_1_done) { //File transfer is complete
            //produce error:
            dst[16367] = 101;
            
            //Reset error LED
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);

            bool mismatch = false;
            //Go through in 16 byte sections (for pretty printing)
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
                     //Point out malformed line
                    if(section_mismatch)
                        uart_printf(" <-- MISMATCH HERE");
                    uart_printf("\n\r");
                }
            }
            if(mismatch) {
                uart_printf("ERROR: some bytes were not copied correctly\n\r");
                //Turn on error LED
                Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
            }

            //Reset flags
            dma_1_done = 0;
            dma_1_error = 0;
        }
        
    }
}

/* [] END OF FILE */
