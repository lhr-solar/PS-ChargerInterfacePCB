#pragma once

//===============RevB Pin Definitions============

// EVSE Pin Defs
#define MCU_PP_PIN GPIO_PIN_0
#define MCU_PP_PORT GPIOC
#define MCU_CP_PIN GPIO_PIN_2
#define MCU_CP_PORT GPIOC

// Status LED Pin Defs
#define LED_EVSE_PIN GPIO_PIN_0
#define LED_EVSE_PORT GPIOA
#define LED_CHARGE_PIN GPIO_PIN_1
#define LED_CHARGE_PORT GPIOA
#define LED_HEART_PIN GPIO_PIN_2
#define LED_HEART_PORT GPIOA
#define LED_FAULT_PIN GPIO_PIN_0
#define LED_FAULT_PORT GPIOB
#define LED_HV_PIN GPIO_PIN_1
#define LED_HV_PORT GPIOB

// Buzzer Pin Defs
#define BUZZPWM_PIN GPIO_PIN_2
#define BUZZPWM_PORT GPIOB

// Estop Pin Defs
#define ESTOP_PIN GPIO_PIN_13
#define ESTOP_PORT GPIOB

// Elcon CAN defs
#define ELCONCAN_TX_PIN GPIO_PIN_11
#define ELCONCAN_TX_PORT GPIOA
#define ELCONCAN_RX_PIN GPIO_PIN_12
#define ELCONCAN_RX_PORT GPIOA

// CarCAN defs
#define CARCAN_RX_PIN GPIO_PIN_8
#define CARCAN_RX_PORT GPIOA
#define CARCAN_TX_PIN GPIO_PIN_15
#define CARCAN_TX_PORT GPIOA

// LCD SPI Display defs
#define DISPLAY_RES_PIN GPIO_PIN_1
#define DISPLAY_RES_PORT GPIOC
#define DISPLAY_SCK_PIN GPIO_PIN_3
#define DISPLAY_SCK_PORT GPIOB
#define DISPLAY_MISO_PIN GPIO_PIN_4
#define DISPLAY_MISO_PORT GPIOB
#define DISPLAY_MOSI_PIN GPIO_PIN_5
#define DISPLAY_MOSI_PORT GPIOB
#define DISPLAY_NSS_PIN GPIO_PIN_10
#define DISPLAY_NSS_PORT GPIOA

/*




//===============RevA Pin Definitions============
#define Estop_Pin GPIO_PIN_2
#define Estop_GPIO_Port GPIOC

#define Buzzer_Pin GPIO_PIN_2
#define Buzzer_GPIO_Port GPIOB

#define Display_RES_Pin GPIO_PIN_12
#define Display_RES_GPIO_Port GPIOB
#define Display_SCK_Pin GPIO_PIN_13
#define Display_SCK_GPIO_Port GPIOB
#define Display_MISO_Pin GPIO_PIN_14
#define Display_MISO_GPIO_Port GPIOB
#define Display_MOSI_Pin GPIO_PIN_15
#define Display_MOSI_GPIO_Port GPIOB
#define Display_NSS_Pin GPIO_PIN_9
#define Display_NSS_GPIO_Port GPIOB


#define CarCAN_RX_Pin GPIO_PIN_8
#define CarCAN_RX_GPIO_Port GPIOA

#define LED_Fault_Pin GPIO_PIN_10
#define LED_Fault_GPIO_Port GPIOA

#define ElconCAN_RX_Pin GPIO_PIN_11
#define ElconCAN_RX_GPIO_Port GPIOA
#define ElconCAN_TX_Pin GPIO_PIN_12
#define ElconCAN_TX_GPIO_Port GPIOA

#define CarCAN_TX_Pin GPIO_PIN_15
#define CarCAN_TX_GPIO_Port GPIOA

#define LED_Heart_Pin GPIO_PIN_3
#define LED_Heart_GPIO_Port GPIOB
#define LED_HV_Pin GPIO_PIN_4
#define LED_HV_GPIO_Port GPIOB
#define LED_Charge_Pin GPIO_PIN_5
#define LED_Charge_GPIO_Port GPIOB

//fake EVSE pin for testing
#define LED_EVSE_Pin GPIO_PIN_0
#define LED_EVSE_GPIO_Port GPIOA

*/
