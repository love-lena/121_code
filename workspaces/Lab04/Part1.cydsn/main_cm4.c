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

//Data ready flag
bool data_ready = false;

void enable_mic_int() {
    PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() | CY_PDM_PCM_INTR_RX_NOT_EMPTY);
}

void disable_mic_int() {
    PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() & ~CY_PDM_PCM_INTR_RX_NOT_EMPTY);
}

void PDM_PCM_ISR(void) {
    
    uint32_t interrupts = Cy_PDM_PCM_GetInterruptStatusMasked(PDM_PCM_1_HW);
    Cy_PDM_PCM_ClearInterrupt(PDM_PCM_1_HW, interrupts);
    
    data_ready = true;
    
    disable_mic_int();
    
}

int main(void)
{
    init_uart_printf();
    
    __enable_irq();
    
    //PDM - PCM setup
    Cy_PDM_PCM_Init(PDM_PCM_1_HW, &PDM_PCM_1_config);
    Cy_PDM_PCM_Enable(PDM_PCM_1_HW);
    
    Cy_SysInt_Init(&PDM_PCM_1_INT_cfg, PDM_PCM_ISR);
    NVIC_EnableIRQ(PDM_PCM_1_INT_cfg.intrSrc);
    
    
    //VDAC setup
    Cy_CTDAC_FastInit(CTDAC0, &Cy_CTDAC_Fast_VddaRef_UnbufferedOut);
    Cy_CTDAC_Enable(CTDAC0);
    
    //Data store
    size_t BUFFER_SIZE = 65536;
    uint16_t sample_buffer [BUFFER_SIZE];
    size_t sample_buffer_index = 0;
    
    bool recording = false;
    //Data ready to send
    bool ready_to_play = false;
    
    disable_mic_int();
    
    for(;;)
    {
        //Attempt 3
        //Clear FIFO if we're not recording
        PDM_PCM_1_ClearFifo();
        
        //Record as long as button is down, only keep first 8 seconds
        while(!Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM)) {
            
            //Data is ready, once done with while loop, this flag will be read
            ready_to_play = true;
            
            //Store data
            if(data_ready) {
                disable_mic_int();
                
                //Only store first 8 seconds
                if(sample_buffer_index < BUFFER_SIZE) {
                    //Make sure FIFO actually has data
                    if(PDM_PCM_1_GetNumInFifo()) {
                        
                        //Read data, convert format, store in buffer
                        uint32_t rxData_raw = PDM_PCM_1_ReadFifo();
                        int16_t rxData_signed = (int16_t) rxData_raw;
                        uint16_t rxData_ready = (uint16_t) abs(rxData_signed);
                        sample_buffer[sample_buffer_index++] = rxData_ready;
            
                    }
                }
            }
            
            //Ready for next block of data
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
