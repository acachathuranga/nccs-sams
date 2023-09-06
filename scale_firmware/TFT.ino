#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <avr/dtostrf.h>
#include "TFT.hpp"
#include "Util.hpp"

// Constants and Defines
const int TFT_CS = 10;
const int TFT_RST = 9;
const int TFT_DC = 8;
const int TFT_INFO_UPDATE_MS = 1000;
const int TFT_SELECTION_UPDATE_MS = 10;

// Variables
Adafruit_ST7735 TFT_display_ = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
DisplayParameters TFT_parameters_;
DisplayParameters TFT_prev_parameters_;
TFT_Screen TFT_screen_;
unsigned long int TFT_update_time_;

void TFT_init()
{
  TFT_display_.initR(INITR_GREENTAB);      // Init ST7735S chip
  TFT_display_.setRotation(1);             // Rotate display 90 degrees
  TFT_display_.fillScreen(ST77XX_BLACK);
  delay(500);
  
  TFT_update_time_ = millis();
}

void TFT_run()
{
  switch(TFT_screen_)
  {
    case TFT_Screen::STARTUP_SCREEN: {
      break;
    } 
    case TFT_Screen::SELECTION_SCREEN: { 
      if ((millis() - TFT_update_time_) > TFT_SELECTION_UPDATE_MS) {
        TFT_updateSelectionScreen();
        TFT_update_time_ = millis();
      }
      break;
    }
    case TFT_Screen::INFO_SCREEN: {
      if ((millis() - TFT_update_time_) > TFT_INFO_UPDATE_MS) {
        TFT_updateInfoScreen();
        TFT_update_time_ = millis();
      }
      break;
    }
    case TFT_Screen::SHUTDOWN_SCREEN: {
      break;
    }
    default: {
      TFT_display_.fillScreen(ST77XX_BLACK); // Error
    }
  }
}

TFT_Screen TFT_getScreen()
{
  return TFT_screen_;
}

void TFT_showScreen(TFT_Screen screen)
{
  TFT_screen_ = screen;
  switch(screen)
  {
    case TFT_Screen::STARTUP_SCREEN: TFT_showStartupScreen(); break;
    case TFT_Screen::SELECTION_SCREEN: TFT_showSelectionScreen(); break;
    case TFT_Screen::INFO_SCREEN: TFT_showInfoScreen(); break;
    case TFT_Screen::SHUTDOWN_SCREEN: TFT_showShutdownScreen(); break;
    default: TFT_display_.fillScreen(ST77XX_BLACK); // Error
  }
}

void TFT_updateSelectionScreen()
{
  if (TFT_prev_parameters_.container_type != TFT_parameters_.container_type)
  {
    int cursor_x = 40;
    int cursor_y = 10;
    TFT_display_.setTextSize(2);
  
    // Unhighlight previous selection
    TFT_display_.setTextColor(ST77XX_WHITE);
    TFT_display_.setCursor(cursor_x, cursor_y + 20 * TFT_prev_parameters_.container_type);
    TFT_unHighlightText(container_labels[TFT_prev_parameters_.container_type].c_str());
    
    // New Selection
    TFT_display_.setTextColor(ST77XX_BLACK);
    TFT_display_.setCursor(cursor_x, cursor_y + 20 * TFT_parameters_.container_type);
    TFT_drawHighlightedText(container_labels[TFT_parameters_.container_type].c_str(), ST77XX_CYAN);
  }
  TFT_prev_parameters_.container_type = TFT_parameters_.container_type;
}

void TFT_showSelectionScreen()
{
  TFT_display_.setTextWrap(false);
  TFT_display_.fillScreen(ST77XX_BLACK);
  
  TFT_display_.setTextColor(ST77XX_WHITE);
  TFT_display_.setTextSize(2);
  
  int cursor_x = 40;
  int cursor_y = 10;
  
  for (int i = 0; i < (sizeof(container_volumes) / sizeof(container_volumes[0])); i++)
  {
    TFT_display_.setCursor(cursor_x, cursor_y + 20 * i);
    TFT_display_.println(container_labels[i].c_str());
  }
  
  TFT_display_.setCursor(0, 110);
  TFT_display_.setTextColor(ST77XX_WHITE);
  TFT_display_.setTextSize(1);
  TFT_display_.println("  Press to select \n  Long press to start");
  
  // Highlight current selection
  TFT_display_.setTextColor(ST77XX_BLACK);
  TFT_display_.setTextSize(2);
  TFT_display_.setCursor(cursor_x, cursor_y + 20 * TFT_parameters_.container_type);
  TFT_drawHighlightedText(container_labels[TFT_parameters_.container_type].c_str(), ST77XX_CYAN);
}

void TFT_showShutdownScreen()
{
  TFT_display_.setTextWrap(false);
  TFT_display_.fillScreen(ST77XX_BLACK);
  
  TFT_display_.setTextColor(ST77XX_WHITE);
  TFT_display_.setCursor(2, 2);
  TFT_display_.setTextSize(0);
  TFT_display_.print("Device can be safely \n powered off");
}

void TFT_updateInfoScreen()
{
  if (TFT_parameters_.container_type != TFT_prev_parameters_.container_type) {
    TFT_display_.fillRect(98, 2, 160, 10, ST77XX_BLACK);
    TFT_display_.setTextColor(ST77XX_MAGENTA);
    TFT_display_.setCursor(98, 2);
    TFT_display_.setTextSize(0);
    TFT_display_.print(container_labels[TFT_parameters_.container_type].c_str());
  }
  
  TFT_showBattery();
  
  TFT_display_.setTextColor(ST77XX_GREEN);
  TFT_display_.setCursor(10, 40);
  TFT_display_.setTextSize(3);
  char volume[7], prev_volume[7];
  TFT_parameterToChars(TFT_parameters_.volume_ml, volume);
  TFT_parameterToChars(TFT_prev_parameters_.volume_ml, prev_volume);
  // Clear changed characters
  for (int i = 0; i < 6; i++) {
    if (volume[i] != prev_volume[i]) {
      TFT_display_.setTextColor(ST77XX_BLACK);
      TFT_display_.print(prev_volume[i]);
     
    } else {
       TFT_display_.setTextColor(ST77XX_GREEN);
       TFT_display_.print(volume[i]);
    }
  }
  // Write complete string
  TFT_display_.setTextColor(ST77XX_GREEN);
  TFT_display_.setCursor(10, 40);
  TFT_display_.print(volume);
  TFT_display_.println("ml");
  
  TFT_display_.setTextColor(ST77XX_YELLOW);
  TFT_display_.setCursor(38, 70);
  TFT_display_.setTextSize(2);
  char flow[7], prev_flow[7];
  TFT_parameterToChars(TFT_parameters_.flow_rate_mlps, flow);
  TFT_parameterToChars(TFT_prev_parameters_.flow_rate_mlps, prev_flow);
  // Clear changed characters
  for (int i = 0; i < 6; i++) {
    if (flow[i] != prev_flow[i]) {
      TFT_display_.setTextColor(ST77XX_BLACK);
      TFT_display_.print(prev_flow[i]);
      
    } else {
      TFT_display_.setTextColor(ST77XX_YELLOW);
      TFT_display_.print(flow[i]);
    }
  }
  // Write complete string
  TFT_display_.setTextColor(ST77XX_YELLOW);
  TFT_display_.setCursor(38, 70);
  TFT_display_.print(flow);
  TFT_display_.println("ml/s");
  
  TFT_display_.setTextColor(ST77XX_CYAN);
  TFT_display_.setCursor(92, 90);
  TFT_display_.setTextSize(1);
  char time[9], prev_time[9];
  SecondsToHMS(TFT_parameters_.time, time);
  SecondsToHMS(TFT_prev_parameters_.time, prev_time);
  for (int i = 0; i < 9; i++) {
    if (prev_time[i] != time[i]) {
      TFT_display_.setTextColor(ST77XX_BLACK);
      TFT_display_.print(prev_time[i]);
    } else {
      TFT_display_.setTextColor(ST77XX_CYAN);
      TFT_display_.print(time[i]);
    }
  }
  // Write complete time
  TFT_display_.setCursor(92, 90);
  TFT_display_.setTextColor(ST77XX_CYAN);
  TFT_display_.print(time);
  
  if (TFT_parameters_.network_available != TFT_prev_parameters_.network_available) {
    TFT_display_.setCursor(20, 110);
    TFT_display_.setTextSize(1);
    TFT_display_.fillRect(20, 110, 160, 125, ST77XX_BLACK); // Clear Line
    if (TFT_parameters_.network_available) {
      TFT_display_.setTextColor(ST77XX_WHITE);
      TFT_display_.print("Network Connected");
    } else {
      TFT_display_.setTextColor(ST77XX_BLUE);
      TFT_display_.print("Network Disconnected");
    }
  }
  
  TFT_prev_parameters_ = TFT_parameters_;
}

void TFT_showInfoScreen()
{
  TFT_display_.setTextWrap(false);
  TFT_display_.fillScreen(ST77XX_BLACK);
  
  TFT_display_.setTextColor(ST77XX_WHITE);
  TFT_display_.setCursor(2, 2);
  TFT_display_.setTextSize(0);
  TFT_display_.print("Container Type: ");
 
  TFT_display_.setTextColor(ST77XX_RED);
  TFT_display_.setCursor(2, 20);
  TFT_display_.setTextSize(2);
  TFT_display_.print("Vol: ");
  
  TFT_display_.setTextColor(ST77XX_RED);
  TFT_display_.setCursor(2, 70);
  TFT_display_.setTextSize(1);
  TFT_display_.print("Flow: ");
  
  TFT_display_.setTextColor(ST77XX_RED);
  TFT_display_.setCursor(2, 90);
  TFT_display_.setTextSize(1);
  TFT_display_.print("Time Elapsed:  ");
  char time[9];
  TFT_display_.setTextColor(ST77XX_CYAN);
  TFT_display_.setCursor(92, 90);
  SecondsToHMS(TFT_parameters_.time, time);
  TFT_display_.print(time);
  
  // Changing Previous state (Required for initial display update)
  TFT_prev_parameters_.container_type++;
  TFT_prev_parameters_.network_available = !TFT_prev_parameters_.network_available;
  TFT_prev_parameters_.battery = 100;
}

DisplayParameters TFT_getParameters(void)
{
  return TFT_parameters_;
}

void TFT_setParameters(DisplayParameters param)
{
  TFT_parameters_ = param;
}

void TFT_showStartupScreen()
{
  TFT_bootLogo();
  TFT_display_.fillScreen(ST77XX_BLACK);
  TFT_drawText("National Cancer Center \nSignapore : Breast Drainage Weight Measurement. "
                "Developed By SUTD. Prototpye V1.0"
                "\nDisclaimer: This product has a built-in backup power supply. "
                " However it should be used with charger connected", ST77XX_WHITE);
  delay(2000);
  // TFT_display_.fillScreen(ST77XX_BLACK);              
}

void TFT_bootLogo()
{
  TFT_display_.fillScreen(ST77XX_BLACK);
  uint16_t color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 1; t+=1) {
    int x = 0;
    int y = 0;
    int w = TFT_display_.width()-2;
    int h = TFT_display_.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      TFT_display_.drawRoundRect(x, y, w, h, 5, color);
      delay(100);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

void TFT_drawText(char *text, uint16_t color) 
{
  TFT_display_.setCursor(0, 0);
  TFT_display_.setTextColor(color);
  TFT_display_.setTextWrap(true);
  TFT_display_.print(text);
}

void TFT_showBattery()
{
  // Check if change in value
  if (TFT_prev_parameters_.battery != TFT_parameters_.battery) {
    if (TFT_parameters_.battery < 0) {
      // Clear battery and show plug
      TFT_display_.fillRect(110, 15, 25, 10, ST77XX_BLACK);
      
      TFT_display_.fillTriangle(120, 20, 130, 15, 130, 25, ST77XX_WHITE);
      TFT_display_.drawLine(115, 20, 120, 20, ST77XX_WHITE);
      TFT_display_.drawLine(130, 18, 133, 18, ST77XX_WHITE);
      TFT_display_.drawLine(130, 22, 133, 22, ST77XX_WHITE);
    }
    else if (abs(TFT_prev_parameters_.battery - TFT_parameters_.battery) > 20) {
      // Significant change. Update display
      
      // Clear plug and show battery
      TFT_display_.fillRect(110, 15, 25, 10, ST77XX_BLACK);
      
      TFT_display_.drawRect(110, 15, 20, 10, ST77XX_WHITE);
      TFT_display_.fillRect(130, 18, 2, 4, ST77XX_WHITE);
      int fill = 18 * TFT_parameters_.battery / 100;
      TFT_display_.fillRect(111, 16, fill, 8, ST77XX_GREEN);
    }
  }
}

void TFT_drawHighlightedText(const char *text, uint16_t highlight_color)
{
  int16_t x, y;
  uint16_t w, h;
  TFT_display_.getTextBounds(text, TFT_display_.getCursorX(), TFT_display_.getCursorY(), &x, &y, &w, &h);
  TFT_display_.fillRect(x, y, w, h, highlight_color);
  TFT_display_.println(text);
}

void TFT_unHighlightText(const char *text)
{
  int16_t x, y;
  uint16_t w, h;
  TFT_display_.getTextBounds(text, TFT_display_.getCursorX(), TFT_display_.getCursorY(), &x, &y, &w, &h);
  TFT_display_.fillRect(x, y, w, h, ST77XX_BLACK);
  TFT_display_.println(text);
}

/*
 * The Char buffer should be at least 7 elements in size
 */
void TFT_parameterToChars(float param, char* buf)
{
  if ( abs(param - TFT_Error::NO_DATA) < 0.1 ) { // No data error
    toChars("No Dt ", buf, 6);
  } else if ( abs(param - TFT_Error::UNSTABLE) < 0.1 ) { // Unstable error
    toChars("Unst  ", buf, 6);
  } else if ( abs(param - TFT_Error::PROCESSING) < 0.1 ) { // Wait
    toChars("Wait  ", buf, 6);
  } else {
    dtostrf(param, 6, 1, buf);
  }
  buf[6] = '\0';
}