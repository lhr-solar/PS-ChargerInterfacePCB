// Elcon CAN = CAN1
// Literals used directly: CAN_Common.c doesn't include ElconCAN.h, so macros
// from that header are not in scope when this file is expanded.
// ELCONCAN_RX_ID = 0x18FF50E5, DLC = 5

#include "ElconCAN.h"
#include "CarCAN_can_msgs.h"
CAN_RECV_ENTRY(ELCONCAN_RX_ID, 5, false)
