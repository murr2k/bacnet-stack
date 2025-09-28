/**
 * @file
 * @brief Analog Output object implementation for PIC32MX795F512L
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
#include "bacnet/basic/object/ao.h"
#include "bacnet/basic/services.h"
#include "hardware.h"

/* Number of Analog Output objects */
#ifndef MAX_ANALOG_OUTPUTS
#define MAX_ANALOG_OUTPUTS 4
#endif

/* Priority array size */
#ifndef BACNET_PRIORITY_ARRAY_SIZE
#define BACNET_PRIORITY_ARRAY_SIZE 16
#endif

/* Analog Output object structure */
typedef struct {
    float Present_Value;
    float Relinquish_Default;
    float Min_Present_Value;
    float Max_Present_Value;
    float Priority_Array[BACNET_PRIORITY_ARRAY_SIZE];
    bool Priority_Active[BACNET_PRIORITY_ARRAY_SIZE];
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_CHARACTER_STRING Description;
    uint16_t Units;
    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    float COV_Increment;
    bool Changed;
} ANALOG_OUTPUT_DATA;

/* Analog Output objects database */
static ANALOG_OUTPUT_DATA AO_Data[MAX_ANALOG_OUTPUTS];

/**
 * @brief Initialize analog output objects
 */
void Analog_Output_Init(void)
{
    unsigned i, j;
    char name[32];

    for (i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        AO_Data[i].Present_Value = 0.0;
        AO_Data[i].Relinquish_Default = 0.0;
        AO_Data[i].Min_Present_Value = 0.0;
        AO_Data[i].Max_Present_Value = 100.0;
        AO_Data[i].Units = UNITS_PERCENT;
        AO_Data[i].Out_Of_Service = false;
        AO_Data[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
        AO_Data[i].COV_Increment = 1.0;
        AO_Data[i].Changed = false;

        /* Initialize priority array */
        for (j = 0; j < BACNET_PRIORITY_ARRAY_SIZE; j++) {
            AO_Data[i].Priority_Array[j] = 0.0;
            AO_Data[i].Priority_Active[j] = false;
        }

        sprintf(name, "AO-%u", i);
        characterstring_init_ansi(&AO_Data[i].Object_Name, name);
        characterstring_init_ansi(&AO_Data[i].Description, "Analog Output");
    }
}

/**
 * @brief Check if analog output object is valid
 */
bool Analog_Output_Valid_Instance(uint32_t object_instance)
{
    return (object_instance < MAX_ANALOG_OUTPUTS);
}

/**
 * @brief Get number of analog output objects
 */
unsigned Analog_Output_Count(void)
{
    return MAX_ANALOG_OUTPUTS;
}

/**
 * @brief Get analog output object instance by index
 */
uint32_t Analog_Output_Index_To_Instance(unsigned index)
{
    return index;
}

/**
 * @brief Get analog output object index by instance
 */
unsigned Analog_Output_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_ANALOG_OUTPUTS;

    if (object_instance < MAX_ANALOG_OUTPUTS) {
        index = object_instance;
    }

    return index;
}

/**
 * @brief Calculate present value from priority array
 */
static void Analog_Output_Present_Value_Calculate(unsigned index)
{
    unsigned priority;
    bool found = false;

    if (index < MAX_ANALOG_OUTPUTS) {
        /* Find highest priority active value */
        for (priority = 0; priority < BACNET_PRIORITY_ARRAY_SIZE; priority++) {
            if (AO_Data[index].Priority_Active[priority]) {
                AO_Data[index].Present_Value =
                    AO_Data[index].Priority_Array[priority];
                found = true;
                break;
            }
        }

        /* Use relinquish default if no priority is active */
        if (!found) {
            AO_Data[index].Present_Value = AO_Data[index].Relinquish_Default;
        }
    }
}

/**
 * @brief Get analog output object name
 */
bool Analog_Output_Object_Name(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        status = characterstring_copy(object_name, &AO_Data[index].Object_Name);
    }

    return status;
}

/**
 * @brief Set analog output object name
 */
bool Analog_Output_Object_Name_Set(
    uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        status = characterstring_copy(&AO_Data[index].Object_Name, object_name);
    }

    return status;
}

/**
 * @brief Get analog output present value
 */
float Analog_Output_Present_Value(uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        value = AO_Data[index].Present_Value;
    }

    return value;
}

/**
 * @brief Set analog output present value
 */
bool Analog_Output_Present_Value_Set(
    uint32_t object_instance, float value, unsigned priority)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        /* Check limits */
        if (value >= AO_Data[index].Min_Present_Value &&
            value <= AO_Data[index].Max_Present_Value) {
            if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
                /* Set priority value */
                priority--; /* Convert to 0-based index */
                AO_Data[index].Priority_Array[priority] = value;
                AO_Data[index].Priority_Active[priority] = true;

                /* Calculate new present value */
                float prior_value = AO_Data[index].Present_Value;
                Analog_Output_Present_Value_Calculate(index);

                /* Check for COV */
                if (fabs(prior_value - AO_Data[index].Present_Value) >=
                    AO_Data[index].COV_Increment) {
                    AO_Data[index].Changed = true;
                }
                status = true;
            } else if (AO_Data[index].Out_Of_Service) {
                /* Direct write when out of service */
                float prior_value = AO_Data[index].Present_Value;
                AO_Data[index].Present_Value = value;

                if (fabs(prior_value - value) >= AO_Data[index].COV_Increment) {
                    AO_Data[index].Changed = true;
                }
                status = true;
            }
        }
    }

    return status;
}

/**
 * @brief Relinquish analog output present value
 */
bool Analog_Output_Present_Value_Relinquish(
    uint32_t object_instance, unsigned priority)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
            priority--; /* Convert to 0-based index */
            AO_Data[index].Priority_Active[priority] = false;

            /* Calculate new present value */
            float prior_value = AO_Data[index].Present_Value;
            Analog_Output_Present_Value_Calculate(index);

            /* Check for COV */
            if (fabs(prior_value - AO_Data[index].Present_Value) >=
                AO_Data[index].COV_Increment) {
                AO_Data[index].Changed = true;
            }
            status = true;
        }
    }

    return status;
}

/**
 * @brief Get analog output units
 */
uint16_t Analog_Output_Units(uint32_t object_instance)
{
    uint16_t units = UNITS_NO_UNITS;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        units = AO_Data[index].Units;
    }

    return units;
}

/**
 * @brief Set analog output units
 */
bool Analog_Output_Units_Set(uint32_t object_instance, uint16_t units)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        AO_Data[index].Units = units;
        status = true;
    }

    return status;
}

/**
 * @brief Get analog output out-of-service status
 */
bool Analog_Output_Out_Of_Service(uint32_t object_instance)
{
    bool oos = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        oos = AO_Data[index].Out_Of_Service;
    }

    return oos;
}

/**
 * @brief Set analog output out-of-service status
 */
void Analog_Output_Out_Of_Service_Set(uint32_t object_instance, bool oos_flag)
{
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        AO_Data[index].Out_Of_Service = oos_flag;
    }
}

/**
 * @brief Get analog output relinquish default
 */
float Analog_Output_Relinquish_Default(uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        value = AO_Data[index].Relinquish_Default;
    }

    return value;
}

/**
 * @brief Set analog output relinquish default
 */
bool Analog_Output_Relinquish_Default_Set(uint32_t object_instance, float value)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        AO_Data[index].Relinquish_Default = value;
        status = true;
    }

    return status;
}

/**
 * @brief Get analog output priority array value
 */
float Analog_Output_Priority_Array_Value(
    uint32_t object_instance, unsigned priority)
{
    float value = 0.0;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
            priority--; /* Convert to 0-based index */
            if (AO_Data[index].Priority_Active[priority]) {
                value = AO_Data[index].Priority_Array[priority];
            }
        }
    }

    return value;
}

/**
 * @brief Get analog output description
 */
const char *Analog_Output_Description(uint32_t object_instance)
{
    unsigned index = Analog_Output_Instance_To_Index(object_instance);
    const char *description = NULL;

    if (index < MAX_ANALOG_OUTPUTS) {
        description = characterstring_value(&AO_Data[index].Description);
    }

    return description;
}

/**
 * @brief Set analog output description
 */
bool Analog_Output_Description_Set(
    uint32_t object_instance, const char *description)
{
    bool status = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS && description) {
        status =
            characterstring_init_ansi(&AO_Data[index].Description, description);
    }

    return status;
}

/**
 * @brief Check if analog output value changed
 */
bool Analog_Output_Change_Of_Value(uint32_t object_instance)
{
    bool changed = false;
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        changed = AO_Data[index].Changed;
    }

    return changed;
}

/**
 * @brief Clear analog output value changed flag
 */
void Analog_Output_Change_Of_Value_Clear(uint32_t object_instance)
{
    unsigned index = Analog_Output_Instance_To_Index(object_instance);

    if (index < MAX_ANALOG_OUTPUTS) {
        AO_Data[index].Changed = false;
    }
}
