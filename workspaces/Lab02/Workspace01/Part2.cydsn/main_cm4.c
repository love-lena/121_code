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

uint16_t buffer_1[256];
uint16_t buffer_2[256];

int working_buffer = 1; //1 and 2 are valid values
int readable_buffer = 0; //1 and 2 are valid values, 0 means none are ready

bool dma_1_error = false;

void DMA_done_ISR(void) {
    readable_buffer = working_buffer;
    working_buffer = (working_buffer == 1) ? 2 : 1;   //switch working buffer
            
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);      
}

void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

uint16_t find_crossing_point(uint16_t* buffer) {
    uint16_t max_value = 0;
    uint16_t min_value = UINT16_MAX;
    for(int i = 0; i<256; i++) {
        if(buffer[i] > max_value)
            max_value = buffer[i];
        else if(buffer[i] < min_value)
            min_value = buffer[i];
    }
    return (max_value+min_value)/2;
}

int main(void)
{
    Cy_SysLib_Delay(50);
    lcd_init();
    
    init_uart_printf();

    __enable_irq();
    
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    //channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *) &(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,buffer_1);
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2,&DMA_1_Descriptor_2_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2,(uint32_t *) &(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2,buffer_2);
    
    Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    Cy_DMA_Enable(DMA_1_HW);
    
    Cy_SysInt_Init(&DMA_1_INT_cfg, DMA_done_ISR);
    NVIC_EnableIRQ(DMA_1_INT_cfg.intrSrc);
    
    Cy_DMA_Channel_SetInterruptMask(DMA_1_HW, DMA_1_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    ADC_1_Start();
    ADC_1_StartConvert();
    
    bool print_output = false;
    
    int cur_freq = 0;
    int update_cycles = 0;

    int plast = 0;
    
    for(;;)
    {
        if(readable_buffer) {
            uint16* read_from_buffer = readable_buffer==1 ? buffer_1 : buffer_2;
            readable_buffer = 0;
            
            uint16_t cross_point = find_crossing_point(read_from_buffer);
        
            int below = read_from_buffer[0] < cross_point;
            
            int p1=0;
            int p2=0;
            
            for(int i = 1; i < 256; i++) {
                char str[8];
                if(below) {
                    if(read_from_buffer[i] > cross_point) {
                        below = 0;
                        if(!p1)
                            p1=i;
                        else if(!p2) {
                            p2=i;
                            cur_freq = 1000/(255+p2-plast);
                        }
                        else 
                            plast = i;
                    }
                } else {
                    if(read_from_buffer[i] < cross_point) {
                        below = 1;
                        if(!p1)
                            p1=i;
                        else if(!p2) {
                            p2=i;
                            cur_freq = 1000/(255+p2-plast);
                        }
                        else
                            plast = i;
                    }
                }
                
            }

        }
        
        //update LCD
        if(update_cycles >= 1000000) {
            update_cycles=0;
            char freq_str[16];
            sprintf(freq_str, "%d\n\r", cur_freq);
            uart_printf(freq_str);
            if(cur_freq > 0 && cur_freq < 101) {
                lcd_cursor(0,5);
                char msg[8];
                sprintf(msg, "%03dkHz", cur_freq);
                lcd_write(msg, sizeof(msg));
            } else {
                lcd_cursor(0,5);
                char msg[] = "OOR   ";
                lcd_write(msg, sizeof(msg));
            }
        } else {
            update_cycles++;
        }
    }
}

/* [] END OF FILE */
