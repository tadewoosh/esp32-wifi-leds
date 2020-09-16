#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "hw_io.h"
#include "server.h"

void app_main(void) {
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	xTaskCreate(hw_io_task, "hw_io", 4096, NULL, 5, NULL);
	xTaskCreate(wifi_task, "wifi", 4096, NULL, 5, NULL);
	xTaskCreate(server_task, "server", 2*4096, NULL, 5, NULL);

}
