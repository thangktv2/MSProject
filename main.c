#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "semphr.h"
#include "string.h"

#define USB_TASK_PRIORITY   8
#define CONVEYER_TASK_PRIORITY   8
#define REJECT_TASK_PRIORITY   8

TaskHandle_t rejecttask = NULL;
TaskHandle_t conveyertask = NULL;
TaskHandle_t usbtask = NULL;


int result[] = {-1,-1,-1,-1,-1,-1};

char prefix[2];
int input = -1;

static SemaphoreHandle_t mutex;

// Normal function, fill the result into an array
void fill(int A[], int size, int value) {
    int count = 1;
    for (int i = 0; i < size; i++)
    {
        if (count > 0) {
            if (A[i] == 0 || A[i] == 1) {}
            else {
                A[i] = value;
                count = 0;
            }
        }
        else break;
    }
    prefix[0] = ' ';
    int input = -1;

    for (int y = 0; y < size; y++)
    {
        printf("%d ", A[y]);
    }
    printf("\n");
}

// Normal function, take the first element from a result array
void take(int A[], int size) {
    int run = A[0];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size - i - 1; j++)
        {
            A[i] = A[i + 1];
        }
    }
    A[size - 1] = -1;
    for (int y = 0; y < size; y++)
    {
        printf("%d ", A[y]);
    }
    printf("\n");
}

// FreeRTOS task
void usb_task(void *pvParameters){
    while (1){
        if (xSemaphoreTake(mutex, 0) == pdTRUE){
            scanf("%1s%d", prefix, &input);
            if ((char)prefix[0] == 'C'){ // Conveyer Control
                gpio_put(PICO_DEFAULT_LED_PIN, input);
            }
            else if ((char)prefix[0] == 'F'){ // Fill result value for STEPPER MOTOR
                fill(result,sizeof(result)/sizeof(result[0]), input);
                gpio_put(PICO_DEFAULT_LED_PIN, 1);
            }
            else if ((char)prefix[0] == 'I'){
                // Initialize system, move STEPPER MOTOR to home position
            }
            else if ((char)prefix[0] == 'R'){
                // Use event group to ENABLE freertos task
            }
            else if ((char)prefix[0] == 'S'){
                // Use event group to DISABLE freertos task
            }
        }
        xSemaphoreGive(mutex);
    }
}

// FreeRTOS task
void reject_task(void *pvParameters){
    while (1){
        take(result,sizeof(result)/sizeof(result[0]));
        // code dk motor phan loai
    }
}

int main()
{
    stdio_init_all();
    while(!stdio_usb_init()){}

    mutex = xSemaphoreCreateMutex();

    // Configure conveyer
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_init(18);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_set_dir(18, GPIO_OUT);

    // FreeRTOS Task init
    // xTaskCreate(reject_task, "reject_task", 256, NULL, CONVEYER_TASK_PRIORITY, &rejecttask);
    xTaskCreate(usb_task, "usb_task", 2048, NULL, USB_TASK_PRIORITY, &usbtask);

    vTaskStartScheduler();

    while(1){}
}