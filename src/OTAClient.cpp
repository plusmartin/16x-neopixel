#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "OTAClient.h"

// Follow redirects and return the final URL.
static String resolveUrl(const char* url)
{
  String current = url;
  for (int i = 0; i < 5; i++) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, current);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
    int code = http.sendRequest("HEAD");
    String location = http.getLocation();
    http.end();
    if ((code == 301 || code == 302 || code == 307 || code == 308) && location.length()) {
      Serial0.printf("[OTA] Redirect → %s\n", location.c_str());
      current = location;
    } else {
      break;
    }
  }
  return current;
}

void otaUpdate(const char* url)
{
  Serial0.printf("[OTA] Fetching %s\n", url);

  String finalUrl = resolveUrl(url);
  Serial0.printf("[OTA] Final URL: %s\n", finalUrl.c_str());

  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.rebootOnUpdate(true);

  t_httpUpdate_return ret = httpUpdate.update(client, finalUrl);

  switch (ret) {
    case HTTP_UPDATE_OK:
      Serial0.println("[OTA] Success — rebooting.");
      break;
    case HTTP_UPDATE_FAILED:
      Serial0.printf("[OTA] Failed (%d): %s\n",
                     httpUpdate.getLastError(),
                     httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial0.println("[OTA] No update available.");
      break;
  }
}
