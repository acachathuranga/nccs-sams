#ifndef TFT_H
#define TFT_H

enum class TFT_Screen
{
  STARTUP_SCREEN,
  SELECTION_SCREEN,
  INFO_SCREEN,
  SHUTDOWN_SCREEN
};

class TFT_Error
{
  public:
    static constexpr float NO_DATA = -1.0;
    static constexpr float UNSTABLE = -2.0;
    static constexpr float PROCESSING = -3.0;
};

typedef struct DisplayParameters
{
  bool network_available = false;
  float volume_ml = 0.0;
  float flow_rate_mlps = 0.0;
  int container_type = 0;
  int battery = 0;            // -1 Supply Connected. 
  unsigned long int time = 0;
};

DisplayParameters TFT_getParameters(void);
void TFT_setParameters(DisplayParameters param);
void TFT_showScreen(TFT_Screen screen);

#endif // TFT_H