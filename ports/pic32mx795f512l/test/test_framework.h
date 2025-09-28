/**
 * @file
 * @brief Test framework for PIC32MX795F512L BACnet port regression tests
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Test result tracking */
typedef struct {
    const char *test_name;
    const char *api_function;
    bool passed;
    const char *error_message;
    int line_number;
    const char *file_name;
} TestResult;

typedef struct {
    const char *suite_name;
    int total_tests;
    int passed_tests;
    int failed_tests;
    TestResult *results;
    int max_results;
    clock_t start_time;
    clock_t end_time;
} TestSuite;

/* Test macros */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            test_record_failure(__func__, #condition, message, __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "Expected %ld but got %ld", \
                (long)(expected), (long)(actual)); \
            test_record_failure(__func__, #expected " == " #actual, msg, __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            test_record_failure(__func__, #ptr " != NULL", "Pointer is NULL", __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            test_record_failure(__func__, #ptr " == NULL", "Pointer is not NULL", __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) \
    TEST_ASSERT((condition), "Condition is false")

#define TEST_ASSERT_FALSE(condition) \
    TEST_ASSERT(!(condition), "Condition is true")

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            char msg[512]; \
            snprintf(msg, sizeof(msg), "Expected \"%s\" but got \"%s\"", \
                (expected), (actual)); \
            test_record_failure(__func__, "String equality", msg, __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_FLOAT_WITHIN(expected, actual, tolerance) \
    do { \
        float diff = (expected) - (actual); \
        if (diff < 0) diff = -diff; \
        if (diff > (tolerance)) { \
            char msg[256]; \
            snprintf(msg, sizeof(msg), "Expected %f ± %f but got %f", \
                (float)(expected), (float)(tolerance), (float)(actual)); \
            test_record_failure(__func__, "Float within tolerance", msg, __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

/* API signature verification macros */
#define VERIFY_API_SIGNATURE(function_name, return_type, ...) \
    do { \
        typedef return_type (*expected_signature)(__VA_ARGS__); \
        expected_signature fp = (expected_signature)&function_name; \
        if (fp == NULL) { \
            test_record_failure(__func__, #function_name, \
                "Function not found", __LINE__, __FILE__); \
            return false; \
        } \
    } while(0)

/* Test suite functions */
TestSuite *test_suite_create(const char *name, int max_tests);
void test_suite_destroy(TestSuite *suite);
void test_suite_run(TestSuite *suite, bool (*test_func)(void), const char *test_name);
void test_suite_report(TestSuite *suite, FILE *output);
void test_suite_generate_xml(TestSuite *suite, const char *filename);
void test_suite_generate_json(TestSuite *suite, const char *filename);

/* Test recording functions */
void test_record_failure(const char *test_name, const char *condition,
                         const char *message, int line, const char *file);
void test_record_success(const char *test_name);

/* Current test suite context */
extern TestSuite *current_suite;

/* Helper to run a test and record result */
#define RUN_TEST(test_func) \
    test_suite_run(current_suite, test_func, #test_func)

#endif /* TEST_FRAMEWORK_H */