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

//uint16_t buffer_1[256];
//uint16_t buffer_2[256];

int working_buffer = 1; //1 and 2 are valid values
int readable_buffer = 0; //1 and 2 are valid values, 0 means none are ready

bool dma_1_error = false;

void DMA_done_ISR(void) {
    uart_printf("transfer done!\n\r");
    
    //readable_buffer = working_buffer;
    //working_buffer = working_buffer == 1 ? 2 : 1;
            
    //char printme[32];        
    //sprintf(printme, "%d \n\rf", (int)Cy_DMA_Channel_GetStatus(DMA_1_HW, DMA_1_DW_CHANNEL));
    //uart_printf(printme);
            
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);
            
}

void lcd_init(void);
void lcd_write(char *data, uint8_t size);
void lcd_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

int main(void)
{
    lcd_init();
    lcd_cursor(0,2);
    char msg[] = "Starting....";
    lcd_write(msg, sizeof(msg));
    
    uint16_t sample_array[256];
    
    init_uart_printf(); 
    
    __enable_irq();
    
    cy_stc_dma_channel_config_t channelConfig;
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *) &(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,sample_array);
    
    //Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2,&DMA_1_Descriptor_2_config);
    //Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2,(uint32_t *) &(SAR->CHAN_RESULT[0]));
    //Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2,buffer_2);
    
    Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    Cy_DMA_Enable(DMA_1_HW);
    
    Cy_SysInt_Init(&DMA_1_INT_cfg, DMA_done_ISR);
    NVIC_EnableIRQ(DMA_1_INT_cfg.intrSrc);
    
    Cy_DMA_Channel_SetInterruptMask(DMA_1_HW, DMA_1_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    ADC_1_Start();
    ADC_1_StartConvert();
    
    bool print_output = true;
    
    uint16_t max_value[50];
    uint16_t min_value[50];
    uint16_t max_max_value=0;
    uint16_t min_max_value=UINT16_MAX;
    for(int i = 0; i < 50; i++) {
        min_value[i] = UINT16_MAX;    
    }
    int j = 0;
    int q = 0;
    for(;;) {
        if(j < 50) {
            for(int i = 0; i<256; i++) {
                if(sample_array[i] > max_value[j])
                    max_value[j] = sample_array[i];
                else if(sample_array[i] < min_value[j])
                    min_value[j] = sample_array[i];
            }
            j++;
        } else {
            for(int i = 0; i<50; i++) {
                if(max_value[i] > max_max_value)
                    max_max_value = max_value[i];
                else if(max_value[i] < min_max_value)
                    min_max_value = max_value[i];
            }
            q = max_max_value - min_max_value;
            Cy_SysLib_Delay(1);
        }
    }
    
}

/* [] END OF FILE */
