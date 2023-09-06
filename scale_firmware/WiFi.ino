#define BLYNK_PRINT Serial

#include <SPI.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>
#include "arduino_secrets.h"

#define BLYNK_TEMPLATE_ID "TMPLf27sVrh6"
#define BLYNK_TEMPLATE_NAME "NCCCS Drain Monitor"
#define BLYNK_AUTH_TOKEN SECRET_BLYNK_AUTH_TOKEN

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

#define BLYNK_DATASTREAM_VOLUME    V0
#define BLYNK_DATASTREAM_FLOW_RATE V1
#define BLYNK_DATASTREAM_TIME      V2
#define BLYNK_DATASTREAM_CONTAINER V3
#define BLYNK_DATASTREAM_PERCENTAGE_FILL V4

void WiFi_init()
{
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void WiFi_run()
{
  DisplayParameters param = TFT_getParameters();
  if (Blynk.connectionHandler() == BlynkWifiCommon::Status::CONNECTED) {
    Blynk.run();
    param.network_available = true;
  } else {
    param.network_available = false;
  }
  TFT_setParameters(param);
}

void WiFi_sendFlowRate(float value)
{
  Blynk.virtualWrite(BLYNK_DATASTREAM_FLOW_RATE, (double)value);
}

void WiFi_sendVolume(float value)
{
  Blynk.virtualWrite(BLYNK_DATASTREAM_VOLUME, (double)value);
}

void WiFi_sendTime(String value)
{
  Blynk.virtualWrite(BLYNK_DATASTREAM_TIME, value);
}

void WiFi_sendPercentageFill(int value)
{
  Blynk.virtualWrite(BLYNK_DATASTREAM_PERCENTAGE_FILL, value);
}

void WiFi_sendContainerType(float value)
{
  Blynk.virtualWrite(BLYNK_DATASTREAM_CONTAINER, value);
}