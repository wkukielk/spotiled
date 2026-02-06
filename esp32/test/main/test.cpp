/* Example test application for testable component.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "unity.h"
#include <stdio.h>
#include <string.h>

extern "C" void app_main(void) {
    printf("\nBegin Test\n");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
    printf("\nEnd Test\n");
}
