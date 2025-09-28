/**
 * @file
 * @brief Device Object API regression tests
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include "test_framework.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacstr.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"

/* External device functions */
extern void Device_Init(const char *name);
extern bool Device_Valid_Object_Instance_Number(BACNET_OBJECT_TYPE object_type,
                                               uint32_t instance);
extern unsigned Device_Count(void);
extern uint32_t Device_Index_To_Instance(unsigned index);
extern bool Device_Object_Name(uint32_t instance, BACNET_CHARACTER_STRING *object_name);

/* Test that all required Device API functions exist with correct signatures */
static bool test_device_api_signatures(void)
{
    /* Initialization */
    VERIFY_API_SIGNATURE(Device_Init, void, const char*);

    /* Instance management */
    VERIFY_API_SIGNATURE(Device_Valid_Object_Instance_Number, bool,
                        BACNET_OBJECT_TYPE, uint32_t);
    VERIFY_API_SIGNATURE(Device_Count, unsigned, void);
    VERIFY_API_SIGNATURE(Device_Index_To_Instance, uint32_t, unsigned);

    /* Object properties */
    VERIFY_API_SIGNATURE(Device_Object_Name, bool, uint32_t, BACNET_CHARACTER_STRING*);
    VERIFY_API_SIGNATURE(Device_Object_Instance_Number, uint32_t, void);
    VERIFY_API_SIGNATURE(Device_Set_Object_Instance_Number, bool, uint32_t);

    /* System status */
    VERIFY_API_SIGNATURE(Device_System_Status, BACNET_DEVICE_STATUS, void);
    VERIFY_API_SIGNATURE(Device_System_Status_Set, int, BACNET_DEVICE_STATUS);

    /* Vendor information */
    VERIFY_API_SIGNATURE(Device_Vendor_Identifier, uint16_t, void);
    VERIFY_API_SIGNATURE(Device_Vendor_Name, const char*, void);

    /* Model and firmware */
    VERIFY_API_SIGNATURE(Device_Model_Name, const char*, void);
    VERIFY_API_SIGNATURE(Device_Firmware_Revision, const char*, void);
    VERIFY_API_SIGNATURE(Device_Application_Software_Version, const char*, void);

    /* Location and description */
    VERIFY_API_SIGNATURE(Device_Description, const char*, void);
    VERIFY_API_SIGNATURE(Device_Description_Set, bool, const char*);
    VERIFY_API_SIGNATURE(Device_Location, const char*, void);
    VERIFY_API_SIGNATURE(Device_Location_Set, bool, const char*);

    /* Protocol information */
    VERIFY_API_SIGNATURE(Device_Protocol_Version, uint16_t, void);
    VERIFY_API_SIGNATURE(Device_Protocol_Revision, uint16_t, void);
    VERIFY_API_SIGNATURE(Device_Protocol_Services_Supported,
                        BACNET_SERVICES_SUPPORTED*, void);
    VERIFY_API_SIGNATURE(Device_Protocol_Object_Types_Supported,
                        BACNET_BIT_STRING*, void);

    /* Max APDU */
    VERIFY_API_SIGNATURE(Device_Max_APDU_Length_Accepted, uint16_t, void);
    VERIFY_API_SIGNATURE(Device_Segmentation_Supported, BACNET_SEGMENTATION, void);

    /* Database revision */
    VERIFY_API_SIGNATURE(Device_Database_Revision, uint32_t, void);
    VERIFY_API_SIGNATURE(Device_Inc_Database_Revision, void, void);

    return true;
}

/* Test device initialization */
static bool test_device_initialization(void)
{
    /* Initialize device with custom name */
    Device_Init("PIC32-Test-Device");

    /* Verify device count (should be 1) */
    unsigned count = Device_Count();
    TEST_ASSERT_EQUAL(1, count);

    /* Get device instance */
    uint32_t instance = Device_Object_Instance_Number();
    TEST_ASSERT(instance > 0, "Device instance should be valid");
    TEST_ASSERT(instance <= 4194303, "Device instance should be within BACnet range");

    /* Verify device name */
    BACNET_CHARACTER_STRING name;
    bool result = Device_Object_Name(instance, &name);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT(name.length > 0, "Device name should not be empty");

    return true;
}

/* Test object instance validation */
static bool test_device_object_validation(void)
{
    /* Test valid object types and instances */
    TEST_ASSERT_TRUE(Device_Valid_Object_Instance_Number(OBJECT_DEVICE,
                     Device_Object_Instance_Number()));

    /* Initialize objects to ensure they exist */
    Analog_Input_Init();
    Analog_Output_Init();
    Binary_Input_Init();
    Binary_Output_Init();

    /* Test AI objects */
    if (Analog_Input_Count() > 0) {
        TEST_ASSERT_TRUE(Device_Valid_Object_Instance_Number(OBJECT_ANALOG_INPUT, 0));
        TEST_ASSERT_FALSE(Device_Valid_Object_Instance_Number(OBJECT_ANALOG_INPUT, 65535));
    }

    /* Test AO objects */
    if (Analog_Output_Count() > 0) {
        TEST_ASSERT_TRUE(Device_Valid_Object_Instance_Number(OBJECT_ANALOG_OUTPUT, 0));
        TEST_ASSERT_FALSE(Device_Valid_Object_Instance_Number(OBJECT_ANALOG_OUTPUT, 65535));
    }

    /* Test BI objects */
    if (Binary_Input_Count() > 0) {
        TEST_ASSERT_TRUE(Device_Valid_Object_Instance_Number(OBJECT_BINARY_INPUT, 0));
        TEST_ASSERT_FALSE(Device_Valid_Object_Instance_Number(OBJECT_BINARY_INPUT, 65535));
    }

    /* Test BO objects */
    if (Binary_Output_Count() > 0) {
        TEST_ASSERT_TRUE(Device_Valid_Object_Instance_Number(OBJECT_BINARY_OUTPUT, 0));
        TEST_ASSERT_FALSE(Device_Valid_Object_Instance_Number(OBJECT_BINARY_OUTPUT, 65535));
    }

    /* Test invalid object type */
    TEST_ASSERT_FALSE(Device_Valid_Object_Instance_Number(OBJECT_SCHEDULE, 0));

    return true;
}

/* Test device instance number */
static bool test_device_instance_number(void)
{
    uint32_t original = Device_Object_Instance_Number();
    bool result;

    /* Set new instance number */
    result = Device_Set_Object_Instance_Number(12345);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(12345, Device_Object_Instance_Number());

    /* Try invalid instance number (> 4194303) */
    result = Device_Set_Object_Instance_Number(4194304);
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(12345, Device_Object_Instance_Number());

    /* Restore original */
    Device_Set_Object_Instance_Number(original);

    return true;
}

/* Test system status */
static bool test_device_system_status(void)
{
    BACNET_DEVICE_STATUS status;
    int result;

    /* Get initial status */
    status = Device_System_Status();
    TEST_ASSERT_EQUAL(STATUS_OPERATIONAL, status);

    /* Set different status values */
    result = Device_System_Status_Set(STATUS_OPERATIONAL_READ_ONLY);
    TEST_ASSERT(result >= 0, "Should succeed");
    status = Device_System_Status();
    TEST_ASSERT_EQUAL(STATUS_OPERATIONAL_READ_ONLY, status);

    result = Device_System_Status_Set(STATUS_DOWNLOAD_REQUIRED);
    TEST_ASSERT(result >= 0, "Should succeed");
    status = Device_System_Status();
    TEST_ASSERT_EQUAL(STATUS_DOWNLOAD_REQUIRED, status);

    /* Restore operational */
    Device_System_Status_Set(STATUS_OPERATIONAL);

    return true;
}

/* Test vendor information */
static bool test_device_vendor_info(void)
{
    /* Get vendor ID */
    uint16_t vendor_id = Device_Vendor_Identifier();
    TEST_ASSERT(vendor_id > 0, "Vendor ID should be valid");

    /* Get vendor name */
    const char *vendor_name = Device_Vendor_Name();
    TEST_ASSERT_NOT_NULL(vendor_name);
    TEST_ASSERT(strlen(vendor_name) > 0, "Vendor name should not be empty");

    /* Get model name */
    const char *model_name = Device_Model_Name();
    TEST_ASSERT_NOT_NULL(model_name);
    TEST_ASSERT(strlen(model_name) > 0, "Model name should not be empty");

    /* Get firmware revision */
    const char *firmware = Device_Firmware_Revision();
    TEST_ASSERT_NOT_NULL(firmware);
    TEST_ASSERT(strlen(firmware) > 0, "Firmware revision should not be empty");

    /* Get application software version */
    const char *app_version = Device_Application_Software_Version();
    TEST_ASSERT_NOT_NULL(app_version);
    TEST_ASSERT(strlen(app_version) > 0, "App version should not be empty");

    return true;
}

/* Test location and description */
static bool test_device_location_description(void)
{
    const char *text;
    bool result;

    /* Test description */
    text = Device_Description();
    TEST_ASSERT_NOT_NULL(text);

    result = Device_Description_Set("Test Device Description");
    TEST_ASSERT_TRUE(result);
    text = Device_Description();
    TEST_ASSERT_STRING_EQUAL("Test Device Description", text);

    /* Test location */
    text = Device_Location();
    TEST_ASSERT_NOT_NULL(text);

    result = Device_Location_Set("Test Lab");
    TEST_ASSERT_TRUE(result);
    text = Device_Location();
    TEST_ASSERT_STRING_EQUAL("Test Lab", text);

    return true;
}

/* Test protocol information */
static bool test_device_protocol_info(void)
{
    /* Protocol version */
    uint16_t version = Device_Protocol_Version();
    TEST_ASSERT_EQUAL(1, version);  /* BACnet protocol version 1 */

    /* Protocol revision */
    uint16_t revision = Device_Protocol_Revision();
    TEST_ASSERT(revision >= 14, "Should be revision 14 or higher");

    /* Services supported */
    BACNET_SERVICES_SUPPORTED services;
    Device_Protocol_Services_Supported(&services);

    /* Check some required services */
    TEST_ASSERT_TRUE(services.Who_Is);
    TEST_ASSERT_TRUE(services.I_Am);
    TEST_ASSERT_TRUE(services.Read_Property);

    /* Object types supported */
    BACNET_BIT_STRING types;
    Device_Protocol_Object_Types_Supported(&types);

    /* Check supported object types */
    TEST_ASSERT_TRUE(bitstring_bit(&types, OBJECT_DEVICE));
    TEST_ASSERT_TRUE(bitstring_bit(&types, OBJECT_ANALOG_INPUT));
    TEST_ASSERT_TRUE(bitstring_bit(&types, OBJECT_ANALOG_OUTPUT));
    TEST_ASSERT_TRUE(bitstring_bit(&types, OBJECT_BINARY_INPUT));
    TEST_ASSERT_TRUE(bitstring_bit(&types, OBJECT_BINARY_OUTPUT));

    return true;
}

/* Test APDU and segmentation */
static bool test_device_apdu_segmentation(void)
{
    /* Max APDU length */
    uint16_t max_apdu = Device_Max_APDU_Length_Accepted();
    TEST_ASSERT(max_apdu >= 50, "Min APDU is 50");
    TEST_ASSERT(max_apdu <= 1476, "Max APDU for MS/TP is 1476");

    /* Segmentation */
    BACNET_SEGMENTATION segmentation = Device_Segmentation_Supported();
    TEST_ASSERT(segmentation <= SEGMENTATION_BOTH, "Valid segmentation value");

    return true;
}

/* Test database revision */
static bool test_device_database_revision(void)
{
    uint32_t initial_rev = Device_Database_Revision();

    /* Increment revision */
    Device_Inc_Database_Revision();
    uint32_t new_rev = Device_Database_Revision();
    TEST_ASSERT_EQUAL(initial_rev + 1, new_rev);

    /* Increment again */
    Device_Inc_Database_Revision();
    new_rev = Device_Database_Revision();
    TEST_ASSERT_EQUAL(initial_rev + 2, new_rev);

    return true;
}

/* Main test runner for Device API */
void run_device_api_tests(TestSuite *suite)
{
    current_suite = suite;

    RUN_TEST(test_device_api_signatures);
    RUN_TEST(test_device_initialization);
    RUN_TEST(test_device_object_validation);
    RUN_TEST(test_device_instance_number);
    RUN_TEST(test_device_system_status);
    RUN_TEST(test_device_vendor_info);
    RUN_TEST(test_device_location_description);
    RUN_TEST(test_device_protocol_info);
    RUN_TEST(test_device_apdu_segmentation);
    RUN_TEST(test_device_database_revision);
}