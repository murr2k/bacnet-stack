/**
 * @file
 * @brief Device Object for PIC32MX795F512L BACnet MS/TP
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacenum.h"
#include "bacnet/bactext.h"
#include "bacnet/config.h"
#include "bacnet/wp.h"
#include "bacnet/basic/object/device.h"
#include "bacnet/basic/services.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/datalink/dlmstp.h"

/* Device configuration - using CETCI BACnet Module requirements */
#define DEVICE_VENDOR_IDENTIFIER 815 /* CETCI vendor ID */
#define DEVICE_VENDOR_NAME "CETCI"
#define DEVICE_MODEL_NAME "PIC32MX795F512L"
#define DEVICE_FIRMWARE_REVISION "1.0.0"
#define DEVICE_APPLICATION_SOFTWARE_VERSION "1.0.0"
#define DEVICE_LOCATION "CETCI Lab"
#define DEVICE_DESCRIPTION "BACnet MS/TP Module"

/* BACnet object counts */
#define MAX_ANALOG_INPUTS 8
#define MAX_ANALOG_OUTPUTS 2
#define MAX_BINARY_INPUTS 4
#define MAX_BINARY_OUTPUTS 4

/* Device object instance */
static uint32_t Device_Object_Instance = 1234;

/**
 * @brief Return the Device object instance number
 */
uint32_t Device_Object_Instance_Number(void)
{
    return Device_Object_Instance;
}

/**
 * @brief Set the Device object instance number
 */
bool Device_Set_Object_Instance_Number(uint32_t instance)
{
    bool status = false;

    if (instance <= BACNET_MAX_INSTANCE) {
        /* Make the change */
        Device_Object_Instance = instance;
        /* Update the data link layer */
        dlmstp_set_mac_address(instance & 0x7F);
        /* We could send an I-Am broadcast to let the world know */
        status = true;
    }

    return status;
}

/**
 * @brief Return whether the Device object is valid
 */
bool Device_Valid_Object_Instance_Number(uint32_t object_id)
{
    return (Device_Object_Instance == object_id);
}

/**
 * @brief Get vendor identifier
 */
uint16_t Device_Vendor_Identifier(void)
{
    return DEVICE_VENDOR_IDENTIFIER;
}

/**
 * @brief Get product identifier
 */
uint16_t Device_Product_Identifier(void)
{
    return 1; /* Generic product ID */
}

/**
 * @brief Get vendor name
 */
const char *Device_Vendor_Name(void)
{
    return DEVICE_VENDOR_NAME;
}

/**
 * @brief Get model name
 */
const char *Device_Model_Name(void)
{
    return DEVICE_MODEL_NAME;
}

/**
 * @brief Get firmware revision
 */
const char *Device_Firmware_Revision(void)
{
    return DEVICE_FIRMWARE_REVISION;
}

/**
 * @brief Get application software version
 */
const char *Device_Application_Software_Version(void)
{
    return DEVICE_APPLICATION_SOFTWARE_VERSION;
}

/**
 * @brief Get device location
 */
const char *Device_Location(void)
{
    return DEVICE_LOCATION;
}

/**
 * @brief Get device description
 */
const char *Device_Description(void)
{
    return DEVICE_DESCRIPTION;
}

/**
 * @brief Get protocol version
 */
uint8_t Device_Protocol_Version(void)
{
    return BACNET_PROTOCOL_VERSION;
}

/**
 * @brief Get protocol revision
 */
uint8_t Device_Protocol_Revision(void)
{
    return BACNET_PROTOCOL_REVISION;
}

/**
 * @brief Get max APDU length
 */
uint16_t Device_Max_APDU_Length_Accepted(void)
{
    return MAX_APDU;
}

/**
 * @brief Get segmentation support
 */
BACNET_SEGMENTATION Device_Segmentation_Supported(void)
{
    return SEGMENTATION_NONE;
}

/**
 * @brief Get database revision
 */
uint32_t Device_Database_Revision(void)
{
    return 1; /* Initial revision */
}

/**
 * @brief Get system status
 */
BACNET_DEVICE_STATUS Device_System_Status(void)
{
    return STATUS_OPERATIONAL;
}

/**
 * @brief Get device object name
 */
bool Device_Object_Name(
    uint32_t instance,
    BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;

    if (instance == Device_Object_Instance) {
        characterstring_init_ansi(object_name, "PIC32MX795F512L");
        status = true;
    }

    return status;
}

/**
 * @brief Copy the device object name
 */
bool Device_Object_Name_Copy(
    BACNET_OBJECT_TYPE object_type,
    uint32_t instance,
    BACNET_CHARACTER_STRING *object_name)
{
    if (object_type == OBJECT_DEVICE) {
        return Device_Object_Name(instance, object_name);
    }
    return false;
}

/**
 * @brief Set device object name (not supported)
 */
bool Device_Set_Object_Name(const BACNET_CHARACTER_STRING *object_name)
{
    (void)object_name;
    return false; /* Read-only */
}

/**
 * @brief Initialize the device object
 */
void Device_Init(object_functions_t *object_table)
{
    /* Initialize object table with supported objects */
    object_table[0].Object_Type = OBJECT_DEVICE;
    object_table[0].Object_Init = NULL;
    object_table[0].Object_Count = Device_Count;
    object_table[0].Object_Index_To_Instance = Device_Index_To_Instance;
    object_table[0].Object_Valid_Instance = Device_Valid_Object_Instance_Number;
    object_table[0].Object_Name = Device_Object_Name_Copy;
    object_table[0].Object_Read_Property = Device_Read_Property_Local;
    object_table[0].Object_Write_Property = Device_Write_Property_Local;
    object_table[0].Object_RPM_List = Device_Property_Lists;
    object_table[0].Object_RR_Info = NULL;
    object_table[0].Object_Iterator = NULL;
    object_table[0].Object_Value_List = NULL;
    object_table[0].Object_COV = NULL;
    object_table[0].Object_COV_Clear = NULL;
    object_table[0].Object_Intrinsic_Reporting = NULL;

    /* Analog Input */
    object_table[1].Object_Type = OBJECT_ANALOG_INPUT;
    object_table[1].Object_Init = Analog_Input_Init;
    object_table[1].Object_Count = Analog_Input_Count;
    object_table[1].Object_Index_To_Instance = Analog_Input_Index_To_Instance;
    object_table[1].Object_Valid_Instance = Analog_Input_Valid_Instance;
    object_table[1].Object_Name = Analog_Input_Object_Name;
    object_table[1].Object_Read_Property = Analog_Input_Read_Property;
    object_table[1].Object_Write_Property = Analog_Input_Write_Property;
    object_table[1].Object_RPM_List = Analog_Input_Property_Lists;
    object_table[1].Object_RR_Info = NULL;
    object_table[1].Object_Iterator = NULL;
    object_table[1].Object_Value_List = NULL;
    object_table[1].Object_COV = Analog_Input_Change_Of_Value;
    object_table[1].Object_COV_Clear = Analog_Input_Change_Of_Value_Clear;
    object_table[1].Object_Intrinsic_Reporting = Analog_Input_Intrinsic_Reporting;

    /* Analog Output */
    object_table[2].Object_Type = OBJECT_ANALOG_OUTPUT;
    object_table[2].Object_Init = Analog_Output_Init;
    object_table[2].Object_Count = Analog_Output_Count;
    object_table[2].Object_Index_To_Instance = Analog_Output_Index_To_Instance;
    object_table[2].Object_Valid_Instance = Analog_Output_Valid_Instance;
    object_table[2].Object_Name = Analog_Output_Object_Name;
    object_table[2].Object_Read_Property = Analog_Output_Read_Property;
    object_table[2].Object_Write_Property = Analog_Output_Write_Property;
    object_table[2].Object_RPM_List = Analog_Output_Property_Lists;
    object_table[2].Object_RR_Info = NULL;
    object_table[2].Object_Iterator = NULL;
    object_table[2].Object_Value_List = NULL;
    object_table[2].Object_COV = Analog_Output_Change_Of_Value;
    object_table[2].Object_COV_Clear = Analog_Output_Change_Of_Value_Clear;
    object_table[2].Object_Intrinsic_Reporting = NULL;

    /* Binary Input */
    object_table[3].Object_Type = OBJECT_BINARY_INPUT;
    object_table[3].Object_Init = Binary_Input_Init;
    object_table[3].Object_Count = Binary_Input_Count;
    object_table[3].Object_Index_To_Instance = Binary_Input_Index_To_Instance;
    object_table[3].Object_Valid_Instance = Binary_Input_Valid_Instance;
    object_table[3].Object_Name = Binary_Input_Object_Name;
    object_table[3].Object_Read_Property = Binary_Input_Read_Property;
    object_table[3].Object_Write_Property = Binary_Input_Write_Property;
    object_table[3].Object_RPM_List = Binary_Input_Property_Lists;
    object_table[3].Object_RR_Info = NULL;
    object_table[3].Object_Iterator = NULL;
    object_table[3].Object_Value_List = NULL;
    object_table[3].Object_COV = Binary_Input_Change_Of_Value;
    object_table[3].Object_COV_Clear = Binary_Input_Change_Of_Value_Clear;
    object_table[3].Object_Intrinsic_Reporting = Binary_Input_Intrinsic_Reporting;

    /* Binary Output */
    object_table[4].Object_Type = OBJECT_BINARY_OUTPUT;
    object_table[4].Object_Init = Binary_Output_Init;
    object_table[4].Object_Count = Binary_Output_Count;
    object_table[4].Object_Index_To_Instance = Binary_Output_Index_To_Instance;
    object_table[4].Object_Valid_Instance = Binary_Output_Valid_Instance;
    object_table[4].Object_Name = Binary_Output_Object_Name;
    object_table[4].Object_Read_Property = Binary_Output_Read_Property;
    object_table[4].Object_Write_Property = Binary_Output_Write_Property;
    object_table[4].Object_RPM_List = Binary_Output_Property_Lists;
    object_table[4].Object_RR_Info = NULL;
    object_table[4].Object_Iterator = NULL;
    object_table[4].Object_Value_List = NULL;
    object_table[4].Object_COV = Binary_Output_Change_Of_Value;
    object_table[4].Object_COV_Clear = Binary_Output_Change_Of_Value_Clear;
    object_table[4].Object_Intrinsic_Reporting = NULL;

    /* Mark end of object table */
    object_table[5].Object_Type = MAX_BACNET_OBJECT_TYPE;
}

/**
 * @brief Return the number of device objects (always 1)
 */
unsigned Device_Count(void)
{
    return 1;
}

/**
 * @brief Map index to instance (always returns device instance)
 */
uint32_t Device_Index_To_Instance(unsigned index)
{
    (void)index;
    return Device_Object_Instance;
}

/**
 * @brief Encode a device object property into the APDU
 */
int Device_Read_Property_Local(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BIT_STRING bit_string;

    if ((rpdata->object_type != OBJECT_DEVICE) ||
        (rpdata->object_instance != Device_Object_Instance)) {
        return BACNET_STATUS_ERROR;
    }

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&rpdata->application_data[0],
                OBJECT_DEVICE, Device_Object_Instance);
            break;

        case PROP_OBJECT_NAME:
            Device_Object_Name(Device_Object_Instance, &char_string);
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&rpdata->application_data[0],
                OBJECT_DEVICE);
            break;

        case PROP_SYSTEM_STATUS:
            apdu_len = encode_application_enumerated(&rpdata->application_data[0],
                Device_System_Status());
            break;

        case PROP_VENDOR_NAME:
            characterstring_init_ansi(&char_string, Device_Vendor_Name());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_VENDOR_IDENTIFIER:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                Device_Vendor_Identifier());
            break;

        case PROP_MODEL_NAME:
            characterstring_init_ansi(&char_string, Device_Model_Name());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_FIRMWARE_REVISION:
            characterstring_init_ansi(&char_string, Device_Firmware_Revision());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_APPLICATION_SOFTWARE_VERSION:
            characterstring_init_ansi(&char_string, Device_Application_Software_Version());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_LOCATION:
            characterstring_init_ansi(&char_string, Device_Location());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_DESCRIPTION:
            characterstring_init_ansi(&char_string, Device_Description());
            apdu_len = encode_application_character_string(&rpdata->application_data[0],
                &char_string);
            break;

        case PROP_PROTOCOL_VERSION:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                Device_Protocol_Version());
            break;

        case PROP_PROTOCOL_REVISION:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                Device_Protocol_Revision());
            break;

        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            bitstring_init(&bit_string);
            /* Per PICS, we support these services */
            bitstring_set_bit(&bit_string, SERVICE_SUPPORTED_I_AM, true);
            bitstring_set_bit(&bit_string, SERVICE_SUPPORTED_WHO_IS, true);
            bitstring_set_bit(&bit_string, SERVICE_SUPPORTED_READ_PROPERTY, true);
            bitstring_set_bit(&bit_string, SERVICE_SUPPORTED_WRITE_PROPERTY, true);
            bitstring_set_bit(&bit_string, SERVICE_SUPPORTED_DEVICE_COMMUNICATION_CONTROL, true);
            apdu_len = encode_application_bitstring(&rpdata->application_data[0],
                &bit_string);
            break;

        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            bitstring_init(&bit_string);
            /* Per PICS, we support these objects */
            bitstring_set_bit(&bit_string, OBJECT_DEVICE, true);
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_INPUT, true);
            bitstring_set_bit(&bit_string, OBJECT_ANALOG_OUTPUT, true);
            bitstring_set_bit(&bit_string, OBJECT_BINARY_INPUT, true);
            bitstring_set_bit(&bit_string, OBJECT_BINARY_OUTPUT, true);
            apdu_len = encode_application_bitstring(&rpdata->application_data[0],
                &bit_string);
            break;

        case PROP_MAX_APDU_LENGTH_ACCEPTED:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                Device_Max_APDU_Length_Accepted());
            break;

        case PROP_SEGMENTATION_SUPPORTED:
            apdu_len = encode_application_enumerated(&rpdata->application_data[0],
                Device_Segmentation_Supported());
            break;

        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                3000);  /* 3 seconds */
            break;

        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                3);
            break;

        case PROP_DEVICE_ADDRESS_BINDING:
            /* Empty list */
            apdu_len = 0;
            break;

        case PROP_DATABASE_REVISION:
            apdu_len = encode_application_unsigned(&rpdata->application_data[0],
                Device_Database_Revision());
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

/**
 * @brief Write a device object property from the APDU
 */
bool Device_Write_Property_Local(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;

    /* Only allow writes to certain properties */
    switch (wp_data->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            /* Could allow changing instance number here */
            break;

        case PROP_LOCATION:
        case PROP_DESCRIPTION:
            /* Could allow these to be writable */
            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
    }

    return status;
}

/**
 * @brief Property lists for RPM support
 */
void Device_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    static const int Required[] = {
        PROP_OBJECT_IDENTIFIER,
        PROP_OBJECT_NAME,
        PROP_OBJECT_TYPE,
        PROP_SYSTEM_STATUS,
        PROP_VENDOR_NAME,
        PROP_VENDOR_IDENTIFIER,
        PROP_MODEL_NAME,
        PROP_FIRMWARE_REVISION,
        PROP_APPLICATION_SOFTWARE_VERSION,
        PROP_PROTOCOL_VERSION,
        PROP_PROTOCOL_REVISION,
        PROP_PROTOCOL_SERVICES_SUPPORTED,
        PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
        PROP_MAX_APDU_LENGTH_ACCEPTED,
        PROP_SEGMENTATION_SUPPORTED,
        PROP_APDU_TIMEOUT,
        PROP_NUMBER_OF_APDU_RETRIES,
        PROP_DEVICE_ADDRESS_BINDING,
        PROP_DATABASE_REVISION,
        -1
    };

    static const int Optional[] = {
        PROP_LOCATION,
        PROP_DESCRIPTION,
        -1
    };

    if (pRequired) {
        *pRequired = Required;
    }
    if (pOptional) {
        *pOptional = Optional;
    }
    if (pProprietary) {
        *pProprietary = NULL;
    }
}