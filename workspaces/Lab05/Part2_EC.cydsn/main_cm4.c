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
    char print_string[50]; // string to print 
    // Initialize UART 
    Cy_SCB_UART_Init(UART_PRINTF_HW, &UART_PRINTF_config, &UART_PRINTF_context); 

    Cy_SCB_UART_Enable(UART_PRINTF_HW); 
    
    for (;;){ 
        xQueueReceive(print_queue, print_string, portMAX_DELAY);
        Cy_SCB_UART_PutString(UART_PRINTF_HW, print_string);
    } 
} 

void soft_uart_error() {
    uint8_t error;
    
    TickType_t ending_time = 0;
    
    //0 -> off
    //1 -> half
    //2 -> full
    int brightness_level = 0;
    
    for (;;){
        
        error = 0;
        
        xQueueReceive(error_queue, &error, 0);
        
        if(error == FRAMING_ERROR) {
    
            brightness_level = 2;
            //Set delay for 5 seconds at 100khz
            ending_time = xTaskGetTickCount() + 500000;
            
            //xQueueSend(print_queue, "F", 0);
            
        } else if(error == PARITY_ERROR) {

            brightness_level = 1;
            //Set delay for 5 seconds at 100khz
            ending_time = xTaskGetTickCount() + 500000;
            
            //xQueueSend(print_queue, "P", 0);
        }
        
        //uart 0 - red led
        if(brightness_level == 2) {
            
            Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, 0);
            
        } else if(brightness_level == 1) {
            
            Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, 0); 
            Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, 0); 
            Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, 1);
            
        }
        
        if(xTaskGetTickCount() > ending_time) {
            
            //Turn off LED
            brightness_level = 0;
            Cy_GPIO_Write(RED_LED_0_PORT, RED_LED_0_NUM, 1);
            
        }
            
        
    } 
}

//soft uart helper functions


void soft_uart_0() {
    //char recieved_data[100];
    
    TickType_t one_second_from_now = xTaskGetTickCount() + 50000;
    unsigned int bytes_recieved = 0;
    char bps_string[20];
    
    for(;;) {

        while(Status_Reg_0_Read()) { //Wait until edge of start bit
            vTaskDelay(1);
        }
        
        //put ourselves in the middle of the start bit
        vTaskDelay(BIT_TIME/2);
        
        //Verify start bit is low
        if(Status_Reg_0_Read()) {
            continue;
        }
            
        
        //Read 8 bits
        uint8_t byte = 0;
        uint8_t parity = 0;
        for(int i = 0; i < 8; i++) {
            vTaskDelay(BIT_TIME);
            
            uint8_t bit = Status_Reg_0_Read(); //exactly one bit
            parity += bit;
            bit = bit << i; //Shift bit to its place, assuming LSB transmitted first
            byte = byte | bit; //Save bit to byte
        }
        
        //sprintf(recieved_data, "%x\r\n", byte); //put byte in output string
        
        //Error checking
        uint8_t err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_0_Read(); //exactly one bit
        parity += bit; //get parity with parity bit
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        
        if(!Status_Reg_0_Read()) { //Verify stop bit is high
            
            err = FRAMING_ERROR; //Set framing error
            xQueueSend(error_queue, &err, 0);
            
        } else if(parity % 2 == 0) { //Check for odd parity
            
            err = PARITY_ERROR; //Set parity error
            xQueueSend(error_queue, &err, 0);
            
        }
        
        if(!err) {
            bytes_recieved++;
        }
        
        //Print throughput every second
        if(false && xTaskGetTickCount() > one_second_from_now) {
            
            sprintf(bps_string, "0:%d\r\n", bytes_recieved);
            //sprintf(bps_string, "%d", bytes_recieved);
            xQueueSend(print_queue, bps_string, 0);
            
            bytes_recieved = 0;
            one_second_from_now = xTaskGetTickCount() + 100000;
            
        }
        
        while(!Status_Reg_0_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}

void soft_uart_1() {
    char recieved_data[10];
    
    TickType_t one_second_from_now = xTaskGetTickCount() + 50000;
    unsigned int bytes_recieved = 0;
    char bps_string[20];
    
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
        
        //sprintf(recieved_data, "%x\r\n", byte); //put byte in output string
        //xQueueSend(print_queue, recieved_data, 0);
        
        //Error checking
        uint8_t err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_1_Read(); //exactly one bit
        parity += bit; //get parity with parity bit
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        
        if(!Status_Reg_1_Read()) { //Verify stop bit is high
            
            //xQueueSend(print_queue, "F", 0);
            err = FRAMING_ERROR; //Set framing error
            xQueueSend(error_queue, &err, 0);
            
        } else if(parity % 2 == 0) { //Check for odd parity
            
            //xQueueSend(print_queue, "P", 0);
            err = PARITY_ERROR; //Set parity error
            xQueueSend(error_queue, &err, 0);
            
        }
        
        if(!err) {
            bytes_recieved++;
        }
        
        //Print throughput every second
        if(false && xTaskGetTickCount() > one_second_from_now) {
            
            sprintf(bps_string, "1:%d\r\n", bytes_recieved);
            //sprintf(bps_string, "%d", bytes_recieved);
            xQueueSend(print_queue, bps_string, 0);
            
            bytes_recieved = 0;
            one_second_from_now = xTaskGetTickCount() + 100000;
            
        }
        
        while(!Status_Reg_1_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}

void soft_uart_2() {
    char recieved_data[10];
    
    TickType_t one_second_from_now = xTaskGetTickCount() + 50000;
    unsigned int bytes_recieved = 0;
    char bps_string[20];
    
    for(;;) {

        while(Status_Reg_2_Read()) { //Wait until edge of start bit
            vTaskDelay(1);
        }
        
        //put ourselves in the middle of the start bit
        vTaskDelay(BIT_TIME/2);
        
        //Verify start bit is low
        if(Status_Reg_2_Read()) {
            continue;
        }
            
        
        //Read 8 bits
        uint8_t byte = 0;
        uint8_t parity = 0;
        for(int i = 0; i < 8; i++) {
            vTaskDelay(BIT_TIME);
            
            uint8_t bit = Status_Reg_2_Read(); //exactly one bit
            parity += bit;
            bit = bit << i; //Shift bit to its place, assuming LSB transmitted first
            byte = byte | bit; //Save bit to byte
        }
        
        sprintf(recieved_data, "%x\r\n", byte); //put byte in output string
        xQueueSend(print_queue, recieved_data, 0);
        
        //Error checking
        uint8_t err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_2_Read(); //exactly one bit
        parity += bit; //get parity with parity bit
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        
        if(!Status_Reg_2_Read()) { //Verify stop bit is high
            
            err = FRAMING_ERROR; //Set framing error
            xQueueSend(error_queue, &err, 0);
            
        } else if(parity % 2 == 0) { //Check for odd parity
            
            err = PARITY_ERROR; //Set parity error
            xQueueSend(error_queue, &err, 0);
            
        }
        
        if(!err) {
            bytes_recieved++;
        }
        
        //Print throughput every second
        if(false && xTaskGetTickCount() > one_second_from_now) {
            
            sprintf(bps_string, "2:%d\r\n", bytes_recieved);
            //sprintf(bps_string, "%d", bytes_recieved);
            xQueueSend(print_queue, bps_string, 0);
            
            bytes_recieved = 0;
            one_second_from_now = xTaskGetTickCount() + 100000;
            
        }
        
        while(!Status_Reg_2_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}

void soft_uart_3() {
    //char recieved_data[100];
    
    TickType_t one_second_from_now = xTaskGetTickCount() + 50000;
    unsigned int bytes_recieved = 0;
    char bps_string[20];
    
    for(;;) {

        while(Status_Reg_3_Read()) { //Wait until edge of start bit
            vTaskDelay(1);
        }
        
        //put ourselves in the middle of the start bit
        vTaskDelay(BIT_TIME/2);
        
        //Verify start bit is low
        if(Status_Reg_3_Read()) {
            continue;
        }
            
        
        //Read 8 bits
        uint8_t byte = 0;
        uint8_t parity = 0;
        for(int i = 0; i < 8; i++) {
            vTaskDelay(BIT_TIME);
            
            uint8_t bit = Status_Reg_3_Read(); //exactly one bit
            parity += bit;
            bit = bit << i; //Shift bit to its place, assuming LSB transmitted first
            byte = byte | bit; //Save bit to byte
        }
        
        //sprintf(recieved_data, "%x\r\n", byte); //put byte in output string
        
        //Error checking
        uint8_t err = 0;
        
        vTaskDelay(BIT_TIME);
        //We are on the parity bit
        uint8_t bit = Status_Reg_3_Read(); //exactly one bit
        parity += bit; //get parity with parity bit
        
        vTaskDelay(BIT_TIME);
        //We are on the stop bit
        
        if(!Status_Reg_3_Read()) { //Verify stop bit is high
            
            err = FRAMING_ERROR; //Set framing error
            xQueueSend(error_queue, &err, 0);
            
        } else if(parity % 2 == 0) { //Check for odd parity
            
            err = PARITY_ERROR; //Set parity error
            xQueueSend(error_queue, &err, 0);
            
        }
        
        if(!err) {
            bytes_recieved++;
        }
        
        //Print throughput every second
        if(false && xTaskGetTickCount() > one_second_from_now) {
            
            sprintf(bps_string, "3:%d\r\n", bytes_recieved);
            //sprintf(bps_string, "%d", bytes_recieved);
            xQueueSend(print_queue, bps_string, 0);
            
            bytes_recieved = 0;
            one_second_from_now = xTaskGetTickCount() + 100000;
            
        }
        
        while(!Status_Reg_3_Read()) { //wait until we go high
            vTaskDelay(1);
        }
    }
}


int main(void)
{
    __enable_irq(); 
    
    print_queue = xQueueCreate(4, 50);
    error_queue = xQueueCreate(4, 1);
    
    xTaskCreate(soft_uart_0, "SOFT_UART_0", 400, NULL, 2, 0); 
    xTaskCreate(soft_uart_1, "SOFT_UART_1", 400, NULL, 2, 0); 
    //xTaskCreate(soft_uart_2, "SOFT_UART_2", 400, NULL, 2, 0); 
    //xTaskCreate(soft_uart_3, "SOFT_UART_3", 400, NULL, 2, 0); 
    
    xTaskCreate(soft_uart_error, "SOFT_UART_ERR", 400, NULL, 1, 0); 
    
    xTaskCreate(uart_printf, "UART_PRINTF", 400, NULL, 1, 0); 
    
    vTaskStartScheduler(); /* Start FreeRTOS scheduler */ 
    /* Nothing after this point will be executed. */
    for(;;){ } 
}

/* [] END OF FILE */
