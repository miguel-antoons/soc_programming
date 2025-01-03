#include <assert.h>
#include <cache.h>
#include <defs.h>
#include <locks.h>
#include <stdio.h>
#include <taskman/taskman.h>

#include <implement_me.h>

/// @brief Maximum number of wait handlers.
#define TASKMAN_NUM_HANDLERS 32

/// @brief Maximum number of scheduled tasks.
#define TASKMAN_NUM_TASKS 128

/// @brief Maximum total stack size.
#define TASKMAN_STACK_SIZE (256 << 10)

#define TASKMAN_LOCK_ID 2

#define TASKMAN_LOCK()             \
    do {                           \
        get_lock(TASKMAN_LOCK_ID); \
    } while (0)

#define TASKMAN_RELEASE()              \
    do {                               \
        release_lock(TASKMAN_LOCK_ID); \
    } while (0)

__global static struct {
    /// @brief Wait handlers.
    struct taskman_handler* handlers[TASKMAN_NUM_HANDLERS];

    /// @brief Number of wait handlers;
    size_t handlers_count;

    /// @brief Stack area. Contains multiple independent stacks.
    uint8_t stack[TASKMAN_STACK_SIZE];

    /// @brief Stack offset (for the next allocation).
    size_t stack_offset;

    /// @brief Scheduled tasks.
    void* tasks[TASKMAN_NUM_TASKS];

    /// @brief Number of tasks scheduled.
    size_t tasks_count;

    /// @brief True if the task manager should stop.
    uint32_t should_stop;
} taskman;

/**
 * @brief Extra information attached to the coroutine used by the task manager.
 *
 */
struct task_data {
    struct {
        /// @brief Handler
        /// @note NULL if waiting on `coro_yield`.
        struct taskman_handler* handler;

        /// @brief Argument to the wait handler
        void* arg;
    } wait;

    /// @brief 1 if running, 0 otherwise.
    int running;
};

void taskman_glinit() {
    taskman.handlers_count = 0;
    taskman.stack_offset = 0;
    taskman.tasks_count = 0;
    taskman.should_stop = 0;
}

// struct task_data* task = (struct task_data*)((uint8_t*) &taskman.stack) + taskman.stack_offset + stack_sz);
// taskman.stack_offset += sizeof(task_data);
void* taskman_spawn(coro_fn_t coro_fn, void* arg, size_t stack_sz) {
    // (1) allocate stack space for the new task
    // (2) initialize the coroutine and struct task_data
    // (3) register the coroutine in the tasks array
    // use die_if_not() statements to handle error conditions (like no memory)

    die_if_not(TASKMAN_STACK_SIZE >= taskman.stack_offset + stack_sz);
    die_if_not(TASKMAN_NUM_TASKS > taskman.tasks_count);

    // (1)
    uint8_t* stack = (uint8_t*)&taskman.stack + taskman.stack_offset;
    taskman.stack_offset += stack_sz;

    // (2)
    coro_init(stack, stack_sz, coro_fn, arg);

    struct task_data* data = (struct task_data*)coro_data(stack);
    data->wait.handler = NULL;
    data->wait.arg = arg;

    // (3)
    taskman.tasks[taskman.tasks_count] = stack;
    taskman.tasks_count += 1;
}

void taskman_loop() {
    // (a) Call the `loop` functions of all the wait handlers.
    // (b) Iterate over all the tasks, and resume them if.
    //        * The task is not complete.
    //        * it yielded using `taskman_yield`.
    //        * the waiting handler says it can be resumed.
    while (!taskman.should_stop) {

        // (a)
        for (int i = 0; i < taskman.handlers_count; i += 1) {
            // taskman.handlers[i]->loop(taskman.handlers[i]);
            taskman.handlers[i]->loop(NULL);
        }

        // (b)
        for (int i = 0; i < taskman.tasks_count; i += 1) {
            void* coro_stack = taskman.tasks[i];
            // * 1
            void* result;
            TASKMAN_LOCK();
            if (coro_completed(coro_stack, &result)) {
                continue;
            }
            struct task_data* task = coro_data(coro_stack);
            if (task->running) {
                TASKMAN_RELEASE();
                continue;
            }
            task->running = 1;
            TASKMAN_RELEASE();

            // * 2
            if (task->wait.handler == NULL) {
                coro_resume(coro_stack);
                TASKMAN_LOCK();
                task->running = 0;
                TASKMAN_RELEASE();
                continue;
            }

            int can_resume = task->wait.handler->can_resume(task->wait.handler, coro_stack, task->wait.arg);
            // * 3
            if (can_resume) {
                coro_resume(coro_stack);
            }
            TASKMAN_LOCK();
            task->running = 0;
            TASKMAN_RELEASE();
        }
    }
    printf("exit loop\n");
}

void taskman_stop() {
    TASKMAN_LOCK();
    taskman.should_stop = 1;
    printf("taskman stopped: %d\n", taskman.should_stop);
    TASKMAN_RELEASE();
}

void taskman_register(struct taskman_handler* handler) {
    die_if_not(handler != NULL);
    die_if_not(taskman.handlers_count < TASKMAN_NUM_HANDLERS);

    taskman.handlers[taskman.handlers_count] = handler;
    taskman.handlers_count++;
}

void taskman_wait(struct taskman_handler* handler, void* arg) {
    void* stack = coro_stack();
    struct task_data* task_data = coro_data(stack);

    // I suggest that you read `struct taskman_handler` definition.
    // Call handler->on_wait, see if there is a need to yield.
    // Update the wait field of the task_data.
    // Yield if necessary.

    int should_yield = handler == NULL || !handler->on_wait(handler, stack, arg);
    task_data->wait.handler = handler;
    task_data->wait.arg = arg;
    if (should_yield) {
        coro_yield();
    } else {
        task_data->wait.handler = NULL;
        task_data->wait.arg = NULL;
    }
}

void taskman_yield() {
    taskman_wait(NULL, NULL);
}

void taskman_return(void* result) {
    coro_return(result);
}
