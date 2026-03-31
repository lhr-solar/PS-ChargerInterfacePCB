#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"
#include "CAN_FD.h"

#define BPS_Status_ID        0x1
#define BPS_Aggregate_Arr_ID 0xB

// BPS_Status byte 1 (bits 8-13): only charge_ok is used
#define BPS_STATUS_FLAGS_BYTE_IDX  1
#define BPS_STATUS_CHARGE_OK_MASK  0x01U  // bit 8: BPS_Charge_OK

//predefined fault payload that sets charge_okay to 0
#define BPS_STATUS_FAULT_PAYLOAD { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
//predefined ok payload that sets charge_okay to 1 (bit 8 = byte 1 bit 0)
#define BPS_STATUS_OK_PAYLOAD    { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#define BPS_TAP_COUNT 32

// One entry in the per-tap aggregate array (BO_ 11 BPS_Aggregate_Arr)
typedef struct {
    float voltage;     // V,  scale 0.001
    float temperature; // °C, scale 0.001, signed
} CarCAN_BPS_Tap_t;

// BPS streams one tap per frame — accumulate into taps[] indexed by BPS_Tap_idx
typedef struct {
    CarCAN_BPS_Tap_t taps[BPS_TAP_COUNT];
    uint8_t          last_updated_idx;
} CarCAN_BPS_Aggregate_t;

can_status_t CarCAN_Init(void);

can_status_t CarCAN_Send(uint32_t id, uint8_t data[8], TickType_t delay_ticks);

// Generic receive — populates data[] and sets *id_out to the received message ID
can_status_t CarCAN_Receive(uint32_t *id_out, uint8_t data[8], TickType_t delay_ticks);

// Unpack one BPS_Aggregate_Arr frame into the correct tap slot in agg
void CarCAN_Unpack_BPS_Aggregate(const uint8_t data[8], CarCAN_BPS_Aggregate_t *agg);
