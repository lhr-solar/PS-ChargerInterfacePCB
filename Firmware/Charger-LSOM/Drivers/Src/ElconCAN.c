#include <stdint.h>

/*
 * Packs the 8-byte payload for CAN ID 0x1806E5F4
 * target_voltage_v: Desired voltage in Volts (e.g., 320.1)
 * target_current_a: Desired current in Amps (e.g., 58.2)
 * stop_charging: 0 = Open/Charging, 1 = Battery Protection/Stop 
 * payload: Pointer to an 8-byte array to store the result
 */
void ElconCAN_TX(float target_voltage_v, float target_current_a, uint8_t stop_charging, uint8_t *payload) {
    // scale the floats based on 0.1/V and 0.1/A bit
    uint16_t v_scaled = (uint16_t)(target_voltage_v * 10.0f);
    uint16_t c_scaled = (uint16_t)(target_current_a * 10.0f);

    //byte 1 is high (voltage)
    //byte 2 is low (voltage)
    payload[0] = (uint8_t)(v_scaled >> 8);   
    payload[1] = (uint8_t)(v_scaled & 0xFF); 

    // byte 3 is high (current)
    // byte 4 is low (current)
    payload[2] = (uint8_t)(c_scaled >> 8);
    payload[3] = (uint8_t)(c_scaled & 0xFF);

    // byte 5 = control flag that is 0 or 1 
    // 0 = elcon charging
    // 1 = elcon stop charging to enable BPS 
    payload[4] = stop_charging ? 1 : 0;

    //reserved. byte for 6-8
    payload[5] = 0x00; 
    payload[6] = 0x00; 
    payload[7] = 0x00; 
}

#include <stdint.h>

// A struct to hold the human-readable charger data
typedef struct {
    float output_voltage;
    float output_current;
    uint8_t flag_hw_failure;
    uint8_t flag_over_temp;
    uint8_t flag_input_voltage_wrong;
    uint8_t flag_starting_state;
    uint8_t flag_comm_timeout;
} ElconStatus_t;

/*
 * Unpacks the 8-byte payload from CAN ID 0x18FF50E5
 * payload: Pointer to the 8-byte array received from the CAN peripheral
 * status: Pointer to the struct where the decoded data will be saved
 */
void ElconCAN_RX(const uint8_t *payload, ElconStatus_t *status) {
    // 1. Reconstruct the 16-bit integers (High Byte << 8 | Low Byte) 
    uint16_t v_raw = ((uint16_t)payload[0] << 8) | payload[1];
    uint16_t c_raw = ((uint16_t)payload[2] << 8) | payload[3];

    // 2. Reverse the scaling to get real Volts and Amps 
    status->output_voltage = (float)v_raw / 10.0f;
    status->output_current = (float)c_raw / 10.0f;

    // 3. Extract the bit-mapped Status Flags from BYTE5 (payload[4]) [cite: 64, 65, 68]
    // Bit 0: 1 = Hardware Failure [cite: 65]
    status->flag_hw_failure = (payload[4] & 0x01) ? 1 : 0;
    
    // Bit 1: 1 = Over temperature protection [cite: 65]
    status->flag_over_temp = (payload[4] & 0x02) ? 1 : 0;
    
    // Bit 2: 1 = Input voltage is wrong [cite: 65]
    status->flag_input_voltage_wrong = (payload[4] & 0x04) ? 1 : 0;
    
    // Bit 3: 1 = Charger stays closed to prevent reverse polarity [cite: 68]
    status->flag_starting_state = (payload[4] & 0x08) ? 1 : 0;
    
    // Bit 4: 1 = Communication receive time-out [cite: 68]
    status->flag_comm_timeout = (payload[4] & 0x10) ? 1 : 0;
}