/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/
#include "project.h"
#include <stdlib.h>
#include <stdio.h>

void init_uart_printf(void);
void uart_printf(char *print_string);

int main(void)
{
    init_uart_printf();
    
    __enable_irq();
    
    //PDM - PCM setup
    Cy_PDM_PCM_Init(PDM_PCM_1_HW, &PDM_PCM_1_config);
    Cy_PDM_PCM_Enable(PDM_PCM_1_HW);
    
    //VDAC setup
    Cy_CTDAC_FastInit(CTDAC0, &Cy_CTDAC_Fast_VddaRef_UnbufferedOut);
    Cy_CTDAC_Enable(CTDAC0);
    
    //Data store
    size_t BUFFER_SIZE = 65536;
    uint16_t sample_buffer [BUFFER_SIZE];
    
    //DMA setup
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    
    //SAR->CHAN_RESULT[0] is the index of the ADC registry
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    //src -> PDM_PCM RX FIFO pointer
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *) &(PDM_PCM_1_HW->RX_FIFO_RD));
    //dst -> sample buffer base pointer
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,sample_buffer);
    
    bool recording = false;
    //Data ready to send
    bool ready_to_play = false;
    
    for(;;)
    {
        
        //Start recording
        if(!ready_to_play && !recording && !Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
            Cy_DMA_Enable(DMA_1_HW);
            Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
            recording = true;
        }
        
        //Stop recording
        if(recording && Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            Cy_DMA_Disable(DMA_1_HW);
            Cy_DMA_Channel_Disable(DMA_1_HW, DMA_1_DW_CHANNEL);
            recording = false;
            ready_to_play = true;
        }
        
        if(ready_to_play) {

            //Get the index that DMA made it to
            size_t count = DMA_1_HW->CH_STRUCT[DMA_1_DW_CHANNEL].CH_IDX;
            //Reset DMA
            Cy_DMA_Channel_DeInit(DMA_1_HW, DMA_1_DW_CHANNEL);
            
            //If DMA completed, set count to the end of the buffer
            count = !count ? BUFFER_SIZE : count;
            
            for(size_t i = 0; i < count; i++) {
                
                //convert data
                uint32_t rxData_raw = sample_buffer[i];
                int16_t rxData_signed = (int16_t) rxData_raw;
                uint16_t rxData_ready = (uint16_t) abs(rxData_signed);
                    
                Cy_CTDAC_SetValueBuffered(CTDAC0, rxData_ready);
                CyDelayUs(125);
                
            }
            
            ready_to_play = false;
            
        }
        
    }
}

/* [] END OF FILE */
