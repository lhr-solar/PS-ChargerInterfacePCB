#include <stdint.h>


void ElconCAN_TX(float target_voltage_v, float target_current_a, uint8_t stop_charging, uint8_t *payload);

void ElconCAN_RX(const uint8_t *payload, ElconStatus_t *status);