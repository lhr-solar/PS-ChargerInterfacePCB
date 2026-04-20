
/*
Elcon CAN bus communication specs: https://utexas.sharepoint.com/:b:/r/sites/ENGR-LonghornRacing/LHR%20Solar/Design/F24-S26/Electrical/Power%20Systems/COTS%20Datasheets/Elcon%20CAN%20Data%20Sheet.pdf?csf=1&web=1&e=nZA3p0 
*/
#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"
#include "CAN_FD.h"

#define ELCONCAN_MSG_1_ID  0x1806E5F4
#define ELCONCAN_MSG_10_ID 0x1806E6F4
#define ELCONCAN_MSG_11_ID 0x1806E6F4
#define ELCONCAN_MSG_12_ID 0x1806E6F4
#define ELCONCAN_RX_ID  0x18FF50E5


typedef struct {
    uint16_t output_voltage_dv;  // actual output voltage in 0.1V units
    uint16_t output_current_da;  // actual output current in 0.1A units
    uint8_t flag_hw_failure;
    uint8_t flag_over_temp;
    uint8_t flag_input_voltage_wrong;
    uint8_t flag_starting_state;
    uint8_t flag_comm_timeout;
} ElconStatus_t;


/**
 * @brief Initializes the Elcon charger CAN interface on FDCAN1 with a filter
 *        that accepts all incoming extended-ID messages. Configures bit timing
 *        for classic CAN at the appropriate baud rate, then starts the peripheral.
 * @return CAN_OK on success, CAN_ERR if initialization or start fails
 */
can_status_t ElconCAN_Init(void);


/**
 * @brief Transmits a CAN frame with the given extended ID, payload, and DLC.
 *        Toggles the HV LED to reflect TX success or failure.
 * @param id            29-bit extended CAN ID to transmit on
 * @param data          Pointer to the payload buffer
 * @param dlc           Data length code — use FDCAN_DLC_BYTES_x to match the
 *                      message's defined payload size (e.g. FDCAN_DLC_BYTES_5).
 * @param delay_ticks   FreeRTOS tick timeout to wait for a free TX mailbox
 * @return CAN_OK on successful transmission, CAN_ERR if TX fails
 */
can_status_t ElconCAN_Send(uint32_t id, uint8_t data[8], uint32_t dlc, TickType_t delay_ticks);


/**
 * @brief Receives and decodes a status frame from the Elcon charger (CAN ID 0x18FF50E5).
 *        Parses bytes 0–3 for actual output voltage and current (scaled by 0.1),
 *        and byte 4 for status flags (hardware failure, over-temperature, wrong input
 *        voltage, starting state, and communication timeout).
 * @param status        Pointer to an ElconStatus_t struct to populate with decoded values
 * @param id            Expected CAN message ID to filter against
 * @param data          Pointer to an 8-byte buffer to store the raw received payload
 * @param delay_ticks   FreeRTOS tick timeout to wait for an incoming message
 * @return CAN_OK on successful receive and decode, CAN_EMPTY if no message available, CAN_ERR on failure
 */
can_status_t ElconCAN_Receive(ElconStatus_t* status, uint32_t id, uint8_t *data, TickType_t delay_ticks);
