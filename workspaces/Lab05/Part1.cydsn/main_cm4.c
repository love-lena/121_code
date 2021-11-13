/* ========================================
 *
 * Lena Hickson Long
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t print_queue;

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

int main(void)
{
    __enable_irq(); 
    
    print_queue = xQueueCreate(4, 100);
    xQueueSend(print_queue, "Goodbye World\r\n", 0);   
    
    xTaskCreate(uart_printf, "UART_PRINTF", 400, NULL, 1, 0); 
    /* Assign a priority lower than other tasks */ 
    
    vTaskStartScheduler(); /* Start FreeRTOS scheduler */ 
    /* The following code will not be executed. */
    
    for(;;){ 
         
    } 
}

/* [] END OF FILE */
