#include <WiFi.h>
#include "WEB_SERVER_LED/webserver.h"
#include "LCD/lcd.h"

/**
 * @brief This unit implement the web serverto control an LED on pin 2 of the ESP32 device (switch on/off).
 *
 */
// Network login details
const char *ssid = "GG-MagentaWLAN";
const char *password = "5994C1143g6124G2566c5313";

// LED Pin
const int ledPin = 2;

// Set up web server on port 80
WiFiServer server(80);

void web_setup()
{
    // Initialize LCD
    LCD_init();

    Serial.begin(115200);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    Serial.println("Connect to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connection is being established...");
    }

    Serial.println("Connected!");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());

    // Start web server
    server.begin();
}

void web_loop()
{
    // Waiting for Client request
    WiFiClient client = server.available();

    if (client)
    {
        Serial.println("New Client connected!");
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();

        // Check whether the request contains “GET /on” or “GET /off”.
        if (request.indexOf("GET /on") != -1)
        {
            digitalWrite(ledPin, HIGH);
            printlcd(0, 0, MESSAGE_LED);
            printlcd(0, 1, "ON ");
        }
        else if (request.indexOf("GET /off") != -1)
        {
            digitalWrite(ledPin, LOW);
            printlcd(0, 0, MESSAGE_LED);
            printlcd(0, 1, "OFF");
        }

        // Send HTTP response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("");
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.println("<h1>ESP32 LED switch</h1>");
        client.println("<p><a href=\"/on\"><button>ON</button></a></p>");
        client.println("<p><a href=\"/off\"><button>OFF</button></a></p>");
        client.println("</html>");

        delay(1);
        client.stop();
        Serial.println("Client disconnected");
    }
}