/**
 * @file
 * @brief Binary Input API regression tests (stub)
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/basic/object/bi.h"

/* Minimal stub implementation for now */
static bool test_bi_api_basic(void)
{
    /* Basic initialization test */
    Binary_Input_Init();

    unsigned count = Binary_Input_Count();
    TEST_ASSERT(count > 0, "BI count should be greater than 0");

    return true;
}

void run_bi_api_tests(TestSuite *suite)
{
    current_suite = suite;
    RUN_TEST(test_bi_api_basic);
}
