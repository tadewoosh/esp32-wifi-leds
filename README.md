# WiFi LED Controller

This repository harbours code for the ESP32 in the lighting ontroller. It is configured to be built with the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/).

The application consists of four main parts contained in the four 'c' files:
* `main.c`: contains the `app_main` function, which calls the required tasks
* `wifi.c`: contains the **WiFi task** capable of connecting to your local network.
* `server.c` contains the **Server task**, being a UDP server handling connections with the mobile app
* `hw_io.c` contains the **Hardware task** for low-level control of the inputs and outputs (mostly PWM).

### Wifi credentials
In order to protect pirvacy the `wifi.h` file has been removed, but one might implement it like this to present the public interface of `wifi.c` to the main file:

```C

#ifndef WIFI_H
#define WIFI_H

void wifi_task(void *pvParameters);

#define EXAMPLE_ESP_WIFI_SSID      "Wifi Name"
#define EXAMPLE_ESP_WIFI_PASS      "Wifi password"

#endif
```
### Console output
After running `make flash monitor` the software should build, binary should be flashed the device and the app should print some info. After the bootloader messages this should be shown:

```

Hardware IO task started.
Wifi task started.
Server waiting for network conniection.
Got ip: 192.168.8.142 
Wifi connected to ap SSID: 'Wifi Name'
Server starting.
Socket ready.
Waiting for data

```
This shows, that all the tasks have started, the Wifi is properly configured and is connected. The UDP server waits for the wifi, then starts listening for incomming messages on port `3333`. At this moment the device should be discoverable and controllable by the mobile app.
