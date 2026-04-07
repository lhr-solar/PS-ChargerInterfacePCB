#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"
#include "CAN_FD.h"
#include "CarCAN_can_msgs.h"

// BPS_Status signal: BPS_Charge_OK
// Signal start bit 8 → byte index 1, bit 0 within that byte.
// Enum values defined in CarCAN_can_msgs.h: bps_status_bps_charge_ok_e
//   BPS_STATUS_BPS_CHARGE_OK_OK    = 1  (charging permitted)
//   BPS_STATUS_BPS_CHARGE_OK_NOT_OK = 0  (charging not permitted)
#define BPS_STATUS_CHARGE_OK_BYTE  1U     // byte index of BPS_Charge_OK in the frame
#define BPS_STATUS_CHARGE_OK_MASK  0x01U  // 1-bit mask at bit 0 of that byte


#define BPS_TAP_COUNT 32


// elcon_comm_ok values for CarCAN_Send_ChargerInterface_Status
#define ELCON_COMM_OK     1U
#define ELCON_COMM_FAULT  0U

// elcon_fault values for CarCAN_Send_ChargerInterface_Status
#define ELCON_NO_FAULT    0U
#define ELCON_FAULT       1U

//TODO: implement BPS aggregate logic
// bps tap data
typedef struct {
    uint16_t voltage_mv;      // in 0.001V (mV) units
    int32_t  temperature_mdc; // in 0.001°C units, signed
} CarCAN_BPS_Tap_t;

//TODO: implement all of this aggregated struct
// BPS streams one tap per frame and all together in this struct
typedef struct {
    CarCAN_BPS_Tap_t taps[BPS_TAP_COUNT];
    uint8_t          last_updated_idx;
} CarCAN_BPS_Aggregate_t;



/**
 * @brief Initializes the CarCAN (FDCAN3) peripheral with fixed 500 kbps classic CAN settings
 *        and an open receive filter that accepts all standard IDs.
 * @return CAN_OK on success, CAN_ERR if initialization or start fails.
 */
can_status_t CarCAN_Init(void);

/**
 * @brief Sends a CAN frame on CarCAN with the given ID, payload, and DLC.
 * @param id           Standard CAN ID to transmit.
 * @param data         Payload buffer to send.
 * @param dlc          Data length code — use FDCAN_DLC_BYTES_x to match the
 *                     message's defined payload size (e.g. FDCAN_DLC_BYTES_5).
 * @param delay_ticks  FreeRTOS tick timeout to wait for a free TX mailbox.
 * @return CAN_OK on successful transmission, CAN_ERR on failure.
 */
can_status_t CarCAN_Send(uint32_t id, uint8_t data[8], uint32_t dlc, TickType_t delay_ticks);

/**
 * @brief Polls the CarCAN RX FIFO for a BPS_Status or BPS_Voltage_Aggregate_Arr frame.
 *        Checks BPS_Status (ID 0x1) first (non-blocking), then BPS_Voltage_Aggregate_Arr
 *        (ID 0xB) with the provided timeout.
 * @param id_out       Set to the ID of the received frame on CAN_OK.
 * @param data         8-byte buffer to write the received payload into.
 * @param delay_ticks  FreeRTOS tick timeout applied to the second ID check.
 * @return CAN_OK if a frame was received, CAN_EMPTY if neither ID had data.
 */
can_status_t CarCAN_Receive(uint32_t *id_out, uint8_t data[8], TickType_t delay_ticks);

/**
 * @brief Unpacks a BPS_Voltage_Aggregate_Arr frame (ID 0xB) into the aggregate struct.
 *        Extracts the tap index (bits 0-4 of byte 0), voltage (bytes 1-2, scale 0.001 V),
 *        and temperature (bytes 3-6, scale 0.001 °C, signed) and updates the corresponding
 *        tap slot and last_updated_idx.
 * @param data  8-byte received payload.
 * @param agg   Aggregate struct to update in place.
 */
void CarCAN_Unpack_BPS_Aggregate(const uint8_t data[8], CarCAN_BPS_Aggregate_t *agg);


/**
 * @brief Packs and sends the charger interface status frame (ID 0xE) onto CarCAN.
 *        Encodes output voltage and current as uint16 little-endian (scale 0.1),
 *        and packs elcon_comm_ok into byte 4 bit 0, elcon_fault into byte 4 bit 1.
 * @param output_voltage_dv  Actual charger output voltage in 0.1V units (from ElconStatus_t)
 * @param output_current_da  Actual charger output current in 0.1A units (from ElconStatus_t)
 * @param elcon_comm_ok   1 if Elcon is communicating, 0 if timed out
 * @param elcon_fault     1 if any Elcon fault is active, 0 if clear
 * @param delay_ticks     FreeRTOS tick timeout to wait for a free TX mailbox
 * @return CAN_OK on successful transmission, CAN_ERR on failure
 */
can_status_t CarCAN_Send_ChargerInterface_Status(uint16_t output_voltage_dv, uint16_t output_current_da, uint8_t elcon_comm_ok, uint8_t elcon_fault, TickType_t delay_ticks);
