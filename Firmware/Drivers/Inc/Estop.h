
#include "stm32xx_hal.h"
#include "pinDef.h"

typedef enum
{
    ESTOP_PRESSED,
    ESTOP_RELEASED
} Estop_status_t;

/**
 * @brief   Reads the Estop state
 * @return  ESTOP_PRESSED if the estop button is currently pressed, ESTOP_RELEASED if it is not pressed. Also updates the display and fault LED accordingly.
 */
Estop_status_t Estop_State();
