/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/
#include "project.h"
#include <stdlib.h>
#include <stdio.h>

//void init_uart_printf(void);
//void uart_printf(char *print_string);

//Data ready flag
bool data_ready = false;

void PDM_PCM_ISR(void) {
    
    uint32_t interrupts = Cy_PDM_PCM_GetInterruptStatusMasked(PDM_PCM_1_HW);
    Cy_PDM_PCM_ClearInterrupt(PDM_PCM_1_HW, interrupts);
    
    data_ready = true;
    
    //Disable RX FIFO NOT EMPTY interrupt
    PDM_PCM_1_SetInterruptMask(PDM_PCM_1_GetInterruptMask() & ~CY_PDM_PCM_INTR_RX_NOT_EMPTY);
    
}

int main(void)
{
    //init_uart_printf();
    
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
    
    //Data ready to send
    bool ready_to_play = false;
    
    for(;;)
    {
        //char sw2_on[2];
        //sprintf(sw2_on, "%u", Cy_GPIO_Read(SW2_0_PORT, SW2_0_NUM));
        //uart_printf(sw2_on);
        
        Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, Cy_GPIO_Read(switch_2_0_PORT, switch_2_0_NUM));
        CyDelay(1000);
        
        //Clear FIFO
        PDM_PCM_1_ClearFifo();
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
        
        
    }
}

/* [] END OF FILE */
