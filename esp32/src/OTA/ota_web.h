#ifndef OTAWEB_H
#define OTAWEB_H

#define MESSAGE_LED "The LED is"

/**
 * @brief Initialize the OTA web server and WiFi connectivity.
 *
 * Responsibilities:
 *  - Configure the LED pin as OUTPUT.
 *  - Initialize Serial for logging.
 *  - Connect to the configured WiFi network and block until connected.
 *  - Start the mDNS responder using the configured host name.
 *  - Register HTTP routes:
 *      - GET "/"         -> serves the login page (loginIndex).
 *      - GET "/serverIndex" -> serves the OTA upload page (serverIndex).
 *      - POST "/update"  -> handles firmware upload lifecycle using the
 *                           HTTPUpload API and Update.* functions.
 *  - Start the WebServer.
 *
 * OTA upload handling specifics:
 *  - On UPLOAD_FILE_START: calls Update.begin(UPDATE_SIZE_UNKNOWN) to
 *    initialize the update (use full available space).
 *  - On UPLOAD_FILE_WRITE: writes chunks via Update.write(...).
 *  - On UPLOAD_FILE_END: finalizes the update with Update.end(true) which
 *    sets the image size to the uploaded size if successful.
 *  - After the POST handler returns, the device is restarted to boot the new
 *    image if the update succeeded.
 *
 * Notes and caveats:
 *  - Update.begin(UPDATE_SIZE_UNKNOWN) is convenient but be mindful of available
 *    flash space and partition layout for your project.
 *  - The example sends a plain text "OK" or "FAIL" response on completion.
 *  - The code restarts the device unconditionally after the POST handler
 *    returns; in case of failure, Update.hasError() is used to indicate status.
 *
 * @return void
 */
void ota_setup(void);

/**
 * @brief Main loop for OTA service and heartbeat LED.
 *
 * Responsibilities:
 *  - Call server.handleClient() to process incoming HTTP requests.
 *  - Implement a non-blocking LED blink (millis-based) so the device appears
 *    alive without interfering with webserver responsiveness.
 *  - Keep delays minimal so the HTTP server remains responsive to upload
 *    requests and other network events.
 *
 * Usage:
 *  - Call repeatedly from the main program loop or scheduler to serve clients
 *    and maintain the heartbeat.
 *
 * @return void
 */
void ota_loop(void);


#endif // OTAWEB_H