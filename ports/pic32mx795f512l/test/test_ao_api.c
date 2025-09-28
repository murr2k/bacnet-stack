/**
 * @file
 * @brief Analog Output API regression tests (stub)
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/basic/object/ao.h"

/* Minimal stub implementation for now */
static bool test_ao_api_basic(void)
{
    /* Basic initialization test */
    Analog_Output_Init();

    unsigned count = Analog_Output_Count();
    TEST_ASSERT(count > 0, "AO count should be greater than 0");

    return true;
}

void run_ao_api_tests(TestSuite *suite)
{
    current_suite = suite;
    RUN_TEST(test_ao_api_basic);
}