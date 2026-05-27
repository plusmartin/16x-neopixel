#include <Arduino.h>
#include <esp_https_ota.h>
#include <esp_crt_bundle.h>
#include "OTAClient.h"

void otaUpdate(const char* url)
{
  Serial0.printf("[OTA] Fetching %s\n", url);

  esp_http_client_config_t http_cfg = {};
  http_cfg.url                      = url;
  http_cfg.crt_bundle_attach        = arduino_esp_crt_bundle_attach;
  http_cfg.max_redirection_count    = 5;
  http_cfg.timeout_ms               = 30000;
  http_cfg.keep_alive_enable        = true;

  esp_https_ota_config_t ota_cfg = {};
  ota_cfg.http_config = &http_cfg;

  esp_https_ota_handle_t handle = nullptr;
  esp_err_t err = esp_https_ota_begin(&ota_cfg, &handle);
  if (err != ESP_OK) {
    Serial0.printf("[OTA] Begin failed: %s\n", esp_err_to_name(err));
    return;
  }

  while (true) {
    err = esp_https_ota_perform(handle);
    if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) break;
    Serial0.printf("[OTA] Written: %d bytes\n", esp_https_ota_get_image_len_read(handle));
  }

  if (err != ESP_OK) {
    Serial0.printf("[OTA] Failed: %s\n", esp_err_to_name(err));
    esp_https_ota_abort(handle);
    return;
  }

  if (esp_https_ota_finish(handle) == ESP_OK) {
    Serial0.println("[OTA] Success — rebooting.");
    delay(500);
    esp_restart();
  } else {
    Serial0.println("[OTA] Finish failed.");
  }
}
