/**
 * @file
 * @brief Binary Input object implementation for PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacstr.h"
#include "bacnet/config.h"
#include "bacnet/basic/object/bi.h"
#include "bacnet/basic/services.h"
#include "hardware.h"

/* Number of Binary Input objects */
#ifndef MAX_BINARY_INPUTS
#define MAX_BINARY_INPUTS 8
#endif

/* Binary Input object structure */
typedef struct {
    BACNET_BINARY_PV Present_Value;
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_CHARACTER_STRING Description;
    BACNET_CHARACTER_STRING Active_Text;
    BACNET_CHARACTER_STRING Inactive_Text;
    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_POLARITY Polarity;
    bool Changed;
    bool Present_Value_writable;
} BINARY_INPUT_DATA;

/* Binary Input objects database */
static BINARY_INPUT_DATA BI_Data[MAX_BINARY_INPUTS];

/**
 * @brief Initialize binary input objects
 */
void Binary_Input_Init(void)
{
    unsigned i;
    char name[32];

    for (i = 0; i < MAX_BINARY_INPUTS; i++) {
        BI_Data[i].Present_Value = BINARY_INACTIVE;
        BI_Data[i].Out_Of_Service = false;
        BI_Data[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
        BI_Data[i].Polarity = POLARITY_NORMAL;
        BI_Data[i].Changed = false;
        BI_Data[i].Present_Value_writable = false;

        sprintf(name, "BI-%u", i);
        characterstring_init_ansi(&BI_Data[i].Object_Name, name);
        characterstring_init_ansi(&BI_Data[i].Description, "Binary Input");
        characterstring_init_ansi(&BI_Data[i].Active_Text, "Active");
        characterstring_init_ansi(&BI_Data[i].Inactive_Text, "Inactive");
    }
}

/**
 * @brief Check if binary input object is valid
 */
bool Binary_Input_Valid_Instance(uint32_t object_instance)
{
    return (object_instance < MAX_BINARY_INPUTS);
}

/**
 * @brief Get number of binary input objects
 */
unsigned Binary_Input_Count(void)
{
    return MAX_BINARY_INPUTS;
}

/**
 * @brief Get binary input object instance by index
 */
uint32_t Binary_Input_Index_To_Instance(unsigned index)
{
    return index;
}

/**
 * @brief Get binary input object index by instance
 */
unsigned Binary_Input_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_BINARY_INPUTS;

    if (object_instance < MAX_BINARY_INPUTS) {
        index = object_instance;
    }

    return index;
}

/**
 * @brief Get binary input object name
 */
bool Binary_Input_Object_Name(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        status = characterstring_copy(object_name, &BI_Data[index].Object_Name);
    }

    return status;
}

/**
 * @brief Set binary input object name
 */
bool Binary_Input_Object_Name_Set(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        status = characterstring_copy(&BI_Data[index].Object_Name, object_name);
    }

    return status;
}

/**
 * @brief Get binary input present value
 */
BACNET_BINARY_PV Binary_Input_Present_Value(uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        value = BI_Data[index].Present_Value;
    }

    return value;
}

/**
 * @brief Set binary input present value
 */
bool Binary_Input_Present_Value_Set(
    uint32_t object_instance, BACNET_BINARY_PV value)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        if (BI_Data[index].Out_Of_Service ||
            BI_Data[index].Present_Value_writable) {
            /* Check for COV */
            if (BI_Data[index].Present_Value != value) {
                BI_Data[index].Changed = true;
            }
            BI_Data[index].Present_Value = value;
            status = true;
        }
    }

    return status;
}

/**
 * @brief Get binary input out-of-service status
 */
bool Binary_Input_Out_Of_Service(uint32_t object_instance)
{
    bool oos = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        oos = BI_Data[index].Out_Of_Service;
    }

    return oos;
}

/**
 * @brief Set binary input out-of-service status
 */
void Binary_Input_Out_Of_Service_Set(uint32_t object_instance, bool oos_flag)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        BI_Data[index].Out_Of_Service = oos_flag;
    }
}

/**
 * @brief Get binary input reliability
 */
BACNET_RELIABILITY Binary_Input_Reliability(uint32_t object_instance)
{
    BACNET_RELIABILITY reliability = RELIABILITY_NO_FAULT_DETECTED;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        reliability = BI_Data[index].Reliability;
    }

    return reliability;
}

/**
 * @brief Set binary input reliability
 */
bool Binary_Input_Reliability_Set(
    uint32_t object_instance, BACNET_RELIABILITY reliability)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);
    bool status = false;

    if (index < MAX_BINARY_INPUTS) {
        BI_Data[index].Reliability = reliability;
        status = true;
    }

    return status;
}

/**
 * @brief Get binary input polarity
 */
BACNET_POLARITY Binary_Input_Polarity(uint32_t object_instance)
{
    BACNET_POLARITY polarity = POLARITY_NORMAL;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        polarity = BI_Data[index].Polarity;
    }

    return polarity;
}

/**
 * @brief Set binary input polarity
 */
bool Binary_Input_Polarity_Set(
    uint32_t object_instance, BACNET_POLARITY polarity)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        BI_Data[index].Polarity = polarity;
        status = true;
    }

    return status;
}

/**
 * @brief Check if binary input value changed
 */
bool Binary_Input_Change_Of_Value(uint32_t object_instance)
{
    bool changed = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        changed = BI_Data[index].Changed;
    }

    return changed;
}

/**
 * @brief Clear binary input value changed flag
 */
void Binary_Input_Change_Of_Value_Clear(uint32_t object_instance)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS) {
        BI_Data[index].Changed = false;
    }
}

/**
 * @brief Get binary input active text
 */
const char *Binary_Input_Active_Text(uint32_t object_instance)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);
    const char *text = NULL;

    if (index < MAX_BINARY_INPUTS) {
        text = characterstring_value(&BI_Data[index].Active_Text);
    }

    return text;
}

/**
 * @brief Set binary input active text
 */
bool Binary_Input_Active_Text_Set(
    uint32_t object_instance, const char *active_text)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS && active_text) {
        status =
            characterstring_init_ansi(&BI_Data[index].Active_Text, active_text);
    }

    return status;
}

/**
 * @brief Get binary input inactive text
 */
const char *Binary_Input_Inactive_Text(uint32_t object_instance)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);
    const char *text = NULL;

    if (index < MAX_BINARY_INPUTS) {
        text = characterstring_value(&BI_Data[index].Inactive_Text);
    }

    return text;
}

/**
 * @brief Set binary input inactive text
 */
bool Binary_Input_Inactive_Text_Set(
    uint32_t object_instance, const char *inactive_text)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS && inactive_text) {
        status = characterstring_init_ansi(
            &BI_Data[index].Inactive_Text, inactive_text);
    }

    return status;
}

/**
 * @brief Get binary input description
 */
const char *Binary_Input_Description(uint32_t object_instance)
{
    unsigned index = Binary_Input_Instance_To_Index(object_instance);
    const char *description = NULL;

    if (index < MAX_BINARY_INPUTS) {
        description = characterstring_value(&BI_Data[index].Description);
    }

    return description;
}

/**
 * @brief Set binary input description
 */
bool Binary_Input_Description_Set(
    uint32_t object_instance, const char *description)
{
    bool status = false;
    unsigned index = Binary_Input_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_INPUTS && description) {
        status =
            characterstring_init_ansi(&BI_Data[index].Description, description);
    }

    return status;
}
