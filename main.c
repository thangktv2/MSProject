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

char input[3];
int result[] = {-1,-1,-1,-1,-1,-1};

uint8_t conveyer = -1;
uint8_t reject = -1;


static SemaphoreHandle_t mutex;

// Normal function, fill the result into an array
void fill(int A[], int size, int result) {
    int count = 1;
    for (int i = 0; i < size; i++)
    {
        if (count > 0) {
            if (A[i] == 0 || A[i] == 1) {}
            else {
                A[i] = result;
                count = 0;
            }
        }
        else break;
    }
    input[0] = ' ';
}

void take(int A[], int size) {
    int run = A[0];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size - i - 1; j++)
        {
            A[i] = A[i + 1];
        }
    }
    A[size - 1] = -1;
}

// FreeRTOS task
void usb_task(void *pvParameters){
    while (1){
        if (xSemaphoreTake(mutex, 0) == pdTRUE){
            scanf("%s",input);
            printf("%s\n",input);
            if (strcmp(input, "C1") == 0) conveyer = 1;
            if (strcmp(input, "C0") == 0) conveyer = 0;
            if (strcmp(input, "R1") == 0) reject = 1;
            if (strcmp(input, "R0") == 0) reject = 0;
        }
        xSemaphoreGive(mutex);
    }
}

// FreeRTOS task
void conveyer_task(void *pvParameters){
    while (1)
    {
        if (xSemaphoreTake(mutex, 0) == pdTRUE){
            if (strcmp(input, "C1") == 0)   gpio_put(PICO_DEFAULT_LED_PIN,1); // run conveyer
            if (strcmp(input, "C0") == 0)   gpio_put(PICO_DEFAULT_LED_PIN,0); // stop conveyer
        }
        xSemaphoreGive(mutex);
    }
}

// FreeRTOS task
void reject_task(void *pvParameters){
    while (1){
        if (xSemaphoreTake(mutex, 0) == pdTRUE){
            if (strcmp(input, "R1") == 0) fill(result,sizeof(result)/sizeof(result[0]), 1); // Accept
            if (strcmp(input, "R0") == 0) fill(result,sizeof(result)/sizeof(result[0]), 0); // Reject
        }
        xSemaphoreGive(mutex);
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
    xTaskCreate(conveyer_task, "conveyer_task", 256, NULL, CONVEYER_TASK_PRIORITY, &conveyertask);
    xTaskCreate(reject_task, "reject_task", 256, NULL, CONVEYER_TASK_PRIORITY, &rejecttask);
    xTaskCreate(usb_task, "usb_task", 256, NULL, USB_TASK_PRIORITY, &usbtask);

    vTaskStartScheduler();

    while(1){}
}