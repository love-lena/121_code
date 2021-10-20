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

uint16 buffer_1[256];
uint16 buffer_2[256];

int working_buffer = 1; //1 and 2 are valid values
int readable_buffer = 0; //1 and 2 are valid values, 0 means none are ready

bool dma_1_error = false;

void DMA_done_ISR(void) {
    //uart_printf("transfer done!n\r");
    
    Cy_DMA_Channel_ClearInterrupt(DMA_1_HW, DMA_1_DW_CHANNEL);
    
    if (Cy_DMA_Channel_GetStatus(DMA_1_HW, DMA_1_DW_CHANNEL) ==
        CY_DMA_INTR_CAUSE_COMPLETION) {
            readable_buffer = working_buffer;
            working_buffer = working_buffer == 1 ? 2 : 1;   //switch working buffer
        } else {
            /* DMA error, abnormal termination */
            dma_1_error = true;
    }
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
    
    init_uart_printf();
    Cy_SysInt_Init(&DMA_1_INT_cfg, DMA_done_ISR);
    NVIC_EnableIRQ(DMA_1_INT_cfg.intrSrc);
    __enable_irq(); /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    
    /* Allocate descriptor */
    cy_stc_dma_channel_config_t channelConfig;
    /* Set parameters based on settings of DMA component */
    channelConfig.descriptor = &DMA_1_Descriptor_1;
    /* Start of descriptor chain */
    channelConfig.preemptable = DMA_1_PREEMPTABLE;
    channelConfig.priority = DMA_1_PRIORITY;
    channelConfig.enable = false;
    channelConfig.bufferable = DMA_1_BUFFERABLE;
    
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1,&DMA_1_Descriptor_1_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1,(uint32_t *) &(SAR->CHAN_RESULT));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1,buffer_1);
    
    Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2,&DMA_1_Descriptor_2_config);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2,(uint32_t *) &(SAR->CHAN_RESULT));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2,buffer_2);
        
        
    Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL,&channelConfig);
    
    Cy_DMA_Enable(DMA_1_HW);
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    
    Cy_DMA_Enable(DMA_1_HW);
    
    Cy_DMA_Channel_SetInterruptMask(DMA_1_HW, DMA_1_DW_CHANNEL, CY_DMA_INTR_MASK);
    
    ADC_1_Start();
    ADC_1_StartConvert();
    
    //Cy_TrigMux_Connect(TRIG14_IN_PASS_TR_SAR_OUT, TRIG14_OUT_TR_GROUP0_INPUT50,
    //    CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_PASS_TR_SAR_OUT);
    //Cy_TrigMux_Connect(TRIG1_IN_TR_GROUP14_OUTPUT7, TRIG1_OUT_CPUSS_DW1_TR_IN0,
    //    CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_TR_GROUP_OUTPUT__LEVEL);
        
    //Cy_TrigMux_Connect(TRIG14_IN_PASS_TR_CTDAC_EMPTY, TRIG14_OUT_TR_GROUP0_INPUT50,
    //    CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_PASS_TR_CTDAC_EMPTY);
    //Cy_TrigMux_Connect(TRIG1_IN_TR_GROUP14_OUTPUT7, TRIG1_OUT_CPUSS_DW1_TR_IN0,
    //    CY_TR_MUX_TR_INV_DISABLE, TRIGGER_TYPE_TR_GROUP_OUTPUT__LEVEL);
    
    
    bool print_output = false;
    
    for(;;)
    {
        if(readable_buffer) {
            uint16* read_from_buffer = readable_buffer==1 ? buffer_1 : buffer_2;
            readable_buffer = 0;
            
            int p1 = 0;
            int p2 = 0;
            int dir = read_from_buffer[1]-read_from_buffer[0];
            uint16 prev_value = read_from_buffer[1];
            
            //optimization needed: stop looping if p2 found and not printing
            for(int i = 2; i < 256; i++) { 
                if(!p2 && (read_from_buffer[i]-prev_value) * dir < 0) { //if dir change
                    if(!p1)
                        p1 = i;
                    else 
                        p2 = i;
                }
                prev_value = read_from_buffer[i];
                
                char str[8];
                sprintf(str, "%03x: ", i); 
                uint16 elem1 = read_from_buffer[i]; //max is FFFF
                char str2[8];
                sprintf(str2, "%04x ", elem1);
                
                if(print_output) {
                    uart_printf(str);
                    uart_printf(str2);
                    uart_printf("\n\r");
                }
            }
            
            // Calculate frequency
            int freq = 1000/(p2-p1);
            if(freq > 0 && freq < 101) {
                lcd_cursor(0,5);
                char msg[8];
                sprintf(msg, "%03dkHz", freq);
                lcd_write(msg, sizeof(msg));
            } else {
                lcd_cursor(0,2);
                char msg[] = "OUT OF RANGE";
                lcd_write(msg, sizeof(msg));
            }
            
        }
        
    }
}

/* [] END OF FILE */
