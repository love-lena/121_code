/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "stdio.h"

#define BAUD_RATE 9600 
#define BIT_TIME (100000/BAUD_RATE)  /* Bit time in ticks */ 

QueueHandle_t print_queue;
QueueHandle_t error_queue;

void uart_printf(void){ 
    char print_string[100]; // string to print 
    // Initialize UART 
    Cy_SCB_UART_Init(UART_PRINTF_HW, &UART_PRINTF_config, &UART_PRINTF_context); 

    Cy_SCB_UART_Enable(UART_PRINTF_HW); 
    
    for (;;){ 
        xQueueReceive(print_queue, print_string, portMAX_DELAY);
        Cy_SCB_UART_PutString(UART_PRINTF_HW, print_string);
    } 
} 

void soft_uart_error(void) {
    uint8_t* error_byte;
    
    for (;;){ 
        xQueueReceive(error_queue, error_byte, portMAX_DELAY);
        continue;
        if(*error_byte == 1) {
            //Set LED to full brightness
            // GPIO ON CODE HERE
            //Delay for 5 seconds at 100khz
            vTaskDelay(500000);
        } else if(*error_byte == 2) {
            //Set LED to half brightness
            int ending_time = xTaskGetTickCount() + 500000;
            int light_switch = 1;
            while(xTaskGetTickCount() < ending_time) {
                // GPIO SET TO light_switch
                light_switch = !light_switch;
            }
        }
    } 
}

void soft_uart_reciever(void) {
    char recieved_data[100];
    
    for(;;) {

        while(Status_Reg_1_Read()) { //Wait until edge of start bit
            vTaskDelay(1);
        }
        
        //put ourselves in the middle of the start bit
        vTaskDelay(BIT_TIME/2);
        
        //Verify start bit is low
        if(Status_Reg_1_Read()) {
            xQueueSend(print_queue, "bad start bit\r\n", 0);
            continue;
        }
            
        
        //Read 8 bits
        uint8_t byte = 0;
        uint8_t parity = 0;
        for(int i = 0; i < 8; i++) {
            vTaskDelay(BIT_TIME);
            
            uint8_t bit = Status_Reg_1_Read(); //exactly one bit
            parity += bit;
            bit = bit << i; //Shift bit to its place, assuming LSB transmitted first
            byte = byte | bit; //Save bit to byte
        }
        
        sprintf(recieved_data, "%x\r\n", byte); //put byte in output string
        
        //Error checking
        //uint8_t *err;
        //*err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_1_Read(); //exactly one bit
        parity += bit;
        if(parity % 2 == 0) { //Check for odd parity
            //*err = 1;
            //xQueueSend(soft_uart_error, err, 0); //Print the string
            continue;
        }
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        //Verify stop bit is high
        if(!Status_Reg_1_Read()) {
            //*err = 2;
            //xQueueSend(soft_uart_error, err, 0); //Print the string
            continue;
        }
        xQueueSend(print_queue, recieved_data, 0); //Print the string
        
        while(!Status_Reg_1_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}

int main(void)
{
    __enable_irq(); 
    
    print_queue = xQueueCreate(4, 100);
    //error_queue = xQueueCreate(4, 1);
    
    xTaskCreate(soft_uart_reciever, "SOFT_UART_RECIEVE", 400, NULL, 2, 0); 
    //xTaskCreate(soft_uart_error, "SOFT_UART_ERR", 400, NULL, 1, 0); 
    xTaskCreate(uart_printf, "UART_PRINTF", 400, NULL, 1, 0); 
    
    vTaskStartScheduler(); /* Start FreeRTOS scheduler */ 
    /* The following code will not be executed. */
    for(;;){ } 
}

/* [] END OF FILE */
