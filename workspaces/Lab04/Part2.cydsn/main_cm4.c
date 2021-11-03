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
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *) &(PDM_PCM_1_HW->RX_FIFO_RD));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,sample_buffer);
    
    bool recording = false;
    //Data ready to send
    bool ready_to_play = false;
    
    for(;;)
    {
        //Attempt 3
        
        //Start recording
        if(!ready_to_play && !recording && !Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
            Cy_DMA_Enable(DMA_1_HW);
            Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
            recording = true;
            uart_printf("recording...\n\r");
        }
        
        //Stop recording
        if(recording && Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            Cy_DMA_Disable(DMA_1_HW);
            Cy_DMA_Channel_Disable(DMA_1_HW, DMA_1_DW_CHANNEL);
            recording = false;
            ready_to_play = true;
            uart_printf("done recording\n\r");
        }
        
        if(ready_to_play) {

            size_t count = DMA_1_HW->CH_STRUCT[DMA_1_DW_CHANNEL].CH_IDX;
            //Reset DMA
            Cy_DMA_Channel_DeInit(DMA_1_HW, DMA_1_DW_CHANNEL);
            
            //If DMA completed, set count to the end of the buffer
            count = !count ? BUFFER_SIZE : count;
            
            char count_str[8];
            sprintf(count_str, "%d\n\r", count);
            uart_printf(count_str);

            
            uart_printf("PLAYING\n\r");
         
            
            for(size_t i = 0; i < count; i++) {
                
                //convert data
                uint32_t rxData_raw = sample_buffer[i];
                int16_t rxData_signed = (int16_t) rxData_raw;
                uint16_t rxData_ready = (uint16_t) abs(rxData_signed);
                    
                Cy_CTDAC_SetValueBuffered(CTDAC0, rxData_ready);
                CyDelayUs(125);
                
            }
            
            uart_printf("DONE PLAYING\n\r");
            
            ready_to_play = false;
            
        }
        
        
        //Attempt 2
        /*
        //char recording_str[6];
        //sprintf(recording_str, "%d\n\r", recording);
        //uart_printf(recording_str);
        
        //Clear FIFO
        PDM_PCM_1_ClearFifo();
        
        //Start recording
        if(!ready_to_play && !recording && !Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            uart_printf("START recording");
            recording = true;
            enable_mic_int();
        }
        
        //Stop recording
        if(recording && 
            (Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM) || sample_buffer_index >= BUFFER_SIZE)) {
                
            uart_printf("STOP recording");
            recording = false;
            ready_to_play = true;
            disable_mic_int();
        }  
            
        //Get mic data
        if(data_ready) {
            disable_mic_int();
            
            uint32_t rxData_raw = PDM_PCM_1_ReadFifo();
            int16_t rxData_signed = (int16_t) rxData_raw;
            uint16_t rxData_ready = (uint16_t) abs(rxData_signed);
            sample_buffer[sample_buffer_index++] = rxData_ready;
            
            //char index_str[8];
            //sprintf(index_str, "%d\n\r", sample_buffer_index);
            //uart_printf(index_str);
            
            data_ready = false;
            enable_mic_int();
        }
        
        if(ready_to_play) {
            
            uart_printf("PLAYING\n\r");
         
            disable_mic_int();
            
            for(size_t i = 0; i < sample_buffer_index; i++) {
             
                Cy_CTDAC_SetValueBuffered(CTDAC0, sample_buffer[i]);
                CyDelayUs(125);
                
            }
            
            uart_printf("DONE PLAYING\n\r");
            
            sample_buffer_index = 0;
            ready_to_play = false;
            enable_mic_int();
            
        }
        */
        
        //Attempt 1
        /*
        //Read data from the Mic
        if(data_ready) {
            
            //Buffer is not full
            if(sample_buffer_index < BUFFER_SIZE) {
                //FIFO is not empty
                if(PDM_PCM_1_GetNumInFifo()) {
                    
                    //Save data
                    uint32_t rxData_raw = PDM_PCM_1_ReadFifo();
                    int16_t rxData_signed = (int16_t) rxData_raw;
                    uint16_t rxData_ready = (uint16_t) abs(rxData_signed);
                    sample_buffer[sample_buffer_index++] = rxData_ready;
                            
                } 
            }
            
            data_ready = false;
            //Enable RX FIFO NOT EMPTY interrupt
            PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() | CY_PDM_PCM_INTR_RX_NOT_EMPTY);
            ready_to_play = true;
        }
        
        if(ready_to_play) {
         
            //Disable RX FIFO NOT EMPTY interrupt
            PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() & ~CY_PDM_PCM_INTR_RX_NOT_EMPTY);
            
            for(size_t i = 0; i < BUFFER_SIZE; i++) {
             
                Cy_CTDAC_SetValueBuffered(CTDAC0, sample_buffer[i]);
                CyDelay(125);
                
            }
            
            ready_to_play = false;
            //Enable RX FIFO NOT EMPTY interrupt
            PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() | CY_PDM_PCM_INTR_RX_NOT_EMPTY);
            
        }
        */
        
        
    }
}

/* [] END OF FILE */
