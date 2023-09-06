#include "TFT.hpp"
#include "Util.hpp"
#include "DischargeEstimator.hpp"

#define BUTTON_PIN 5
#define BUTTON_SOURCE 6

unsigned long int time;

String container_labels[5] = {" 100ml", " 200ml", " 300ml", " 500ml", "1000ml"};
int container_volumes[5] = {100, 200, 300, 500, 1000};
int container_type_count = 5;

void setup() {
  Serial.begin(9600);
  Battery_init();
  pinMode(BUTTON_SOURCE, OUTPUT);
  digitalWrite(BUTTON_SOURCE, LOW);
  
  Button_init(BUTTON_PIN, buttonCallback);
  TFT_init();
  TFT_showScreen(TFT_Screen::STARTUP_SCREEN);
  Scale_init();
  DE_init();
  WiFi_init();
  
  TFT_showScreen(TFT_Screen::SELECTION_SCREEN);
  Scale_registerStableWeighingDataCallback(DE_weightCallback);
  Scale_registerUnstableWeighingDataCallback(DE_weightCallback);
}


void loop() {
  delay(1);
  Battery_run();
  Scale_run();
  DE_run();
  TFT_run();
  WiFi_run();
  Button_run();
  
  DisplayParameters param = TFT_getParameters();
  // Time update
  if (millis() - time > 1000) {
    time = millis();
    param.time++;
  }
  
  TFT_setParameters(param);
}

void buttonCallback(int short_press, int long_press)
{
  DisplayParameters param = TFT_getParameters();
  
  switch (TFT_getScreen())
  {
    case TFT_Screen::SELECTION_SCREEN: {
      param.container_type += short_press;
      param.container_type = param.container_type % container_type_count;
      
      if (long_press) {
        param.time = 0;
        TFT_showScreen(TFT_Screen::INFO_SCREEN);
      }
      break;
    }
    case TFT_Screen::INFO_SCREEN: {
      if (long_press) {
        TFT_showScreen(TFT_Screen::SHUTDOWN_SCREEN);
        Battery_disable();
      }
      break;
    }
    case TFT_Screen::SHUTDOWN_SCREEN: {
      if (long_press) {
        Battery_enable();
        TFT_showScreen(TFT_Screen::SELECTION_SCREEN);
      }
      break;
    }
    default:;
  };
  
  TFT_setParameters(param);
}
