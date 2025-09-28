/**
 * @file
 * @brief Test framework implementation
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include <stdlib.h>

TestSuite *current_suite = NULL;

TestSuite *test_suite_create(const char *name, int max_tests)
{
    TestSuite *suite = calloc(1, sizeof(TestSuite));
    if (suite) {
        suite->suite_name = name;
        suite->max_results = max_tests;
        suite->results = calloc(max_tests, sizeof(TestResult));
        suite->total_tests = 0;
        suite->passed_tests = 0;
        suite->failed_tests = 0;
    }
    return suite;
}

void test_suite_destroy(TestSuite *suite)
{
    if (suite) {
        free(suite->results);
        free(suite);
    }
}

void test_record_failure(const char *test_name, const char *condition,
                         const char *message, int line, const char *file)
{
    if (current_suite && current_suite->total_tests < current_suite->max_results) {
        TestResult *result = &current_suite->results[current_suite->total_tests - 1];
        result->passed = false;
        result->error_message = message;
        result->line_number = line;
        result->file_name = file;
    }
}

void test_record_success(const char *test_name)
{
    if (current_suite && current_suite->total_tests < current_suite->max_results) {
        TestResult *result = &current_suite->results[current_suite->total_tests - 1];
        result->passed = true;
    }
}

void test_suite_run(TestSuite *suite, bool (*test_func)(void), const char *test_name)
{
    if (!suite || suite->total_tests >= suite->max_results) {
        return;
    }

    current_suite = suite;
    TestResult *result = &suite->results[suite->total_tests];
    result->test_name = test_name;
    result->passed = false;
    result->error_message = NULL;

    suite->total_tests++;

    if (suite->total_tests == 1) {
        suite->start_time = clock();
    }

    bool success = test_func();

    if (success) {
        result->passed = true;
        suite->passed_tests++;
    } else {
        suite->failed_tests++;
    }

    suite->end_time = clock();
}

void test_suite_report(TestSuite *suite, FILE *output)
{
    if (!suite || !output) return;

    double elapsed = (double)(suite->end_time - suite->start_time) / CLOCKS_PER_SEC;

    fprintf(output, "\n========================================\n");
    fprintf(output, "Test Suite: %s\n", suite->suite_name);
    fprintf(output, "========================================\n");
    fprintf(output, "Total Tests: %d\n", suite->total_tests);
    fprintf(output, "Passed:      %d\n", suite->passed_tests);
    fprintf(output, "Failed:      %d\n", suite->failed_tests);
    fprintf(output, "Time:        %.3f seconds\n", elapsed);
    fprintf(output, "Result:      %s\n",
            suite->failed_tests == 0 ? "PASS" : "FAIL");

    if (suite->failed_tests > 0) {
        fprintf(output, "\nFailed Tests:\n");
        fprintf(output, "--------------\n");

        for (int i = 0; i < suite->total_tests; i++) {
            TestResult *result = &suite->results[i];
            if (!result->passed) {
                fprintf(output, "  %s\n", result->test_name);
                if (result->error_message) {
                    fprintf(output, "    Error: %s\n", result->error_message);
                    if (result->file_name && result->line_number > 0) {
                        fprintf(output, "    Location: %s:%d\n",
                                result->file_name, result->line_number);
                    }
                }
            }
        }
    }

    fprintf(output, "\n");
}

void test_suite_generate_xml(TestSuite *suite, const char *filename)
{
    if (!suite || !filename) return;

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    double elapsed = (double)(suite->end_time - suite->start_time) / CLOCKS_PER_SEC;

    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<testsuite name=\"%s\" tests=\"%d\" failures=\"%d\" time=\"%.3f\">\n",
            suite->suite_name, suite->total_tests, suite->failed_tests, elapsed);

    for (int i = 0; i < suite->total_tests; i++) {
        TestResult *result = &suite->results[i];
        fprintf(fp, "  <testcase name=\"%s\" classname=\"%s\"",
                result->test_name, suite->suite_name);

        if (result->passed) {
            fprintf(fp, "/>\n");
        } else {
            fprintf(fp, ">\n");
            fprintf(fp, "    <failure message=\"%s\"",
                    result->error_message ? result->error_message : "Test failed");
            if (result->file_name && result->line_number > 0) {
                fprintf(fp, " type=\"AssertionError\">%s:%d</failure>\n",
                        result->file_name, result->line_number);
            } else {
                fprintf(fp, "/>\n");
            }
            fprintf(fp, "  </testcase>\n");
        }
    }

    fprintf(fp, "</testsuite>\n");
    fclose(fp);
}

void test_suite_generate_json(TestSuite *suite, const char *filename)
{
    if (!suite || !filename) return;

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    double elapsed = (double)(suite->end_time - suite->start_time) / CLOCKS_PER_SEC;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"suite_name\": \"%s\",\n", suite->suite_name);
    fprintf(fp, "  \"total_tests\": %d,\n", suite->total_tests);
    fprintf(fp, "  \"passed_tests\": %d,\n", suite->passed_tests);
    fprintf(fp, "  \"failed_tests\": %d,\n", suite->failed_tests);
    fprintf(fp, "  \"elapsed_time\": %.3f,\n", elapsed);
    fprintf(fp, "  \"result\": \"%s\",\n", suite->failed_tests == 0 ? "PASS" : "FAIL");
    fprintf(fp, "  \"tests\": [\n");

    for (int i = 0; i < suite->total_tests; i++) {
        TestResult *result = &suite->results[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"name\": \"%s\",\n", result->test_name);
        fprintf(fp, "      \"passed\": %s", result->passed ? "true" : "false");

        if (!result->passed) {
            if (result->error_message) {
                fprintf(fp, ",\n      \"error\": \"%s\"", result->error_message);
            }
            if (result->file_name && result->line_number > 0) {
                fprintf(fp, ",\n      \"location\": \"%s:%d\"",
                        result->file_name, result->line_number);
            }
        }

        fprintf(fp, "\n    }");
        if (i < suite->total_tests - 1) {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");
    fclose(fp);
}