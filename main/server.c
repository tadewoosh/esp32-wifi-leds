#include "server.h"

#include <string.h>
#include <sys/param.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern volatile float pwms[7];
extern volatile int en_H;
extern volatile int en_A;
extern volatile int en_B;

#define PORT 3333

static void sprint_status(char *buffer) {
	sprintf(buffer, "Oswietlenie u Kamy\r\n"
			"0 PWM_A_R %f\r\n"
			"1 PWM_A_G %f\r\n"
			"2 PWM_A_B %f\r\n"
			"3 PWM_B_R %f\r\n"
			"4 PWM_B_G %f\r\n"
			"5 PWM_B_B %f\r\n"
			"6 PWM_H %f\r\n"
			"7 EN_H %d\r\n"
			"8 EN_A %d\r\n"
			"9 EN_B %d\r\n", pwms[0], pwms[1], pwms[2], pwms[3], pwms[4], pwms[5], pwms[6], en_H, en_A, en_B);
}

static int prepare_socket() {

	struct sockaddr_in6 dest_addr;

	struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in*) &dest_addr;
	dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr_ip4->sin_family = AF_INET;
	dest_addr_ip4->sin_port = htons(PORT);
	int ip_protocol = IPPROTO_IP;

	int sock = socket(AF_INET, SOCK_DGRAM, ip_protocol);
	if (sock < 0) {
		printf("Unable to create socket: errno %d\r\n", errno);
		return -1;
	}

	int err = bind(sock, (struct sockaddr* )&dest_addr, sizeof(dest_addr));
	if (err < 0) {
		printf("Socket unable to bind: errno %d\r\n", errno);
		return -2;
	}
	return sock;
}

static int handle_connection(const int sock) {

	char rx_buffer[128];
	char tx_buffer[256];
	char addr_str[128];

	printf("Waiting for data\r\n");

	struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
	socklen_t socklen = sizeof(source_addr);
	int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr* )&source_addr, &socklen);

	// Error occurred during receiving
	if (len < 0) {
		printf("recvfrom failed: errno %d\r\n", errno);
		return -1;
	}
	// Data received
	else {
		// Get the sender's ip address as string
		inet_ntoa_r(((struct sockaddr_in* )&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

		// Null-terminate whatever is received and treat it like a C string
		rx_buffer[len] = 0;

		//prepare len for use in transminntin the response
		len = 0;

		printf("Received %d bytes from %s:\r\n", len, addr_str);
		printf("%s", rx_buffer);

		// interpret and prepare a response
		{
			if (strncmp("?", rx_buffer, 1) == 0) {
				// simple inquiry received, prepare status response
				sprint_status(tx_buffer);
				len = strlen(tx_buffer);

			} else if (strncmp("set ", rx_buffer, 4) == 0) {
				char *next;
				next = &rx_buffer[4];
				for (int i = 0; i < 7; i++) {
					float param_value = strtof(next, &next);

					if (param_value > 1.0)
						param_value = 1;
					if (param_value < 0)
						param_value = 0;
					//set the actual PWMs
					pwms[i] = param_value;
					//printf("Set param %d to %f\r\n", i, param_value);
				}

				// get the next 3 integers
				int param_value = (int) strtof(next, &next);
				if (param_value)
					en_H = 1;
				else
					en_H = 0;

				param_value = (int) strtof(next, &next);
				if (param_value)
					en_A = 1;
				else
					en_A = 0;

				param_value = (int) strtof(next, &next);
				if (param_value)
					en_B = 1;
				else
					en_B = 0;
			}
		}

		if (len) {
			// send the response
			int err = sendto(sock, tx_buffer, len, 0, (struct sockaddr* )&source_addr, sizeof(source_addr));
			if (err < 0) {
				printf("Error occurred during sending: errno %d\r\n", errno);
				return -2;
			}
		}

	}
	return 0;
}

void server_task(void *pvParameters) {

	printf("Server waiting for network conniection.\r\n");
	while (connected == 0) {
		vTaskDelay(100);
	}
	printf("Server starting.\r\n");

	while (1) {

		int sock = prepare_socket();

		if (sock > 0) {
			printf("Socket ready.\r\n");
			int result;
			do {
				result = handle_connection(sock);
			} while (result >= 0);
		} else { // socket creation failed

			//cleanup
			shutdown(sock, 0);
			close(sock);

			vTaskDelay(1000);
		}
	}
	vTaskDelete(NULL);
}

