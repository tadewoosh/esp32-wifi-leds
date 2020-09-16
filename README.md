# WiFi LED Controller

This repository harbours code for the ESP32 in the lighting ontroller. It is configured to be built with the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/).

The application consists of four main parts contained in the four 'c' files:
* 'main.c': contains the 'app_main' function, which calls the required tasks
* 'wifi.c': contains the **WiFi task** capable of connecting to your local network.
* 'server.c' contains the **Server task**, being a UDP server handling connections with the mobile app
* 'hw_io.c' contains the **Hardware task** for low-level control of the inputs and outputs (mostly PWM).

### Wifi credentials
In order to protect pirvacy the wifi.h file has been removed, but one might implement it like this:
'''C
#ifndef WIFI_H
#define WIFI_H

void wifi_task(void *pvParameters);

#define EXAMPLE_ESP_WIFI_SSID      "Wifi Name"
#define EXAMPLE_ESP_WIFI_PASS      "Wifi password"

#endif
'''


