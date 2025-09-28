/**
 * @file
 * @brief Binary Output API regression tests
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacstr.h"

/* Test that all required API functions exist with correct signatures */
static bool test_bo_api_signatures(void)
{
    /* Initialization */
    VERIFY_API_SIGNATURE(Binary_Output_Init, void, void);

    /* Instance management */
    VERIFY_API_SIGNATURE(Binary_Output_Valid_Instance, bool, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Count, unsigned, void);
    VERIFY_API_SIGNATURE(Binary_Output_Index_To_Instance, uint32_t, unsigned);
    VERIFY_API_SIGNATURE(Binary_Output_Instance_To_Index, unsigned, uint32_t);

    /* Object Name */
    VERIFY_API_SIGNATURE(Binary_Output_Object_Name, bool, uint32_t, BACNET_CHARACTER_STRING*);

    /* Present Value with Priority */
    VERIFY_API_SIGNATURE(Binary_Output_Present_Value, BACNET_BINARY_PV, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Present_Value_Set, bool, uint32_t, BACNET_BINARY_PV, unsigned);
    VERIFY_API_SIGNATURE(Binary_Output_Present_Value_Relinquish, bool, uint32_t, unsigned);

    /* Out of Service */
    VERIFY_API_SIGNATURE(Binary_Output_Out_Of_Service, bool, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Out_Of_Service_Set, void, uint32_t, bool);

    /* Polarity */
    VERIFY_API_SIGNATURE(Binary_Output_Polarity, BACNET_POLARITY, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Polarity_Set, bool, uint32_t, BACNET_POLARITY);

    /* Relinquish Default */
    VERIFY_API_SIGNATURE(Binary_Output_Relinquish_Default, BACNET_BINARY_PV, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Relinquish_Default_Set, bool, uint32_t, BACNET_BINARY_PV);

    /* Priority Array */
    VERIFY_API_SIGNATURE(Binary_Output_Priority_Array_Value, BACNET_BINARY_PV, uint32_t, unsigned);

    /* COV */
    VERIFY_API_SIGNATURE(Binary_Output_Change_Of_Value, bool, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Change_Of_Value_Clear, void, uint32_t);

    /* Text Properties */
    VERIFY_API_SIGNATURE(Binary_Output_Active_Text, const char*, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Active_Text_Set, bool, uint32_t, const char*);
    VERIFY_API_SIGNATURE(Binary_Output_Inactive_Text, const char*, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Inactive_Text_Set, bool, uint32_t, const char*);

    /* Description */
    VERIFY_API_SIGNATURE(Binary_Output_Description, const char*, uint32_t);
    VERIFY_API_SIGNATURE(Binary_Output_Description_Set, bool, uint32_t, const char*);

    /* Property Lists */
    VERIFY_API_SIGNATURE(Binary_Output_Property_Lists, void, const int**, const int**, const int**);

    /* Read/Write Property */
    VERIFY_API_SIGNATURE(Binary_Output_Read_Property, int, BACNET_READ_PROPERTY_DATA*);
    VERIFY_API_SIGNATURE(Binary_Output_Write_Property, bool, BACNET_WRITE_PROPERTY_DATA*);

    /* Note: Intrinsic_Reporting not implemented in this port */

    return true;
}

/* Test initialization and instance management */
static bool test_bo_initialization(void)
{
    /* Initialize binary outputs */
    Binary_Output_Init();

    /* Verify count */
    unsigned count = Binary_Output_Count();
    TEST_ASSERT(count > 0, "BO count should be greater than 0");
    TEST_ASSERT(count <= 65535, "BO count should be reasonable");

    /* Test instance validation */
    TEST_ASSERT_TRUE(Binary_Output_Valid_Instance(0));
    TEST_ASSERT_TRUE(Binary_Output_Valid_Instance(count - 1));
    TEST_ASSERT_FALSE(Binary_Output_Valid_Instance(count));
    TEST_ASSERT_FALSE(Binary_Output_Valid_Instance(0xFFFFFFFF));

    /* Test index to instance mapping */
    for (unsigned i = 0; i < count; i++) {
        uint32_t instance = Binary_Output_Index_To_Instance(i);
        unsigned index = Binary_Output_Instance_To_Index(instance);
        TEST_ASSERT_EQUAL(i, index);
    }

    return true;
}

/* Test Priority Array and Present Value operations */
static bool test_bo_priority_array(void)
{
    uint32_t instance = 0;
    BACNET_BINARY_PV value;
    bool result;

    /* Initialize to known state */
    Binary_Output_Init();

    /* Get initial value (should use relinquish default) */
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_INACTIVE, value);

    /* Set value at priority 8 (manual operator) */
    result = Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 8);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);

    /* Set value at higher priority (1 - life safety) */
    result = Binary_Output_Present_Value_Set(instance, BINARY_INACTIVE, 1);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_INACTIVE, value);

    /* Verify priority 8 is still set but not active */
    value = Binary_Output_Priority_Array_Value(instance, 8);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);

    /* Relinquish priority 1 */
    result = Binary_Output_Present_Value_Relinquish(instance, 1);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);  /* Should return to priority 8 value */

    /* Relinquish priority 8 */
    result = Binary_Output_Present_Value_Relinquish(instance, 8);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_INACTIVE, value);  /* Should return to relinquish default */

    /* Test invalid priority */
    result = Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 0);
    TEST_ASSERT_FALSE(result);
    result = Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 17);
    TEST_ASSERT_FALSE(result);

    return true;
}

/* Test Relinquish Default */
static bool test_bo_relinquish_default(void)
{
    uint32_t instance = 0;
    BACNET_BINARY_PV value;
    bool result;

    /* Get initial relinquish default */
    value = Binary_Output_Relinquish_Default(instance);
    TEST_ASSERT_EQUAL(BINARY_INACTIVE, value);

    /* Set new relinquish default */
    result = Binary_Output_Relinquish_Default_Set(instance, BINARY_ACTIVE);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Relinquish_Default(instance);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);

    /* Clear all priorities and verify present value uses relinquish default */
    for (unsigned i = 1; i <= 16; i++) {
        Binary_Output_Present_Value_Relinquish(instance, i);
    }
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);

    /* Restore default */
    result = Binary_Output_Relinquish_Default_Set(instance, BINARY_INACTIVE);
    TEST_ASSERT_TRUE(result);

    return true;
}

/* Test Out of Service property */
static bool test_bo_out_of_service(void)
{
    uint32_t instance = 0;
    bool oos;
    BACNET_BINARY_PV value;
    bool result;

    /* Get initial state */
    oos = Binary_Output_Out_Of_Service(instance);
    TEST_ASSERT_FALSE(oos);

    /* Set Out of Service */
    Binary_Output_Out_Of_Service_Set(instance, true);
    oos = Binary_Output_Out_Of_Service(instance);
    TEST_ASSERT_TRUE(oos);

    /* When OOS, should be able to write present value directly */
    result = Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 0);
    TEST_ASSERT_TRUE(result);
    value = Binary_Output_Present_Value(instance);
    TEST_ASSERT_EQUAL(BINARY_ACTIVE, value);

    /* Clear Out of Service */
    Binary_Output_Out_Of_Service_Set(instance, false);
    oos = Binary_Output_Out_Of_Service(instance);
    TEST_ASSERT_FALSE(oos);

    return true;
}

/* Test Polarity property */
static bool test_bo_polarity(void)
{
    uint32_t instance = 0;
    BACNET_POLARITY polarity;
    bool result;

    /* Get initial polarity */
    polarity = Binary_Output_Polarity(instance);
    TEST_ASSERT_EQUAL(POLARITY_NORMAL, polarity);

    /* Set reverse polarity */
    result = Binary_Output_Polarity_Set(instance, POLARITY_REVERSE);
    TEST_ASSERT_TRUE(result);
    polarity = Binary_Output_Polarity(instance);
    TEST_ASSERT_EQUAL(POLARITY_REVERSE, polarity);

    /* Restore normal polarity */
    result = Binary_Output_Polarity_Set(instance, POLARITY_NORMAL);
    TEST_ASSERT_TRUE(result);
    polarity = Binary_Output_Polarity(instance);
    TEST_ASSERT_EQUAL(POLARITY_NORMAL, polarity);

    return true;
}

/* Test Active and Inactive Text properties */
static bool test_bo_text_properties(void)
{
    uint32_t instance = 0;
    const char *text;
    bool result;

    /* Get default active text */
    text = Binary_Output_Active_Text(instance);
    TEST_ASSERT_NOT_NULL(text);

    /* Set custom active text */
    result = Binary_Output_Active_Text_Set(instance, "Running");
    TEST_ASSERT_TRUE(result);
    text = Binary_Output_Active_Text(instance);
    TEST_ASSERT_STRING_EQUAL("Running", text);

    /* Get default inactive text */
    text = Binary_Output_Inactive_Text(instance);
    TEST_ASSERT_NOT_NULL(text);

    /* Set custom inactive text */
    result = Binary_Output_Inactive_Text_Set(instance, "Stopped");
    TEST_ASSERT_TRUE(result);
    text = Binary_Output_Inactive_Text(instance);
    TEST_ASSERT_STRING_EQUAL("Stopped", text);

    /* Test invalid instance */
    text = Binary_Output_Active_Text(0xFFFF);
    TEST_ASSERT_NULL(text);
    text = Binary_Output_Inactive_Text(0xFFFF);
    TEST_ASSERT_NULL(text);

    return true;
}

/* Test COV operations */
static bool test_bo_cov(void)
{
    uint32_t instance = 0;
    bool changed;

    /* Clear any existing change flag */
    Binary_Output_Change_Of_Value_Clear(instance);
    changed = Binary_Output_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Make a change at priority 8 */
    Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 8);
    changed = Binary_Output_Change_Of_Value(instance);
    TEST_ASSERT_TRUE(changed);

    /* Clear change flag */
    Binary_Output_Change_Of_Value_Clear(instance);
    changed = Binary_Output_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Setting same value should not trigger COV */
    Binary_Output_Present_Value_Set(instance, BINARY_ACTIVE, 8);
    changed = Binary_Output_Change_Of_Value(instance);
    TEST_ASSERT_FALSE(changed);

    /* Changing value should trigger COV */
    Binary_Output_Present_Value_Set(instance, BINARY_INACTIVE, 8);
    changed = Binary_Output_Change_Of_Value(instance);
    TEST_ASSERT_TRUE(changed);

    return true;
}

/* Test Property Lists */
static bool test_bo_property_lists(void)
{
    const int *required = NULL;
    const int *optional = NULL;
    const int *proprietary = NULL;

    /* Get property lists */
    Binary_Output_Property_Lists(&required, &optional, &proprietary);

    /* Verify required properties */
    TEST_ASSERT_NOT_NULL(required);

    /* Check for essential required properties */
    bool found_object_id = false;
    bool found_object_name = false;
    bool found_object_type = false;
    bool found_present_value = false;
    bool found_status_flags = false;
    bool found_out_of_service = false;
    bool found_polarity = false;
    bool found_priority_array = false;
    bool found_relinquish_default = false;

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
            case PROP_POLARITY:
                found_polarity = true;
                break;
            case PROP_PRIORITY_ARRAY:
                found_priority_array = true;
                break;
            case PROP_RELINQUISH_DEFAULT:
                found_relinquish_default = true;
                break;
        }
    }

    TEST_ASSERT_TRUE(found_object_id);
    TEST_ASSERT_TRUE(found_object_name);
    TEST_ASSERT_TRUE(found_object_type);
    TEST_ASSERT_TRUE(found_present_value);
    TEST_ASSERT_TRUE(found_status_flags);
    TEST_ASSERT_TRUE(found_out_of_service);
    TEST_ASSERT_TRUE(found_polarity);
    TEST_ASSERT_TRUE(found_priority_array);
    TEST_ASSERT_TRUE(found_relinquish_default);

    return true;
}

/* Main test runner for Binary Output API */
void run_bo_api_tests(TestSuite *suite)
{
    current_suite = suite;

    RUN_TEST(test_bo_api_signatures);
    RUN_TEST(test_bo_initialization);
    RUN_TEST(test_bo_priority_array);
    RUN_TEST(test_bo_relinquish_default);
    RUN_TEST(test_bo_out_of_service);
    RUN_TEST(test_bo_polarity);
    RUN_TEST(test_bo_text_properties);
    RUN_TEST(test_bo_cov);
    RUN_TEST(test_bo_property_lists);
}
