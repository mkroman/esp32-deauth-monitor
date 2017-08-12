#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <esp_log.h>
#include <soc/rtc.h>
#include "driver/gpio.h"
#include "ieee80211.h"

static const char *TAG = "deauth-monitor";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

extern "C" uint8_t temprature_sens_read();

int8_t wifi_get_target_channel() {
    wifi_scan_config_t config = {
        .ssid = (uint8_t*)CONFIG_WIFI_SSID,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = { .active = { .min = 100, .max = 300 } },
    };

    ESP_ERROR_CHECK( esp_wifi_scan_start(&config, true) );

    uint16_t ap_num = 5;
    wifi_ap_record_t wifi_records[ap_num];

    ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records(&ap_num, wifi_records) );

    // If ap_num is non-zero, we found a matching SSID
    if (ap_num > 0) {
        // Return the channel for the first result
        return wifi_records[0].primary;
    }

    return -1;
}

void wifi_rf_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t* pkt = reinterpret_cast<wifi_promiscuous_pkt_t*>(buf);
    ieee80211::Header* hdr = reinterpret_cast<ieee80211::Header*>(pkt->payload);

    if (hdr->is_deauth()) {
        ESP_LOGI(TAG, "DEAUTH: addr1=" MACSTR " addr2=" MACSTR " addr3=" MACSTR " addr4=" MACSTR,
                 MAC2STR(hdr->addr1), MAC2STR(hdr->addr2) , MAC2STR(hdr->addr3),
                 MAC2STR(hdr->addr4));
    }
}

void init_promiscuous_wifi(uint8_t target_channel) {
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT
    };

    ESP_ERROR_CHECK( esp_wifi_set_promiscuous_filter(&filter) );
    ESP_ERROR_CHECK( esp_wifi_set_channel(target_channel, WIFI_SECOND_CHAN_NONE) );
    ESP_ERROR_CHECK( esp_wifi_set_promiscuous(true) );
    ESP_ERROR_CHECK( esp_wifi_set_promiscuous_rx_cb(wifi_rf_cb) );
}

extern "C" void app_main(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    uint32_t cpu_frequency_hz = rtc_clk_cpu_freq_value(rtc_clk_cpu_freq_get());

    ESP_LOGI(TAG, "CPU frequency: %d MHz", cpu_frequency_hz / 1000000);

    nvs_flash_init();
    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    int8_t target_channel = -1;

    // Find the WiFi channel for the target SSID
    while ((target_channel = wifi_get_target_channel()) == -1) {
        ESP_LOGW(TAG, "Could not find target SSID %s - trying again", CONFIG_WIFI_SSID);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Target monitoring channel: %d", target_channel);

    init_promiscuous_wifi(target_channel);

    fflush(stdout);
}

