/**
 * @file
 * @brief Analog Input object implementation for PIC32MX795F512L
 * @author Murray Kopit <murr2k@gmail.com>
 * @date 2025
 * @copyright SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bacnet/bacdef.h"
#include "bacnet/bacdcode.h"
#include "bacnet/bacstr.h"
#include "bacnet/config.h"
#include "bacnet/basic/object/ai.h"
#include "bacnet/basic/services.h"
#include "hardware.h"

/* Number of Analog Input objects */
#ifndef MAX_ANALOG_INPUTS
#define MAX_ANALOG_INPUTS 8
#endif

/* Analog Input object structure */
typedef struct {
    bool Present_Value_writable;
    float Present_Value;
    float Min_Present_Value;
    float Max_Present_Value;
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_CHARACTER_STRING Description;
    uint16_t Units;
    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    float COV_Increment;
    bool Changed;
} ANALOG_INPUT_DATA;

/* Analog Input objects database */
static ANALOG_INPUT_DATA AI_Data[MAX_ANALOG_INPUTS];

/**
 * @brief Initialize analog input objects
 */
void Analog_Input_Init(void)
{
    unsigned i;
    char name[32];

    for (i = 0; i < MAX_ANALOG_INPUTS; i++) {
        AI_Data[i].Present_Value = 0.0;
        AI_Data[i].Min_Present_Value = -100000.0;
        AI_Data[i].Max_Present_Value = 100000.0;
        AI_Data[i].Units = UNITS_DEGREES_CELSIUS;
        AI_Data[i].Out_Of_Service = false;
        AI_Data[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
        AI_Data[i].COV_Increment = 1.0;
        AI_Data[i].Changed = false;
        AI_Data[i].Present_Value_writable = false;

        sprintf(name, "AI-%u", i);
        characterstring_init_ansi(&AI_Data[i].Object_Name, name);
        characterstring_init_ansi(&AI_Data[i].Description, "Analog Input");
    }
}

/**
 * @brief Check if analog input object is valid
 */
bool Analog_Input_Valid_Instance(uint32_t object_instance)
{
    return (object_instance < MAX_ANALOG_INPUTS);
}

/**
 * @brief Get number of analog input objects
 */
unsigned Analog_Input_Count(void)
{
    return MAX_ANALOG_INPUTS;
}

/**
 * @brief Get analog input object instance by index
 */
uint32_t Analog_Input_Index_To_Instance(unsigned index)
{
    return index;
}

/**
 * @brief Get analog input object index by instance
 */
unsigned Analog_Input_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_ANALOG_INPUTS;

    if (object_instance < MAX_ANALOG_INPUTS) {
        index = object_instance;
    }

    return index;
}

/**
 * @brief Get analog input object name
 */
bool Analog_Input_Object_Name(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        status = characterstring_copy(object_name, &AI_Data[index].Object_Name);
    }

    return status;
}

/**
 * @brief Set analog input object name
 */
bool Analog_Input_Object_Name_Set(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        status = characterstring_copy(&AI_Data[index].Object_Name, object_name);
    }

    return status;
}

/**
 * @brief Get analog input present value
 */
float Analog_Input_Present_Value(uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        value = AI_Data[index].Present_Value;
    }

    return value;
}

/**
 * @brief Set analog input present value
 */
void Analog_Input_Present_Value_Set(uint32_t object_instance, float value)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        if (AI_Data[index].Out_Of_Service ||
            AI_Data[index].Present_Value_writable) {
            /* Check limits */
            if (value >= AI_Data[index].Min_Present_Value &&
                value <= AI_Data[index].Max_Present_Value) {
                /* Check for COV */
                float prior_value = AI_Data[index].Present_Value;
                AI_Data[index].Present_Value = value;

                if (fabs(prior_value - value) >= AI_Data[index].COV_Increment) {
                    AI_Data[index].Changed = true;
                }
            }
        }
    }
}

/**
 * @brief Get analog input units
 */
uint16_t Analog_Input_Units(uint32_t object_instance)
{
    uint16_t units = UNITS_NO_UNITS;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        units = AI_Data[index].Units;
    }

    return units;
}

/**
 * @brief Set analog input units
 */
bool Analog_Input_Units_Set(uint32_t object_instance, uint16_t units)
{
    bool status = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        AI_Data[index].Units = units;
        status = true;
    }

    return status;
}

/**
 * @brief Get analog input out-of-service status
 */
bool Analog_Input_Out_Of_Service(uint32_t object_instance)
{
    bool oos = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        oos = AI_Data[index].Out_Of_Service;
    }

    return oos;
}

/**
 * @brief Set analog input out-of-service status
 */
void Analog_Input_Out_Of_Service_Set(uint32_t object_instance, bool oos_flag)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        AI_Data[index].Out_Of_Service = oos_flag;
    }
}

/**
 * @brief Get analog input reliability
 */
BACNET_RELIABILITY Analog_Input_Reliability(uint32_t object_instance)
{
    BACNET_RELIABILITY reliability = RELIABILITY_NO_FAULT_DETECTED;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        reliability = AI_Data[index].Reliability;
    }

    return reliability;
}

/**
 * @brief Set analog input reliability
 */
bool Analog_Input_Reliability_Set(
    uint32_t object_instance, BACNET_RELIABILITY reliability)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);
    bool status = false;

    if (index < MAX_ANALOG_INPUTS) {
        AI_Data[index].Reliability = reliability;
        status = true;
    }

    return status;
}

/**
 * @brief Get analog input COV increment
 */
float Analog_Input_COV_Increment(uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        value = AI_Data[index].COV_Increment;
    }

    return value;
}

/**
 * @brief Set analog input COV increment
 */
void Analog_Input_COV_Increment_Set(uint32_t object_instance, float value)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        AI_Data[index].COV_Increment = value;
    }
}

/**
 * @brief Check if analog input value changed
 */
bool Analog_Input_Change_Of_Value(uint32_t object_instance)
{
    bool changed = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        changed = AI_Data[index].Changed;
    }

    return changed;
}

/**
 * @brief Clear analog input value changed flag
 */
void Analog_Input_Change_Of_Value_Clear(uint32_t object_instance)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS) {
        AI_Data[index].Changed = false;
    }
}

/**
 * @brief Get analog input description
 */
const char *Analog_Input_Description(uint32_t object_instance)
{
    unsigned index = Analog_Input_Instance_To_Index(object_instance);
    const char *description = NULL;

    if (index < MAX_ANALOG_INPUTS) {
        description = characterstring_value(&AI_Data[index].Description);
    }

    return description;
}

/**
 * @brief Set analog input description
 */
bool Analog_Input_Description_Set(
    uint32_t object_instance, const char *description)
{
    bool status = false;
    unsigned index = Analog_Input_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_INPUTS && description) {
        status =
            characterstring_init_ansi(&AI_Data[index].Description, description);
    }

    return status;
}
