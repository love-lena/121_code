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
    
    //char maxmin_str[16];
    //sprintf(maxmin_str, "%d-%d\n\r", min_value, max_value);
    //uart_printf(maxmin_str);
            
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

    
    int previous_buffer_last_cross = 0; //0-255
    int previous_buffer_second_to_last_cross = 0; //0-255
    uint16_t previous_buffer_cross_point = 0;
    
    int two_buffer_ago_last_cross = 0; //0-255
    uint16_t two_buffer_ago_cross_point = 0;
    
    int current_buffer_first_cross = 0; //0-255
    uint16_t current_buffer_cross_point = 0;
    
    bool first_buffer_processed = false;
    bool second_buffer_processed = false;
    
    uint16_t working_crosspoint;
    
    
    for(;;) {
        
        if(readable_buffer) { //We have a buffer ready to read from
            
            uint16* read_from_buffer = readable_buffer==1 ? buffer_1 : buffer_2;
            readable_buffer = 0;
            
            if(!first_buffer_processed) {//process the first buffer
                first_buffer_processed = true;
                two_buffer_ago_cross_point = find_crossing_point(read_from_buffer);
                for(int i = 1; i < 256; i++) {
                    if(((read_from_buffer[i] > two_buffer_ago_cross_point) && 
                            (read_from_buffer[i-1] < two_buffer_ago_cross_point)) || 
                            ((read_from_buffer[i] < two_buffer_ago_cross_point) && 
                            (read_from_buffer[i-1] > two_buffer_ago_cross_point)))
                    {//we have crossed the crosspoint
                        two_buffer_ago_last_cross = i;
                    }
                }
            } else if(!second_buffer_processed) {//process the second buffer
                second_buffer_processed = true;
                previous_buffer_cross_point = find_crossing_point(read_from_buffer);
                for(int i = 1; i < 256; i++) {
                    bool seconds = false;
                    if(((read_from_buffer[i] > working_crosspoint) && 
                            (read_from_buffer[i-1] < working_crosspoint)) || 
                            ((read_from_buffer[i] < working_crosspoint) && 
                            (read_from_buffer[i-1] > working_crosspoint)))
                    {//we have crossed the crosspoint
                        previous_buffer_second_to_last_cross = previous_buffer_last_cross; //will be 0 first time
                        previous_buffer_last_cross = i; //set to last point
                        
                    }
                }
            } else {//ready to process data
                
                current_buffer_cross_point = find_crossing_point(read_from_buffer);
                
                
                bool using_two_ago = !previous_buffer_second_to_last_cross; //if we have the second to last element, dont use two ago
                
                uint16_t using_this_crossing_point;
                if(using_two_ago) {
                    using_this_crossing_point = two_buffer_ago_cross_point;
                } else {
                    using_this_crossing_point = previous_buffer_cross_point;
                }
                
                bool first_item = true;
                for(int i = 1; i < 256; i++) {
                   
                    
                    if(((read_from_buffer[i] > using_this_crossing_point) && 
                            (read_from_buffer[i-1] < using_this_crossing_point)) || 
                            ((read_from_buffer[i] < using_this_crossing_point) && 
                            (read_from_buffer[i-1] > using_this_crossing_point)))
                    {//we have crossed the crosspoint
                        
                        if(first_item) { //use this cross to compare
                            first_item = false;
                            
                            if(using_two_ago) {
                                cur_freq = 925/(512+i-two_buffer_ago_last_cross); //compare current point with point two buffers ago
                            } else {
                                cur_freq = 925/(256+i-previous_buffer_second_to_last_cross); //compare current point with point in last buffer
                            }
                            
                            two_buffer_ago_last_cross = previous_buffer_last_cross;
                            two_buffer_ago_cross_point = previous_buffer_cross_point;
                            
                            previous_buffer_last_cross = i;
                            previous_buffer_second_to_last_cross = 0;
                            previous_buffer_cross_point = current_buffer_cross_point;
                            
                        } else { //just updating last items
                            previous_buffer_second_to_last_cross = previous_buffer_last_cross;
                            previous_buffer_last_cross = i;
                        }
                            
                    }
                    
                    
                }
                
                //two_buffer_ago_cross_point = previous_buffer_cross_point;
                //previous_buffer_cross_point = current_buffer_cross_point;
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
    
    
    
    /*
    int b1 = 0;
    int b1_gap = 0;
    int b2 = 0;
    int b2_gap = 0;
    int b3 = 0;

    uint16_t last_cross = 0;
    
    for(;;)
    {
        if(readable_buffer) {
            b1_gap += 256;
            b2_gap += 256;
            uint16* read_from_buffer = readable_buffer==1 ? buffer_1 : buffer_2;
            readable_buffer = 0;

            
            if(!last_cross)
                last_cross = find_crossing_point(read_from_buffer);
            int below = read_from_buffer[0] < last_cross;
            
            //int p1=0;
            bool measured_this_time = false;
            uint16_t this_cross = last_cross;
            for(int i = 1; i < 256; i++) {
                
                //char b1b2_str[16];
                //sprintf(b1b2_str, "%d: %d . %d\n\r", i, b1_gap-b1, b2_gap-b2);
                //uart_printf(b1b2_str);
                
                if(below && read_from_buffer[i] > this_cross) {
                    //uart_printf("Found one!");
                    below = 0;
                    if(!b1) {//should only happen ONCE
                        b1 = i;
                    } else if(!b2) {//the skip value - should only happen ONCE
                        b2=i;
                    } else if(!measured_this_time) {//have a measured period
                        measured_this_time = true;
                        //b3=0;
                        
                        cur_freq = 1000/(b1_gap+i-b1);
                        last_cross = find_crossing_point(read_from_buffer);

                        b1=b2;
                        b2=i;
                    } else {//measured a period, have items left. assign to b1/b2
                        b1=b2;
                        b1_gap = b2_gap;
                        b2=i;
                        b2_gap = 256;
                    }

                } else if(!below && read_from_buffer[i] < this_cross) {
                    //uart_printf("Found one!");
                    below = 1;
                    if(!b1) {//should only happen ONCE
                        b1 = i;
                    } else if(!b2) {//the skip value - should only happen ONCE
                        b2=i;
                    } else if(!measured_this_time) {//have a measured period
                        measured_this_time = true;
                        //b3=0;
                        
                        cur_freq = 1000/(b1_gap+i-b1);
                        last_cross = find_crossing_point(read_from_buffer);

                        b1=b2;
                        b2=i;
                    } else {//measured a period, have items left. assign to b1/b2
                        b1=b2;
                        b1_gap = b2_gap;
                        b2=i;
                        b2_gap = 256;
                    }
                }
                
            }
            
            //last_cross = find_crossing_point(read_from_buffer);
            //char last_cross_str[16];
            //sprintf(last_cross_str, "%d\n\r", last_cross);
           // uart_printf(last_cross_str);

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
    
    */
}

/* [] END OF FILE */
