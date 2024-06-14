#ifndef MAIN_H
#define MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define URL "http://ip-api.com/json"
#define TAG "GEOLOCATION"

extern EventGroupHandle_t s_wifi_event_group;
extern const int WIFI_CONNECTED_BIT;
extern const int WIFI_FAIL_BIT;

/**
 * Event handler for WiFi and IP events.
 */
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/**
 * Initialize WiFi as a station with the specified SSID and password.
 */
void wifi_init_sta(void);

/**
 * Task that performs an HTTP GET request to retrieve geolocation information.
 */
void http_get_task(void *pvParameters);

/**
 * Event handler for HTTP events.
 */
esp_err_t http_event_handler(esp_http_client_event_t *evt);

/**
 * Main application entry point.
 */
void app_main(void);

#ifdef __cplusplus
}
#endif

#endif // MAIN_H
