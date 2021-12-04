/*****************************************************************************
 * Macros and definitions
 ****************************************************************************/
/**
 * \brief Return a pointer to the `SsTCB` associated to the task.
 *
 * This macro does not use `pvTaskGetThreadLocalStoragePointer()`. Instead it
 * uses the TCB_t structure directly.
 */
#define getSsTCB( x ) ( ( SsTCB_t * )( ( TCB_t * ) x )->pvThreadLocalStoragePointers[ configSS_STORAGE_POINTER_INDEX ] )

/**
 * \brief Return the system available slack.
 *
 * @return The system available slack.
 */
#define xTaskGetAvailableSlack() xSlackSD

/*****************************************************************************
 * Data declaration
 ****************************************************************************/
/**
 * \brief The system available slack.
 *
 * The system available slack is the minimum value of all the task's slacks.
 */
static volatile BaseType_t xSlackSD;

/**
 * \brief List of tasks blocked by insufficient available slack.
 *
 * This list stores tasks that have been blocked by insufficient available
 * slack. They need to be stored in a separated list because we don't know
 * in advance when there is enough available slack.
 *
 * The blocked (delayed) list of FreeRTOS stores real-time tasks that are
 * blocked in waiting of a resource, with a timeout or for an unspecified
 * amount of time. Although that list could be used to store the slack-blocked
 * tasks, identifying the tasks waiting for slack from the resource-blocked ones
 * by means of the \ref SsTCB could be time consuming.
 */
static List_t xSsTaskBlockedList;

/*****************************************************************************
 * Public functions declaration
 ****************************************************************************/

/**
 * \brief Updates the absolute deadline of \p pxTask.
 *
 * Remove the current release deadline and insert the deadline for the next release of \p pxTask.
 *
 * @param pxTask The task to which update its deadline.
 * @param xTimeToWake The task release time from which calculate the absolute deadline.
 */
#if ( configSS_VERIFY_DEADLINE == 1 )
static inline void vSlackUpdateDeadline( SsTCB_t *pxTask, TickType_t xTimeToWake ) __attribute__((always_inline));
#endif

/**
 * \brief Calculates the available slack of task \p xTask at \p xTc .
 *
 * This is a wrapper function that call the appropriate slack method.
 *
 * @param xTask The task which available slack should be calculated.
 * @param xTc The time at which the slack calculation should be done.
 */
static inline void vSlackCalculateSlack( TaskHandle_t xTask, const TickType_t xTc );

/**
 * \brief Add \p xTicks to all lower priority tasks than \p xTask .
 *
 * @param xTask Task which has gained slack.
 * @param xTicks Amount of ticks to add to the slack counters.
 */
static inline void vSlackGainSlack( const TaskHandle_t xTask, const TickType_t xTicks ) __attribute__((always_inline));

/**
 * \brief Moves all the aperiodic tasks in the xSsTaskBlockedList to the ready list.
 *
 * @return pdTRUE if a context switch is required.
 */
static BaseType_t xTaskSlackResume( void );

/**
 * \brief Decrement the slack counter of all the tasks.
 *
 * Subtract the specified \p xTicks amount from the slack counters of all the tasks.
 * The available slack of a task is stored in \ref SsTCB.xSlack.
 *
 * @param xTicks Amount of ticks to subtract from the available slack of the tasks.
 */
static inline void vSlackDecrementAllTasksSlack( const TickType_t xTicks ) __attribute__((always_inline));

/**
 * \brief Reduce \p xTicks to all higher priority tasks than \p pxTask.
 *
 * Substract the specified \p xTicks amount from the slack counters of higher priority tasks than \p xTask.
 *
 * @param pxTask The task currently running.
 * @param xTicks Amount of ticks to subtract from the slack counters of higher priority tasks.
 */
static inline void vSlackDecrementTasksSlack( TaskHandle_t xTask, const TickType_t xTicks ) __attribute__((always_inline));

/**
 * \brief Moves all the ready-to-execute aperiodic tasks from the ready list
 * to the slack suspended list.
 *
 * @return pdTRUE if a context switch is required.
 */
static BaseType_t xTaskSlackSuspend( void );
