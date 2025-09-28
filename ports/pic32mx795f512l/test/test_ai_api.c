/**
 * @file
 * @brief Analog Input API regression tests
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacstr.h"
#include "bacnet/bacenum.h"
#include <math.h>

/* Test that all required API functions exist with correct signatures */
static bool test_ai_api_signatures(void)
{
    /* Initialization */
    VERIFY_API_SIGNATURE(Analog_Input_Init, void, void);

    /* Instance management */
    VERIFY_API_SIGNATURE(Analog_Input_Valid_Instance, bool, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Count, unsigned, void);
    VERIFY_API_SIGNATURE(Analog_Input_Index_To_Instance, uint32_t, unsigned);
    VERIFY_API_SIGNATURE(Analog_Input_Instance_To_Index, unsigned, uint32_t);

    /* Object Name */
    VERIFY_API_SIGNATURE(Analog_Input_Object_Name, bool, uint32_t, BACNET_CHARACTER_STRING*);

    /* Present Value */
    VERIFY_API_SIGNATURE(Analog_Input_Present_Value, float, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Present_Value_Set, void, uint32_t, float);

    /* Units */
    VERIFY_API_SIGNATURE(Analog_Input_Units, uint16_t, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Units_Set, bool, uint32_t, uint16_t);

    /* Out of Service */
    VERIFY_API_SIGNATURE(Analog_Input_Out_Of_Service, bool, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Out_Of_Service_Set, void, uint32_t, bool);

    /* Reliability */
    VERIFY_API_SIGNATURE(Analog_Input_Reliability, BACNET_RELIABILITY, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Reliability_Set, bool, uint32_t, BACNET_RELIABILITY);

    /* COV */
    VERIFY_API_SIGNATURE(Analog_Input_COV_Increment, float, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_COV_Increment_Set, void, uint32_t, float);
    VERIFY_API_SIGNATURE(Analog_Input_Change_Of_Value, bool, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Change_Of_Value_Clear, void, uint32_t);

    /* Description */
    VERIFY_API_SIGNATURE(Analog_Input_Description, const char*, uint32_t);
    VERIFY_API_SIGNATURE(Analog_Input_Description_Set, bool, uint32_t, const char*);

    /* Property Lists */
    VERIFY_API_SIGNATURE(Analog_Input_Property_Lists, void, const int**, const int**, const int**);

    /* Read/Write Property */
    VERIFY_API_SIGNATURE(Analog_Input_Read_Property, int, BACNET_READ_PROPERTY_DATA*);
    VERIFY_API_SIGNATURE(Analog_Input_Write_Property, bool, BACNET_WRITE_PROPERTY_DATA*);

    /* Intrinsic Reporting */
    VERIFY_API_SIGNATURE(Analog_Input_Intrinsic_Reporting, void, uint32_t);

    return true;
}

/* Test initialization and instance management */
static bool test_ai_initialization(void)
{
    /* Initialize analog inputs */
    Analog_Input_Init();

    /* Verify count */
    unsigned count = Analog_Input_Count();
    TEST_ASSERT(count > 0, "AI count should be greater than 0");
    TEST_ASSERT(count <= 65535, "AI count should be reasonable");

    /* Test instance validation */
    TEST_ASSERT_TRUE(Analog_Input_Valid_Instance(0));
    TEST_ASSERT_TRUE(Analog_Input_Valid_Instance(count - 1));
    TEST_ASSERT_FALSE(Analog_Input_Valid_Instance(count));
    TEST_ASSERT_FALSE(Analog_Input_Valid_Instance(0xFFFFFFFF));

    /* Test index to instance mapping */
    for (unsigned i = 0; i < count; i++) {
        uint32_t instance = Analog_Input_Index_To_Instance(i);
        unsigned index = Analog_Input_Instance_To_Index(instance);
        TEST_ASSERT_EQUAL(i, index);
    }

    /* Test invalid index */
    unsigned invalid_index = Analog_Input_Instance_To_Index(0xFFFFFFFF);
    TEST_ASSERT(invalid_index >= count, "Invalid instance should return invalid index");

    return true;
}

/* Test Present Value operations */
static bool test_ai_present_value(void)
{
    uint32_t instance = 0;
    float value;

    /* Get initial value */
    value = Analog_Input_Present_Value(instance);
    TEST_ASSERT_FLOAT_WITHIN(0.0f, value, 0.001f);

    /* Set Out of Service to allow writes */
    Analog_Input_Out_Of_Service_Set(instance, true);

    /* Set and verify various values */
    Analog_Input_Present_Value_Set(instance, 25.5f);
    value = Analog_Input_Present_Value(instance);
    TEST_ASSERT_FLOAT_WITHIN(25.5f, value, 0.001f);

    Analog_Input_Present_Value_Set(instance, -40.0f);
    value = Analog_Input_Present_Value(instance);
    TEST_ASSERT_FLOAT_WITHIN(-40.0f, value, 0.001f);

    Analog_Input_Present_Value_Set(instance, 0.0f);
    value = Analog_Input_Present_Value(instance);
    TEST_ASSERT_FLOAT_WITHIN(0.0f, value, 0.001f);

    /* Restore Out of Service */
    Analog_Input_Out_Of_Service_Set(instance, false);

    return true;
}

/* Test Object Name operations */
static bool test_ai_object_name(void)
{
    uint32_t instance = 0;
    BACNET_CHARACTER_STRING name, test_name;
    bool result;

    /* Get default name */
    result = Analog_Input_Object_Name(instance, &name);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(name.length > 0, "Name should not be empty");

    /* Note: Object_Name_Set not available in this implementation */
    /* Just verify we can read the name */
    characterstring_init_ansi(&test_name, "Test-AI-1");

    /* Test invalid instance */
    result = Analog_Input_Object_Name(0xFFFF, &name);
    TEST_ASSERT_FALSE(result);

    return true;
}

/* Test Units property */
static bool test_ai_units(void)
{
    uint32_t instance = 0;
    uint16_t units;
    bool result;

    /* Get default units */
    units = Analog_Input_Units(instance);
    TEST_ASSERT(units < 256, "Units should be valid BACnet engineering units");

    /* Set new units */
    result = Analog_Input_Units_Set(instance, UNITS_DEGREES_FAHRENHEIT);
    TEST_ASSERT_TRUE(result);

    /* Verify units were set */
    units = Analog_Input_Units(instance);
    TEST_ASSERT_EQUAL(UNITS_DEGREES_FAHRENHEIT, units);

    /* Set different units */
    result = Analog_Input_Units_Set(instance, UNITS_PERCENT);
    TEST_ASSERT_TRUE(result);
    units = Analog_Input_Units(instance);
    TEST_ASSERT_EQUAL(UNITS_PERCENT, units);

    /* Invalid instance should return NO_UNITS or fail gracefully */
    units = Analog_Input_Units(0xFFFF);
    TEST_ASSERT_EQUAL(UNITS_NO_UNITS, units);

    return true;
}

/* Test Out of Service property */
static bool test_ai_out_of_service(void)
{
    uint32_t instance = 0;
    bool oos;

    /* Get initial state */
    oos = Analog_Input_Out_Of_Service(instance);
    TEST_ASSERT_FALSE(oos);

    /* Set Out of Service */
    Analog_Input_Out_Of_Service_Set(instance, true);
    oos = Analog_Input_Out_Of_Service(instance);
    TEST_ASSERT_TRUE(oos);

    /* Clear Out of Service */
    Analog_Input_Out_Of_Service_Set(instance, false);
    oos = Analog_Input_Out_Of_Service(instance);
    TEST_ASSERT_FALSE(oos);

    /* Invalid instance should return false */
    oos = Analog_Input_Out_Of_Service(0xFFFF);
    TEST_ASSERT_FALSE(oos);

    return true;
}

/* Test Reliability property */
static bool test_ai_reliability(void)
{
    uint32_t instance = 0;
    BACNET_RELIABILITY reliability;
    bool result;

    /* Get initial reliability */
    reliability = Analog_Input_Reliability(instance);
    TEST_ASSERT_EQUAL(RELIABILITY_NO_FAULT_DETECTED, reliability);

    /* Set different reliability values */
    result = Analog_Input_Reliability_Set(instance, RELIABILITY_NO_SENSOR);
    TEST_ASSERT_TRUE(result);
    reliability = Analog_Input_Reliability(instance);
    TEST_ASSERT_EQUAL(RELIABILITY_NO_SENSOR, reliability);

    result = Analog_Input_Reliability_Set(instance, RELIABILITY_OVER_RANGE);
    TEST_ASSERT_TRUE(result);
    reliability = Analog_Input_Reliability(instance);
    TEST_ASSERT_EQUAL(RELIABILITY_OVER_RANGE, reliability);

    /* Restore to no fault */
    result = Analog_Input_Reliability_Set(instance, RELIABILITY_NO_FAULT_DETECTED);
    TEST_ASSERT_TRUE(result);
    reliability = Analog_Input_Reliability(instance);
    TEST_ASSERT_EQUAL(RELIABILITY_NO_FAULT_DETECTED, reliability);

    /* Invalid instance */
    reliability = Analog_Input_Reliability(0xFFFF);
    TEST_ASSERT_EQUAL(RELIABILITY_NO_FAULT_DETECTED, reliability);

    return true;
}

/* Test COV operations */
static bool test_ai_cov(void)
{
    uint32_t instance = 0;
    float cov_increment;
    bool changed;

    /* Get default COV increment */
    cov_increment = Analog_Input_COV_Increment(instance);
    TEST_ASSERT(cov_increment > 0.0f, "COV increment should be positive");

    /* Set new COV increment */
    Analog_Input_COV_Increment_Set(instance, 0.5f);
    cov_increment = Analog_Input_COV_Increment(instance);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, cov_increment, 0.001f);

    /* Clear any existing change flag */
    Analog_Input_Change_Of_Value_Clear(instance);
    changed = Analog_Input_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Set Out of Service to allow value changes */
    Analog_Input_Out_Of_Service_Set(instance, true);

    /* Make a small change (less than COV increment) */
    float original = Analog_Input_Present_Value(instance);
    Analog_Input_Present_Value_Set(instance, original + 0.1f);
    changed = Analog_Input_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Make a large change (more than COV increment) */
    Analog_Input_Present_Value_Set(instance, original + 1.0f);
    changed = Analog_Input_Change_Of_Value(instance);
    TEST_ASSERT_TRUE(changed);

    /* Clear change flag */
    Analog_Input_Change_Of_Value_Clear(instance);
    changed = Analog_Input_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Restore Out of Service */
    Analog_Input_Out_Of_Service_Set(instance, false);

    return true;
}

/* Test Description property */
static bool test_ai_description(void)
{
    uint32_t instance = 0;
    const char *description;
    bool result;

    /* Get default description */
    description = Analog_Input_Description(instance);
    TEST_ASSERT_NOT_NULL(description);
    TEST_ASSERT(strlen(description) > 0, "Description should not be empty");

    /* Set new description */
    result = Analog_Input_Description_Set(instance, "Temperature Sensor");
    TEST_ASSERT_TRUE(result);

    /* Verify description was set */
    description = Analog_Input_Description(instance);
    TEST_ASSERT_NOT_NULL(description);
    TEST_ASSERT_STRING_EQUAL("Temperature Sensor", description);

    /* Test empty description */
    result = Analog_Input_Description_Set(instance, "");
    TEST_ASSERT_TRUE(result);
    description = Analog_Input_Description(instance);
    TEST_ASSERT_NOT_NULL(description);
    TEST_ASSERT_STRING_EQUAL("", description);

    /* Invalid instance should return NULL */
    description = Analog_Input_Description(0xFFFF);
    TEST_ASSERT_NULL(description);

    return true;
}

/* Test Property Lists */
static bool test_ai_property_lists(void)
{
    const int *required = NULL;
    const int *optional = NULL;
    const int *proprietary = NULL;

    /* Get property lists */
    Analog_Input_Property_Lists(&required, &optional, &proprietary);

    /* Verify required properties */
    TEST_ASSERT_NOT_NULL(required);

    /* Check for essential required properties */
    bool found_object_id = false;
    bool found_object_name = false;
    bool found_object_type = false;
    bool found_present_value = false;
    bool found_status_flags = false;
    bool found_out_of_service = false;
    bool found_units = false;

    for (int i = 0; required[i] != -1; i++) {
        switch (required[i]) {
            case PROP_OBJECT_IDENTIFIER:
                found_object_id = true;
                break;
            case PROP_OBJECT_NAME:
                found_object_name = true;
                break;
            case PROP_OBJECT_TYPE:
                found_object_type = true;
                break;
            case PROP_PRESENT_VALUE:
                found_present_value = true;
                break;
            case PROP_STATUS_FLAGS:
                found_status_flags = true;
                break;
            case PROP_OUT_OF_SERVICE:
                found_out_of_service = true;
                break;
            case PROP_UNITS:
                found_units = true;
                break;
        }
    }

    TEST_ASSERT_TRUE(found_object_id);
    TEST_ASSERT_TRUE(found_object_name);
    TEST_ASSERT_TRUE(found_object_type);
    TEST_ASSERT_TRUE(found_present_value);
    TEST_ASSERT_TRUE(found_status_flags);
    TEST_ASSERT_TRUE(found_out_of_service);
    TEST_ASSERT_TRUE(found_units);

    /* Optional properties should be defined but can be empty */
    TEST_ASSERT_NOT_NULL(optional);

    return true;
}

/* Main test runner for Analog Input API */
void run_ai_api_tests(TestSuite *suite)
{
    current_suite = suite;

    RUN_TEST(test_ai_api_signatures);
    RUN_TEST(test_ai_initialization);
    RUN_TEST(test_ai_present_value);
    RUN_TEST(test_ai_object_name);
    RUN_TEST(test_ai_units);
    RUN_TEST(test_ai_out_of_service);
    RUN_TEST(test_ai_reliability);
    RUN_TEST(test_ai_cov);
    RUN_TEST(test_ai_description);
    RUN_TEST(test_ai_property_lists);
}
