#ifndef OTAClient_H
#define OTAClient_H

#include <WiFi.h>
#include <Update.h>

class OTAClient
{
private:
  WiFiClient wifiOTAClient;
  int port = 80;
  String file = "/";
  int aws_s3_tries = 3;
  char *deviceName = nullptr;

private:
  String getHeaderValue(String header, String headerName);

public:
  explicit OTAClient();
  
  void setDeviceName(const char deviceName[]);

  bool validateUpdateMessage(String textPayload, String *updateVersion);

  void update(String version);
};

#endif
