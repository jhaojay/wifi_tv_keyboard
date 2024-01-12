#include <string.h>
#include <mqtt_client.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "tinyusb.h"
#include "class/hid/hid_device.h"


#define WIFI_SSID   ""
#define WIFI_PASS   ""
#define EXAMPLE_ESP_MAXIMUM_RETRY  10


#define LED_PIN GPIO_NUM_8

#define MQTT_SERVER "mqtt://192.168.1.217"

#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)


const uint8_t hid_report_descriptor[] = {
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD) ),
        TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_ITF_PROTOCOL_MOUSE) )
};

/**
 * @brief String descriptor
 */
const char* hid_string_descriptor[5] = {
        // array of pointer to string descriptors
        (char[]){0x09, 0x04},  // 0: is supported language is English (0x0409)
        "TinyUSB",             // 1: Manufacturer
        "TinyUSB Device",      // 2: Product
        "123456",              // 3: Serials, should use chip ID
        "Example HID interface",  // 4: HID
};

/**
 * @brief Configuration descriptor
 *
 * This is a simple configuration descriptor that defines 1 configuration and 1 HID interface
 */
static const uint8_t hid_configuration_descriptor[] = {
        // Configuration number, interface count, string index, total length, attribute, power in mA
        TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

        // Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval
        TUD_HID_DESCRIPTOR(0, 4, false, sizeof(hid_report_descriptor), 0x81, 16, 10),
};

/********* TinyUSB HID callbacks ***************/

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    // We use only one interface and one HID report descriptor, so we can ignore parameter 'instance'
    return hid_report_descriptor;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
}

/********* Application ***************/

static void app_send_hid_demo(const char *key)
{
    static const char *TAG = "HID APP";
    uint8_t keycode[6] = {HID_KEY_SPACE};

    const char alp_key = key[0];
    if (strncmp(key, " ", strlen(" ")) == 0) {
        keycode[0] = HID_KEY_SPACE;
    } else if (strncmp(key, "Enter", strlen("Enter")) == 0) {
        keycode[0] = HID_KEY_ENTER;
    } else if (strncmp(key, "Backspace", strlen("Backspace")) == 0) {
        keycode[0] = HID_KEY_BACKSPACE;
    } else if ('a' <= alp_key && alp_key <= 'z') {
        int diff = alp_key - 'a';
        keycode[0] = HID_KEY_A + diff;
    } else if ('1' <= alp_key && alp_key <= '9') {
        int diff = alp_key - '1';
        keycode[0] = HID_KEY_1 + diff;
    } else if ('A' <= alp_key && alp_key <= 'Z') {
        ESP_LOGI(TAG, "Sending Keyboard report: %d", HID_KEY_SHIFT_LEFT);
        int diff = alp_key - 'A';
        keycode[0] = HID_KEY_SHIFT_LEFT;
        keycode[1] = HID_KEY_A + diff;
        tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
    } else if (alp_key == '0') {
        keycode[0] = HID_KEY_0;
    }
    ESP_LOGI(TAG, "Sending Keyboard report: %d", keycode[0]);

    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
    vTaskDelay(pdMS_TO_TICKS(50));
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, NULL);

}


static int s_retry_num = 0;


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    static const char *TAG = "WIFI_APP";
    ESP_LOGI(TAG, "event_base=%s, event_id=%ld", event_base, event_id); // Changed %ld to %d

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            ESP_LOGI(TAG, "Max retries reached");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

void blink(int ms)
{
    // Set the GPIO as a push/pull output
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_PIN, 1);  // Turn the LED on
    vTaskDelay(ms / portTICK_PERIOD_MS);  // Wait for a second
    gpio_set_level(LED_PIN, 0);  // Turn the LED off
    vTaskDelay(ms / portTICK_PERIOD_MS);  // Wait for a second
}

void connect_wifi()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &wifi_event_handler,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler,
                                               NULL));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = WIFI_SSID,
                    .password = WIFI_PASS,
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    static const char *TAG = "WIFI_APP";
    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// MQTT event handler
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    static const char *TAG = "MQTT_EXAMPLE";

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            // Subscribe to a topic with QoS 0
            int msg_id = esp_mqtt_client_subscribe(client, "tv_keyboard", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if (strncmp(event->data, "blink", strlen("blink")) == 0) {
                ESP_LOGI(TAG, "Received message: blink");
                blink(1000);
                break;
            }
            ESP_LOGI(TAG, "Received message: %s", event->data);
            app_send_hid_demo(event->data);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;

        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

void start_mqtt()
{
    const esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = MQTT_SERVER,
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_ERROR_CHECK(
            esp_mqtt_client_register_event(
                    client,
                    ESP_EVENT_ANY_ID,
                    mqtt_event_handler,
                    client
            )
    );

    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}

void app_main(void)
{
    connect_wifi();
    vTaskDelay(5000 /portTICK_PERIOD_MS); //wait for it to connect to wifi
    start_mqtt();

    const char *TAG = "HID APP";
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
            .device_descriptor = NULL,
            .string_descriptor = hid_string_descriptor,
            .string_descriptor_count = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
            .external_phy = false,
            .configuration_descriptor = hid_configuration_descriptor,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_LOGI(TAG, "USB initialization DONE");


}
