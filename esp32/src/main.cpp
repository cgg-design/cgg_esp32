#include "BLUETOOTH/bluetooth.h"
#include "WEB_SERVER_LED/webserver.h"

/**
 * @brief setup function
 *
 */
void setup()
{
  web_setup();
}

/**
 * @brief  loop function
 *
 */
void loop()
{
  web_loop();
}
