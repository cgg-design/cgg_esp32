#include "BLUETOOTH/bluetooth.h"
// #include "WEB_SERVER_LED/webserver.h"
#include "OTA/ota_web.h"

/**
 * @brief setup function
 *
 */
void setup()
{
  // web_setup();
  ota_setup();
}

/**
 * @brief  loop function
 *
 */
void loop()
{
  // web_loop();
  ota_loop();
}
