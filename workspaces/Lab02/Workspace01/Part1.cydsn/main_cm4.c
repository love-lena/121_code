/* ========================================
 *
 * CSE 121 - Lab 2
 * Part 1a main
 * Lena Hickson Long
 *
 * Uses DMA to copy an array as is
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
    int block_size = 4096;
    uint8 src[block_size];
    uint8 dst[block_size];
    
    //Inizialize src to values and dst to all zeros
    for(int i = 0; i < block_size; i++) {
        src[i] = i % 256;
        dst[i] = 0;
    }
    
    // Set up DMA
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    
    //src -> dst
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,src);
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,dst);
    
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
            dst[4079] = 101;
            
            //Reset error LED
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
            
            bool mismatch = false;
            //Go through in 16 byte sections (for pretty printing)
            for(int i = 0; i < block_size/16; i++) {
                char str[8];
                sprintf(str, "%02x: ", i);
                if(print_output)
                    uart_printf(str);
                
                bool section_mismatch = false;
                for(int j = 0; j < 16; j++) {
                    if(dst[16*i + j] != src[16*i + j]) {
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
