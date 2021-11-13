/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"

#define BAUD_RATE 9600 
#define BIT_TIME (100000/BAUD_RATE)  /* Bit time in ticks */ 

#define FRAMING_ERROR 1
#define PARITY_ERROR 2

QueueHandle_t print_queue;
QueueHandle_t error_queue;

void uart_printf(){ 
    char print_string[100]; // string to print 
    // Initialize UART 
    Cy_SCB_UART_Init(UART_PRINTF_HW, &UART_PRINTF_config, &UART_PRINTF_context); 

    Cy_SCB_UART_Enable(UART_PRINTF_HW); 
    
    for (;;){ 
        xQueueReceive(print_queue, print_string, portMAX_DELAY);
        Cy_SCB_UART_PutString(UART_PRINTF_HW, print_string);
    } 
} 

void soft_uart_error() {
    uint8_t error_byte = 0;
    
    TickType_t ending_time = 0;
    bool half_brightness = 0;
    bool half_brightness_switch = 0;
    
    
    for (;;){
        
        error_byte = 0;
        xQueueReceive(error_queue, &error_byte, 0);
        
        if(error_byte == FRAMING_ERROR) {
    
            //Turn on LED
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
            //Set delay for 5 seconds at 100khz
            ending_time = xTaskGetTickCount() + 500000;
            half_brightness = 0;
            
        } else if(error_byte == PARITY_ERROR) {

            //Enable half brightness
            half_brightness = 1;
            //Set delay for 5 seconds at 100khz
            ending_time = xTaskGetTickCount() + 500000;
            
        }
        
        if(half_brightness) {  
            
            //toggle LED
            half_brightness_switch = !half_brightness_switch;
            
            //2 on then off until next loop to create desired amount of brightness
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0);
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 0); 
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
            
        }
        
        //TickType_t current_tick = xTaskGetTickCount();
        
        //char recieved_data[100];
        //sprintf(recieved_data, "%u > %u = %d\r\n", current_tick, ending_time, (current_tick > ending_time)); //put byte in output string
        //xQueueSend(print_queue, recieved_data, 0);    
            
        if(xTaskGetTickCount() > ending_time) {
            //Turn off LED
            half_brightness = 0;
            Cy_GPIO_Write(LED_0_PORT, LED_0_NUM, 1);
        }
    } 
}

void soft_uart_reciever() {
    char recieved_data[100];
    
    for(;;) {

        while(Status_Reg_1_Read()) { //Wait until edge of start bit
            vTaskDelay(1);
        }
        
        //put ourselves in the middle of the start bit
        vTaskDelay(BIT_TIME/2);
        
        //Verify start bit is low
        if(Status_Reg_1_Read()) {
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
        uint8_t err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_1_Read(); //exactly one bit
        parity += bit; //get parity with parity bit
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        
        if(!Status_Reg_1_Read()) { //Verify stop bit is high
            
            err = FRAMING_ERROR; //Set framing error
            xQueueSend(error_queue, &err, 0);
            
        } else if(parity % 2 == 0) { //Check for odd parity
            
            err = PARITY_ERROR; //Set parity error
            xQueueSend(error_queue, &err, 0);
            
        }
        
        if(!err) {
            //xQueueSend(print_queue, recieved_data, 0); //Print the data
        }
        
        while(!Status_Reg_1_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}

int main(void)
{
    __enable_irq(); 
    
    print_queue = xQueueCreate(4, 100);
    error_queue = xQueueCreate(4, 1);
    
    xTaskCreate(soft_uart_reciever, "SOFT_UART_RECIEVE", 400, NULL, 2, 0); 
    xTaskCreate(soft_uart_error, "SOFT_UART_ERR", 400, NULL, 1, 0); 
    xTaskCreate(uart_printf, "UART_PRINTF", 400, NULL, 1, 0); 
    
    vTaskStartScheduler(); /* Start FreeRTOS scheduler */ 
    /* Nothing after this point will be executed. */
    for(;;){ } 
}

/* [] END OF FILE */
