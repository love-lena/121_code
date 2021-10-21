/* ========================================
 *
 * CSE 121 - Lab 2
 * Part 2 main
 * Lena Hickson Long
 *
 * Uses DMA to sample an analog wave and find its frequency
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

void init_uart_printf(void);
void uart_printf(char *print_string);

//Set up buffers to hold data
uint16_t buffer_1[256];
uint16_t buffer_2[256];

/* working_buffer: keep track of which buffer is being written to
 * 1 and 2 are valid values */
int working_buffer = 1; 
/* readable_buffer: keep track of which buffer is ready to read from
 * 0, 1 and 2 are valid values 
 * 0 indicates no buffers are ready to read from */
int readable_buffer = 0; 

void DMA_done_ISR(void) {
    //set readable_buffer to the one just written in
    readable_buffer = working_buffer;
    //switch working buffer
    working_buffer = (working_buffer == 1) ? 2 : 1;   
            
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);      
}

//Set up LCD functions
void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

// Helper function to calculate the average of a set of data
// Uses max-min/2 to find average
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
    //Wait 50ms before initializing LCD
    Cy_SysLib_Delay(50);
    lcd_init();
    
    init_uart_printf();

    __enable_irq();
    
    //Set up DMA
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    
    //SAR->CHAN_RESULT[0] is the index of the ADC registry
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
    
    // Start ADC
    ADC_1_Start();
    ADC_1_StartConvert();
    
    //Enable debug priting
    bool print_output = false;
    
    
    //Variables for tracking freqency and average across buffers
    int cur_freq = 0;
    int update_cycles = 0;

    int previous_buffer_last_cross = 0; //0-255
    int previous_buffer_second_to_last_cross = 0; //0-255
    uint16_t previous_buffer_cross_point = 0;
    
    int two_buffer_ago_last_cross = 0; //0-255
    uint16_t two_buffer_ago_cross_point = 0;
    
    uint16_t current_buffer_cross_point = 0;
    
    bool first_buffer_processed = false;
    bool second_buffer_processed = false;
    
    for(;;) {
        
        
        /*
         * Main Loop
         *
         * This is the main loop which processes buffers to calculate the frequency of the input wave
         *
         * first_buffer_processed and second_buffer_processed:
         *   The main logic used below keeps track of the previous 2 buffers.
         *   We wait until we have at least 2 buffers of data before measuring
         *   Potential optimization: Using default values may allow the deletion of repeated code
         *                           The measurement would just be innaccurate for the first two buffer cycles
        */
        
        if(readable_buffer) { //We have a buffer ready to read from
            
            uint16* read_from_buffer = readable_buffer==1 ? buffer_1 : buffer_2;
            readable_buffer = 0;
            
            if(!first_buffer_processed) { //process the first buffer
                first_buffer_processed = true;
                two_buffer_ago_cross_point = find_crossing_point(read_from_buffer);
                for(int i = 1; i < 256; i++) {
                    //This if statement determines if we just crossed the crossing point
                    if(((read_from_buffer[i] > two_buffer_ago_cross_point) && 
                            (read_from_buffer[i-1] < two_buffer_ago_cross_point)) || 
                            ((read_from_buffer[i] < two_buffer_ago_cross_point) && 
                            (read_from_buffer[i-1] > two_buffer_ago_cross_point)))
                    { //we have crossed the crosspoint
                        two_buffer_ago_last_cross = i;
                    }
                }
            } else if(!second_buffer_processed) { //process the second buffer
                second_buffer_processed = true;
                previous_buffer_cross_point = find_crossing_point(read_from_buffer);
                for(int i = 1; i < 256; i++) {
                    //This if statement determines if we just crossed the crossing point
                    if(((read_from_buffer[i] > previous_buffer_cross_point) && 
                            (read_from_buffer[i-1] < previous_buffer_cross_point)) || 
                            ((read_from_buffer[i] < previous_buffer_cross_point) && 
                            (read_from_buffer[i-1] > previous_buffer_cross_point)))
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
            }
        }
        
        
    
        //update LCD
        //REFRESH_RATE - only update the LCD occasionally for readability
        const int REFRESH_RATE = 1000000;
        if(update_cycles >= REFRESH_RATE) {
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
