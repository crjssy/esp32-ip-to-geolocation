#include "esp32-ip-to-geolocation.h"
#include <string.h>

#define MAX_HTTP_OUTPUT_BUFFER 8192

char *output_buffer; // Buffer to store HTTP response
int output_len;      // Stores number of bytes in output_buffer

// Global event group for WiFi status
EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;

void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa((ip4_addr_t *)&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = CONFIG_WIFI_AUTH_MODE,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_WIFI_SSID);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", CONFIG_WIFI_SSID);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

// 全局变量声明
char *output_buffer = NULL; // Buffer to store HTTP response
int output_len = 0; // Stores number of bytes in output_buffer

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (evt->data_len > 0)
        {
            if (output_buffer == NULL)
            {
                // Allocate memory for the output buffer
                output_buffer = (char *)malloc(evt->data_len + 1);
                if (output_buffer == NULL)
                {
                    ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                    return ESP_FAIL;
                }
                output_len = 0;
            }
            else
            {
                // Reallocate memory for the output buffer to accommodate new data
                char *new_buffer = (char *)realloc(output_buffer, output_len + evt->data_len + 1);
                if (new_buffer == NULL)
                {
                    ESP_LOGE(TAG, "Failed to reallocate memory for output buffer");
                    free(output_buffer);
                    output_buffer = NULL;
                    output_len = 0;
                    return ESP_FAIL;
                }
                output_buffer = new_buffer;
            }

            // Copy the new data into the buffer
            memcpy(output_buffer + output_len, evt->data, evt->data_len);
            output_len += evt->data_len;
            output_buffer[output_len] = '\0'; // Null-terminate the buffer
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            // Parse the JSON data
            cJSON *json = cJSON_Parse(output_buffer);
            if (json == NULL)
            {
                ESP_LOGE(TAG, "JSON parsing error");
            }
            else
            {
                const char *keys[] = {
                    "status", "country", "countryCode", "region", "regionName", "city",
                    "zip", "lat", "lon", "timezone", "isp", "org", "as", "query"};
                int numKeys = sizeof(keys) / sizeof(keys[0]);
                for (int i = 0; i < numKeys; i++)
                {
                    cJSON *value = cJSON_GetObjectItemCaseSensitive(json, keys[i]);
                    if (cJSON_IsString(value) && (value->valuestring != NULL))
                    {
                        ESP_LOGI(TAG, "%s: %s", keys[i], value->valuestring);
                    }
                    else if (cJSON_IsNumber(value))
                    {
                        ESP_LOGI(TAG, "%s: %f", keys[i], value->valuedouble);
                    }
                }

                cJSON_Delete(json);
            }
            // Free the output buffer
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
        }
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
            output_len = 0;
        }
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;
    default:
        ESP_LOGI(TAG, "Unhandled HTTP event: %d", evt->event_id);
        break;
    }
    return ESP_OK;
}

void http_get_task(void *pvParameters)
{
    output_buffer = NULL;
    output_len = 0;

    esp_http_client_config_t config = {
        .url = URL,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_GET
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status = esp_http_client_get_status_code(client);
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status, content_length);
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    // Free the output buffer if it was allocated
    if (output_buffer != NULL)
    {
        free(output_buffer);
        output_buffer = NULL;
        output_len = 0;
    }
    vTaskDelete(NULL);
}


void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 设置日志级别为调试
    esp_log_level_set("*", ESP_LOG_DEBUG);

    wifi_init_sta();
    xTaskCreate(&http_get_task, "http_get_task", 8192, NULL, 5, NULL);
}
