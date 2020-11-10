/* mock header */
#ifndef CMSIS_OS_H
#define CMSIS_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DESKTOP_TARGET

#include <stdint.h>

#define taskSCHEDULER_NOT_STARTED (0)
#define osMutex(A)
#define osMutexStaticDef(A,B)
#define pdMS_TO_TICKS(A) (A)

#define taskENTER_CRITICAL()
#define taskEXIT_CRITICAL()

#define pdTRUE (1)

#define osOK (1)
#define osWaitForever (0xFFFFFFFF)

#define eSetBits    (0)

#define xQueueCreateStatic(A,B,C,D) (D)

#define xTaskNotify(A,B,C) (void)0

#define portMAX_DELAY 0

typedef void* osMutexId;
typedef void* osThreadId;
typedef void* osStaticMutexDef_t;
typedef void* QueueHandle_t;

typedef uint32_t StaticQueue_t;

static inline void * osMutexCreate(void) { return (void*)0; }

static inline int xTaskGetSchedulerState(void)
{
    return !taskSCHEDULER_NOT_STARTED;
}

static inline int xTaskGetCurrentTaskHandle(void)
{
    return 0;
}

static const char * pcTaskGetName(int arg)
{
    (void)arg;
    return "Desktop test";
}

static inline void osThreadYield(void) {}

static inline int osKernelRunning(void) { return 0; }

static inline int osMutexWait(void * a, uint32_t b) {
    (void)a;
    (void)b;
    return osOK;
}

static inline int osMutexRelease(void * a) {
    (void)a;
    return osOK;
}

/* injection of test data */
typedef uint32_t    BaseType_t;
typedef BaseType_t TickType_t;

BaseType_t xQueuePeek( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait );

BaseType_t xQueueReceive( QueueHandle_t xQueue, void * const pvBuffer, TickType_t xTicksToWait );

BaseType_t xQueueSendToBack( QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait);

static osMutexId ap3_dataMutexHandle = 0U;

static void vTaskDelay(BaseType_t ms) {};

static osMutexId wind_buffersMutexHandle;
static osMutexId wind_resultMutexHandle;
static osMutexId logging_MutexHandle;

#define xSemaphoreTake(A, B) (void)A
#define xSemaphoreGive(A) (void)A

#endif

#ifdef __cplusplus
}
#endif

#endif
