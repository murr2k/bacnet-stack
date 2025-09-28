/**
 * @file
 * @brief Binary Output object implementation for PIC32MX795F512L
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
#include "bacnet/basic/object/bo.h"
#include "bacnet/basic/services.h"
#include "hardware.h"

/* Number of Binary Output objects */
#ifndef MAX_BINARY_OUTPUTS
#define MAX_BINARY_OUTPUTS 4
#endif

/* Priority array size */
#ifndef BACNET_PRIORITY_ARRAY_SIZE
#define BACNET_PRIORITY_ARRAY_SIZE 16
#endif

/* Binary Output object structure */
typedef struct {
    BACNET_BINARY_PV Present_Value;
    BACNET_BINARY_PV Relinquish_Default;
    BACNET_BINARY_PV Priority_Array[BACNET_PRIORITY_ARRAY_SIZE];
    bool Priority_Active[BACNET_PRIORITY_ARRAY_SIZE];
    BACNET_CHARACTER_STRING Object_Name;
    BACNET_CHARACTER_STRING Description;
    BACNET_CHARACTER_STRING Active_Text;
    BACNET_CHARACTER_STRING Inactive_Text;
    bool Out_Of_Service;
    BACNET_RELIABILITY Reliability;
    BACNET_POLARITY Polarity;
    bool Changed;
    uint32_t Minimum_Off_Time_ms;
    uint32_t Minimum_On_Time_ms;
    uint32_t Time_Of_State_Count_Change_ms;
    BACNET_BINARY_PV Last_State;
} BINARY_OUTPUT_DATA;

/* Binary Output objects database */
static BINARY_OUTPUT_DATA BO_Data[MAX_BINARY_OUTPUTS];

/* External millisecond counter from main.c */
extern volatile uint32_t millisecond_counter;

/**
 * @brief Initialize binary output objects
 */
void Binary_Output_Init(void)
{
    unsigned i, j;
    char name[32];

    for (i = 0; i < MAX_BINARY_OUTPUTS; i++) {
        BO_Data[i].Present_Value = BINARY_INACTIVE;
        BO_Data[i].Relinquish_Default = BINARY_INACTIVE;
        BO_Data[i].Out_Of_Service = false;
        BO_Data[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
        BO_Data[i].Polarity = POLARITY_NORMAL;
        BO_Data[i].Changed = false;
        BO_Data[i].Minimum_Off_Time_ms = 0;
        BO_Data[i].Minimum_On_Time_ms = 0;
        BO_Data[i].Time_Of_State_Count_Change_ms = millisecond_counter;
        BO_Data[i].Last_State = BINARY_INACTIVE;

        /* Initialize priority array */
        for (j = 0; j < BACNET_PRIORITY_ARRAY_SIZE; j++) {
            BO_Data[i].Priority_Array[j] = BINARY_INACTIVE;
            BO_Data[i].Priority_Active[j] = false;
        }

        sprintf(name, "BO-%u", i);
        characterstring_init_ansi(&BO_Data[i].Object_Name, name);
        characterstring_init_ansi(&BO_Data[i].Description, "Binary Output");
        characterstring_init_ansi(&BO_Data[i].Active_Text, "On");
        characterstring_init_ansi(&BO_Data[i].Inactive_Text, "Off");
    }
}

/**
 * @brief Calculate present value from priority array
 */
static void Binary_Output_Present_Value_Calculate(unsigned index)
{
    unsigned priority;
    bool found = false;

    if (index < MAX_BINARY_OUTPUTS) {
        /* Find highest priority active value */
        for (priority = 0; priority < BACNET_PRIORITY_ARRAY_SIZE; priority++) {
            if (BO_Data[index].Priority_Active[priority]) {
                BACNET_BINARY_PV new_value = BO_Data[index].Priority_Array[priority];

                /* Check minimum on/off time */
                uint32_t time_in_state = millisecond_counter - BO_Data[index].Time_Of_State_Count_Change_ms;

                if (BO_Data[index].Last_State == BINARY_ACTIVE &&
                    new_value == BINARY_INACTIVE &&
                    time_in_state < BO_Data[index].Minimum_On_Time_ms) {
                    /* Minimum on time not met */
                    return;
                }

                if (BO_Data[index].Last_State == BINARY_INACTIVE &&
                    new_value == BINARY_ACTIVE &&
                    time_in_state < BO_Data[index].Minimum_Off_Time_ms) {
                    /* Minimum off time not met */
                    return;
                }

                /* Update state */
                if (BO_Data[index].Present_Value != new_value) {
                    BO_Data[index].Last_State = BO_Data[index].Present_Value;
                    BO_Data[index].Present_Value = new_value;
                    BO_Data[index].Time_Of_State_Count_Change_ms = millisecond_counter;
                }
                found = true;
                break;
            }
        }

        /* Use relinquish default if no priority is active */
        if (!found) {
            if (BO_Data[index].Present_Value != BO_Data[index].Relinquish_Default) {
                BO_Data[index].Last_State = BO_Data[index].Present_Value;
                BO_Data[index].Present_Value = BO_Data[index].Relinquish_Default;
                BO_Data[index].Time_Of_State_Count_Change_ms = millisecond_counter;
            }
        }
    }
}

/**
 * @brief Check if binary output object is valid
 */
bool Binary_Output_Valid_Instance(uint32_t object_instance)
{
    return (object_instance < MAX_BINARY_OUTPUTS);
}

/**
 * @brief Get number of binary output objects
 */
unsigned Binary_Output_Count(void)
{
    return MAX_BINARY_OUTPUTS;
}

/**
 * @brief Get binary output object instance by index
 */
uint32_t Binary_Output_Index_To_Instance(unsigned index)
{
    return index;
}

/**
 * @brief Get binary output object index by instance
 */
unsigned Binary_Output_Instance_To_Index(uint32_t object_instance)
{
    unsigned index = MAX_BINARY_OUTPUTS;

    if (object_instance < MAX_BINARY_OUTPUTS) {
        index = object_instance;
    }

    return index;
}

/**
 * @brief Get binary output object name
 */
bool Binary_Output_Object_Name(uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        status = characterstring_copy(object_name, &BO_Data[index].Object_Name);
    }

    return status;
}

/**
 * @brief Set binary output object name
 */
bool Binary_Output_Object_Name_Set(uint32_t object_instance, BACNET_CHARACTER_STRING *object_name)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        status = characterstring_copy(&BO_Data[index].Object_Name, object_name);
    }

    return status;
}

/**
 * @brief Get binary output present value
 */
BACNET_BINARY_PV Binary_Output_Present_Value(uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        value = BO_Data[index].Present_Value;
    }

    return value;
}

/**
 * @brief Set binary output present value
 */
bool Binary_Output_Present_Value_Set(uint32_t object_instance, BACNET_BINARY_PV value, unsigned priority)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
            /* Set priority value */
            priority--;  /* Convert to 0-based index */
            BO_Data[index].Priority_Array[priority] = value;
            BO_Data[index].Priority_Active[priority] = true;

            /* Calculate new present value */
            BACNET_BINARY_PV prior_value = BO_Data[index].Present_Value;
            Binary_Output_Present_Value_Calculate(index);

            /* Check for COV */
            if (prior_value != BO_Data[index].Present_Value) {
                BO_Data[index].Changed = true;
            }
            status = true;
        } else if (BO_Data[index].Out_Of_Service) {
            /* Direct write when out of service */
            if (BO_Data[index].Present_Value != value) {
                BO_Data[index].Changed = true;
                BO_Data[index].Last_State = BO_Data[index].Present_Value;
                BO_Data[index].Present_Value = value;
                BO_Data[index].Time_Of_State_Count_Change_ms = millisecond_counter;
            }
            status = true;
        }
    }

    return status;
}

/**
 * @brief Relinquish binary output present value
 */
bool Binary_Output_Present_Value_Relinquish(uint32_t object_instance, unsigned priority)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
            priority--;  /* Convert to 0-based index */
            BO_Data[index].Priority_Active[priority] = false;

            /* Calculate new present value */
            BACNET_BINARY_PV prior_value = BO_Data[index].Present_Value;
            Binary_Output_Present_Value_Calculate(index);

            /* Check for COV */
            if (prior_value != BO_Data[index].Present_Value) {
                BO_Data[index].Changed = true;
            }
            status = true;
        }
    }

    return status;
}

/**
 * @brief Get binary output out-of-service status
 */
bool Binary_Output_Out_Of_Service(uint32_t object_instance)
{
    bool oos = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        oos = BO_Data[index].Out_Of_Service;
    }

    return oos;
}

/**
 * @brief Set binary output out-of-service status
 */
void Binary_Output_Out_Of_Service_Set(uint32_t object_instance, bool oos_flag)
{
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        BO_Data[index].Out_Of_Service = oos_flag;
    }
}

/**
 * @brief Get binary output polarity
 */
BACNET_POLARITY Binary_Output_Polarity(uint32_t object_instance)
{
    BACNET_POLARITY polarity = POLARITY_NORMAL;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        polarity = BO_Data[index].Polarity;
    }

    return polarity;
}

/**
 * @brief Set binary output polarity
 */
bool Binary_Output_Polarity_Set(uint32_t object_instance, BACNET_POLARITY polarity)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        BO_Data[index].Polarity = polarity;
        status = true;
    }

    return status;
}

/**
 * @brief Get binary output relinquish default
 */
BACNET_BINARY_PV Binary_Output_Relinquish_Default(uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        value = BO_Data[index].Relinquish_Default;
    }

    return value;
}

/**
 * @brief Set binary output relinquish default
 */
bool Binary_Output_Relinquish_Default_Set(uint32_t object_instance, BACNET_BINARY_PV value)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        BO_Data[index].Relinquish_Default = value;
        status = true;
    }

    return status;
}

/**
 * @brief Get binary output priority array value
 */
BACNET_BINARY_PV Binary_Output_Priority_Array_Value(uint32_t object_instance, unsigned priority)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        if (priority && (priority <= BACNET_PRIORITY_ARRAY_SIZE)) {
            priority--;  /* Convert to 0-based index */
            if (BO_Data[index].Priority_Active[priority]) {
                value = BO_Data[index].Priority_Array[priority];
            }
        }
    }

    return value;
}

/**
 * @brief Check if binary output value changed
 */
bool Binary_Output_Change_Of_Value(uint32_t object_instance)
{
    bool changed = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        changed = BO_Data[index].Changed;
    }

    return changed;
}

/**
 * @brief Clear binary output value changed flag
 */
void Binary_Output_Change_Of_Value_Clear(uint32_t object_instance)
{
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS) {
        BO_Data[index].Changed = false;
    }
}

/**
 * @brief Get binary output active text
 */
const char *Binary_Output_Active_Text(uint32_t object_instance)
{
    unsigned index = Binary_Output_Instance_To_Index(object_instance);
    const char *text = NULL;

    if (index < MAX_BINARY_OUTPUTS) {
        text = characterstring_value(&BO_Data[index].Active_Text);
    }

    return text;
}

/**
 * @brief Set binary output active text
 */
bool Binary_Output_Active_Text_Set(uint32_t object_instance, const char *text)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS && text) {
        status = characterstring_init_ansi(&BO_Data[index].Active_Text, text);
    }

    return status;
}

/**
 * @brief Get binary output inactive text
 */
const char *Binary_Output_Inactive_Text(uint32_t object_instance)
{
    unsigned index = Binary_Output_Instance_To_Index(object_instance);
    const char *text = NULL;

    if (index < MAX_BINARY_OUTPUTS) {
        text = characterstring_value(&BO_Data[index].Inactive_Text);
    }

    return text;
}

/**
 * @brief Set binary output inactive text
 */
bool Binary_Output_Inactive_Text_Set(uint32_t object_instance, const char *text)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS && text) {
        status = characterstring_init_ansi(&BO_Data[index].Inactive_Text, text);
    }

    return status;
}

/**
 * @brief Get binary output description
 */
const char *Binary_Output_Description(uint32_t object_instance)
{
    unsigned index = Binary_Output_Instance_To_Index(object_instance);
    const char *description = NULL;

    if (index < MAX_BINARY_OUTPUTS) {
        description = characterstring_value(&BO_Data[index].Description);
    }

    return description;
}

/**
 * @brief Set binary output description
 */
bool Binary_Output_Description_Set(uint32_t object_instance, const char *description)
{
    bool status = false;
    unsigned index = Binary_Output_Instance_To_Index(object_instance);

    if (index < MAX_BINARY_OUTPUTS && description) {
        status = characterstring_init_ansi(&BO_Data[index].Description, description);
    }

    return status;
}