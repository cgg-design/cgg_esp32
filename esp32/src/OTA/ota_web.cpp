
/**
 * @file ota_web.cpp
 * @brief ESP32 Over-The-Air (OTA) update web interface and supporting utilities.
 *
 * This file implements a simple HTTP server that hosts a login page and an OTA
 * firmware upload page. It establishes a WiFi connection, advertises the device
 * via mDNS, and exposes endpoints to upload a new firmware image which is
 * written to flash using the Update API. A status LED is toggled periodically
 * to indicate the device is running.
 *
 * Key features:
 *  - Connects to a WiFi network and prints connection state to Serial.
 *  - Starts mDNS responder for easy access (http://<host>.local).
 *  - Hosts a login page and an OTA upload page via an embedded HTML form.
 *  - Handles multipart POST firmware uploads and performs flash update.
 *  - Restarts the device after successful update.
 *  - Non-blocking LED blink logic implemented in ota_loop().
 *
 * Security note:
 *  - The example uses a static username/password in the frontend for demo
 *    purposes only; it provides no real authentication. Protect OTA endpoints
 *    appropriately in production (use HTTPS, authentication, or network
 *    isolation).
 * 
 * @author Cyrille Guetsop Guimapi (you@domain.com)
 * @brief Flashing firmware to ESP Over Ther AIr
 * @version 0.1
 * @date 2025-12-21
 * 
 * @copyright Copyright (c) 2025
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

const char* host = "esp32";
const char *ssid = "GG-MagentaWLAN";
const char *password = "5994C1143g6124G2566c5313";

const int led = 2;
unsigned long previousMillis = 0;    
const long interval = 1000;         
int ledState = LOW;   

WebServer server(80);

/**
 * @brief HTML login page string for the OTA web interface.
 *
 * This C-style string contains the complete HTML + JavaScript for a simple
 * client-side login page. The page includes:
 *  - An HTML form named "loginForm" with text inputs for "userid" and "pwd".
 *  - A JavaScript function `check(form)` that compares the entered values
 *    against the hard-coded credentials "admin" / "admin".
 *  - On successful match the script opens "/serverIndex"; on failure it shows
 *    an alert with an error message.
 *
 * Usage:
 *  - Sent as the HTTP response body to present a login UI to the user.
 *  - The checking logic runs entirely in the browser via JavaScript.
 *
 * Security implications and recommendations:
 *  - WARNING: Authentication is implemented client-side and uses hard-coded
 *    credentials. This is inherently insecure — users can view/modify the page
 *    source or bypass the check entirely. Do NOT rely on this for real
 *    authentication or to protect firmware updates.
 *  - Remove hard-coded credentials from firmware. Implement server-side
 *    authentication and authorization for OTA endpoints.
 *  - Serve the interface over HTTPS/TLS to protect credentials in transit.
 *  - Use POST for credential submission and validate/sanitize inputs server-side.
 *  - Consider rate limiting, brute-force protections, and secure credential storage.
 *
 * Implementation notes:
 *  - The HTML form currently lacks a method/action attribute and uses
 *    window.open('/serverIndex') upon successful client-side validation.
 *  - For maintainability, consider separating HTML/JS from firmware source or
 *    using a templating approach instead of a single long string literal.
 *
 * Variable:
 *  - loginIndex : const char*  — contains the HTML and JS for the login page.
 */
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/**
 * @brief HTML page served for performing OTA firmware updates via a web form.
 *
 * This C string contains a minimal web UI that allows a user to upload a firmware
 * file to the device and performs the update using an AJAX POST to the device's
 * "/update" endpoint.
 *
 * Contents and behavior:
 * - Loads jQuery (via Google CDN) to simplify the AJAX form submission and event handling.
 * - Presents a multipart/form-data form (id="upload_form") with a single file input
 *   named "update" and a submit button.
 * - Prevents default form submission and submits the file using jQuery.ajax with:
 *     - type: POST to "/update"
 *     - data: FormData object built from the form
 *     - contentType: false and processData: false to allow binary/multipart upload
 * - Attaches an XMLHttpRequest upload progress listener that updates the DOM element
 *   with id "prg" to show upload progress in percent (e.g., "progress: 42%").
 * - On successful completion the client logs "success!" to the console. Error handling
 *   is present but left effectively empty (no UI error shown).
 *
 * Usage notes:
 * - The server must implement an HTTP POST handler at "/update" that accepts
 *   multipart/form-data with a file field named "update" and performs the actual
 *   OTA flashing process.
 * - Because jQuery is loaded from an external CDN, ensure the device or client
 *   environment can access that URL or host a local copy if offline operation is required.
 * - The HTML is embedded as a C string literal; take care with memory/storage if the
 *   firmware image or other resources are large.
 *
 * Security considerations:
 * - No authentication is provided in the page itself; ensure the update endpoint is
 *   protected (e.g., by network isolation, access control, or an authentication layer)
 *   to prevent unauthorized firmware uploads.
 *
 * @var serverIndex
 */
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

void ota_setup(void) {
  pinMode(led, OUTPUT);
  
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}


void ota_loop(void) {
  server.handleClient();
  delay(1);

  //loop to blink without delay
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    ledState = not(ledState);

    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

