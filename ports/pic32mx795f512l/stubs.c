/**
 * @file
 * @brief Stub functions for BACnet stack on PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/apdu.h"
#include "bacnet/npdu.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/object/bo.h"
#include "bacnet/rpm.h"
#include "bacnet/rp.h"
#include "bacnet/wp.h"
#include "bacnet/datalink/mstp.h"

/* Debug printf disabled for embedded system */
bool debug_printf_disabled(void)
{
    return true;
}

/* MSTP Get Reply stub */
uint16_t MSTP_Get_Reply(struct mstp_port_struct_t *mstp_port, unsigned timeout)
{
    (void)mstp_port;
    (void)timeout;
    return 0;
}

/* NPDU handler - processes incoming network PDUs */
void npdu_handler(BACNET_ADDRESS *src, uint8_t *pdu, uint16_t pdu_len)
{
    /* Process NPDU here - simplified for now */
    (void)src;
    (void)pdu;
    (void)pdu_len;
}

/* Analog Input property handlers */
int Analog_Input_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    uint8_t *apdu = NULL;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    float real_value = 0.0;
    uint32_t object_instance;
    int len = 0;

    if (!rpdata) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    object_instance = rpdata->object_instance;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_ANALOG_INPUT, object_instance);
            break;

        case PROP_OBJECT_NAME:
            Analog_Input_Object_Name(object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_INPUT);
            break;

        case PROP_PRESENT_VALUE:
            real_value = Analog_Input_Present_Value(object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(
                &bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                Analog_Input_Out_Of_Service(object_instance));
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(
                &apdu[0], Analog_Input_Out_Of_Service(object_instance));
            break;

        case PROP_UNITS:
            apdu_len = encode_application_enumerated(
                &apdu[0], Analog_Input_Units(object_instance));
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

bool Analog_Input_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!wp_data) {
        return false;
    }

    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);

    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                Analog_Input_Present_Value_Set(
                    wp_data->object_instance, value.type.Real);
                status = true;
            }
            break;

        case PROP_OUT_OF_SERVICE:
            if (value.tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                Analog_Input_Out_Of_Service_Set(
                    wp_data->object_instance, value.type.Boolean);
                status = true;
            }
            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
    }

    return status;
}

void Analog_Input_Property_Lists(
    const int **pRequired, const int **pOptional, const int **pProprietary)
{
    static const int required[] = { PROP_OBJECT_IDENTIFIER,
                                    PROP_OBJECT_NAME,
                                    PROP_OBJECT_TYPE,
                                    PROP_PRESENT_VALUE,
                                    PROP_STATUS_FLAGS,
                                    PROP_OUT_OF_SERVICE,
                                    PROP_UNITS,
                                    -1 };

    static const int optional[] = { PROP_DESCRIPTION, PROP_RELIABILITY,
                                    PROP_COV_INCREMENT, -1 };

    if (pRequired) {
        *pRequired = required;
    }
    if (pOptional) {
        *pOptional = optional;
    }
    if (pProprietary) {
        *pProprietary = NULL;
    }
}

void Analog_Input_Intrinsic_Reporting(uint32_t object_instance)
{
    (void)object_instance;
}

/* Analog Output property handlers */
int Analog_Output_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    uint8_t *apdu = NULL;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    float real_value = 0.0;
    uint32_t object_instance;

    if (!rpdata) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    object_instance = rpdata->object_instance;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_ANALOG_OUTPUT, object_instance);
            break;

        case PROP_OBJECT_NAME:
            Analog_Output_Object_Name(object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_OUTPUT);
            break;

        case PROP_PRESENT_VALUE:
            real_value = Analog_Output_Present_Value(object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(
                &bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                Analog_Output_Out_Of_Service(object_instance));
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(
                &apdu[0], Analog_Output_Out_Of_Service(object_instance));
            break;

        case PROP_UNITS:
            apdu_len = encode_application_enumerated(
                &apdu[0], Analog_Output_Units(object_instance));
            break;

        case PROP_RELINQUISH_DEFAULT:
            real_value = Analog_Output_Relinquish_Default(object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

bool Analog_Output_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!wp_data) {
        return false;
    }

    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);

    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                status = Analog_Output_Present_Value_Set(
                    wp_data->object_instance, value.type.Real,
                    wp_data->priority);
            }
            break;

        case PROP_OUT_OF_SERVICE:
            if (value.tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                Analog_Output_Out_Of_Service_Set(
                    wp_data->object_instance, value.type.Boolean);
                status = true;
            }
            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
    }

    return status;
}

void Analog_Output_Property_Lists(
    const int **pRequired, const int **pOptional, const int **pProprietary)
{
    static const int required[] = { PROP_OBJECT_IDENTIFIER,
                                    PROP_OBJECT_NAME,
                                    PROP_OBJECT_TYPE,
                                    PROP_PRESENT_VALUE,
                                    PROP_STATUS_FLAGS,
                                    PROP_OUT_OF_SERVICE,
                                    PROP_UNITS,
                                    PROP_PRIORITY_ARRAY,
                                    PROP_RELINQUISH_DEFAULT,
                                    -1 };

    static const int optional[] = { PROP_DESCRIPTION, -1 };

    if (pRequired) {
        *pRequired = required;
    }
    if (pOptional) {
        *pOptional = optional;
    }
    if (pProprietary) {
        *pProprietary = NULL;
    }
}

/* Binary Input property handlers */
int Binary_Input_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    uint8_t *apdu = NULL;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV binary_value;
    uint32_t object_instance;

    if (!rpdata) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    object_instance = rpdata->object_instance;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_BINARY_INPUT, object_instance);
            break;

        case PROP_OBJECT_NAME:
            Binary_Input_Object_Name(object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_INPUT);
            break;

        case PROP_PRESENT_VALUE:
            binary_value = Binary_Input_Present_Value(object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], binary_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(
                &bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                Binary_Input_Out_Of_Service(object_instance));
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(
                &apdu[0], Binary_Input_Out_Of_Service(object_instance));
            break;

        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(
                &apdu[0], Binary_Input_Polarity(object_instance));
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

bool Binary_Input_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!wp_data) {
        return false;
    }

    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);

    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                status = Binary_Input_Present_Value_Set(
                    wp_data->object_instance,
                    (BACNET_BINARY_PV)value.type.Enumerated);
            }
            break;

        case PROP_OUT_OF_SERVICE:
            if (value.tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                Binary_Input_Out_Of_Service_Set(
                    wp_data->object_instance, value.type.Boolean);
                status = true;
            }
            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
    }

    return status;
}

void Binary_Input_Property_Lists(
    const int **pRequired, const int **pOptional, const int **pProprietary)
{
    static const int required[] = { PROP_OBJECT_IDENTIFIER, PROP_OBJECT_NAME,
                                    PROP_OBJECT_TYPE,       PROP_PRESENT_VALUE,
                                    PROP_STATUS_FLAGS,      PROP_OUT_OF_SERVICE,
                                    PROP_POLARITY,          -1 };

    static const int optional[] = { PROP_DESCRIPTION, PROP_RELIABILITY,
                                    PROP_ACTIVE_TEXT, PROP_INACTIVE_TEXT, -1 };

    if (pRequired) {
        *pRequired = required;
    }
    if (pOptional) {
        *pOptional = optional;
    }
    if (pProprietary) {
        *pProprietary = NULL;
    }
}

void Binary_Input_Intrinsic_Reporting(uint32_t object_instance)
{
    (void)object_instance;
}

/* Binary Output property handlers */
int Binary_Output_Read_Property(BACNET_READ_PROPERTY_DATA *rpdata)
{
    int apdu_len = 0;
    uint8_t *apdu = NULL;
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV binary_value;
    uint32_t object_instance;

    if (!rpdata) {
        return BACNET_STATUS_ERROR;
    }

    apdu = rpdata->application_data;
    object_instance = rpdata->object_instance;

    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(
                &apdu[0], OBJECT_BINARY_OUTPUT, object_instance);
            break;

        case PROP_OBJECT_NAME:
            Binary_Output_Object_Name(object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;

        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_OUTPUT);
            break;

        case PROP_PRESENT_VALUE:
            binary_value = Binary_Output_Present_Value(object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], binary_value);
            break;

        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(
                &bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                Binary_Output_Out_Of_Service(object_instance));
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;

        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(
                &apdu[0], Binary_Output_Out_Of_Service(object_instance));
            break;

        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(
                &apdu[0], Binary_Output_Polarity(object_instance));
            break;

        case PROP_RELINQUISH_DEFAULT:
            binary_value = Binary_Output_Relinquish_Default(object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], binary_value);
            break;

        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }

    return apdu_len;
}

bool Binary_Output_Write_Property(BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false;
    int len;
    BACNET_APPLICATION_DATA_VALUE value;

    if (!wp_data) {
        return false;
    }

    len = bacapp_decode_application_data(
        wp_data->application_data, wp_data->application_data_len, &value);

    if (len < 0) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }

    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                status = Binary_Output_Present_Value_Set(
                    wp_data->object_instance,
                    (BACNET_BINARY_PV)value.type.Enumerated, wp_data->priority);
            }
            break;

        case PROP_OUT_OF_SERVICE:
            if (value.tag == BACNET_APPLICATION_TAG_BOOLEAN) {
                Binary_Output_Out_Of_Service_Set(
                    wp_data->object_instance, value.type.Boolean);
                status = true;
            }
            break;

        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
    }

    return status;
}

void Binary_Output_Property_Lists(
    const int **pRequired, const int **pOptional, const int **pProprietary)
{
    static const int required[] = {
        PROP_OBJECT_IDENTIFIER,  PROP_OBJECT_NAME,
        PROP_OBJECT_TYPE,        PROP_PRESENT_VALUE,
        PROP_STATUS_FLAGS,       PROP_OUT_OF_SERVICE,
        PROP_POLARITY,           PROP_PRIORITY_ARRAY,
        PROP_RELINQUISH_DEFAULT, -1
    };

    static const int optional[] = { PROP_DESCRIPTION, PROP_ACTIVE_TEXT,
                                    PROP_INACTIVE_TEXT, -1 };

    if (pRequired) {
        *pRequired = required;
    }
    if (pOptional) {
        *pOptional = optional;
    }
    if (pProprietary) {
        *pProprietary = NULL;
    }
}

void Binary_Output_Intrinsic_Reporting(uint32_t object_instance)
{
    (void)object_instance;
}

/* Global variables stub */
volatile uint32_t millisecond_counter = 0;

/* Missing symbols from datetime.c */
const uint8_t days_per_month[12] = { 31, 28, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31 };
const uint32_t days_since_epoch[16] = { 0,    366,  731,  1096, 1461, 1827,
                                        2192, 2557, 2922, 3288, 3653, 4018,
                                        4383, 4749, 5114, 5479 };
const uint16_t days_of_year_to_month_day[366] = { 0 }; /* Simplified stub */
const char *days_of_week[7] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

bool days_is_leap_year(uint16_t year)
{
    return ((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0));
}

/* MSTP timer stubs */
void mstimer_init(void)
{
    /* Stub */
}

uint32_t mstimer_now(void)
{
    return millisecond_counter;
}

/* More missing symbols */
void Device_Inc_Database_Revision(void)
{
    /* Stub */
}

/* dlmstp_receive - MSTP receive function */
void dlmstp_receive(void)
{
    /* Stub */
}



