#include <Arduino.h>
#include <unity.h>
#include "kernel/scheduler.h"

void emptyTask() {  }

void test_scheduler_add_task() {
    TEST_ASSERT_TRUE(kernel.addTask(emptyTask, 1000, 0));
    TEST_ASSERT_EQUAL(1, kernel.getTaskCount());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_scheduler_add_task);
    UNITY_END();
}

void loop() {}