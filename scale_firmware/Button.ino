// Variables
typedef void (*ButtonCallbackEvent)(int, int);
ButtonCallbackEvent Button_callback_;
int Button_pin;
volatile unsigned long Button_last_on_time = 0;
volatile unsigned long Button_last_off_time = 0;
volatile int Button_short_press_count = 0;
volatile int Button_long_press_count = 0;

volatile bool Button_state = LOW;

// Parameters
const int Button_JITTER_WINDOW = 200;  // Milliseconds
const int Button_PRESS_TIMEOUT = 500; // Milliseconds
const int Button_LONG_PRESS_TIMEOUT = 2000; // Milliseconds

void Button_callback(int short_press_count, int long_press_count)
{
  if (Button_callback_ != NULL)
  {
    Button_callback_(short_press_count, long_press_count);
  }
}

void Button_input()
{
  bool Button_pressed = !digitalRead(Button_pin);

  if (Button_state == HIGH)
  {
    if (Button_pressed)
    {
    }
    else
    {
      Button_state = LOW;
      if ((millis() - Button_last_on_time) > Button_LONG_PRESS_TIMEOUT)
      {
        Button_long_press_count++;
      }
      else
      {
        Button_short_press_count++;
      }
      Button_last_off_time = millis();
    }
  }
  else
  {
    if (Button_pressed)
    {
      if ((millis() - Button_last_off_time) > Button_JITTER_WINDOW)
      {
        Button_state = HIGH;
        Button_last_on_time = millis();
      }
    }
  }
}

void Button_init(const int INPUT_PIN, void (*CallbackEvent)(int, int))
{
  Button_pin = INPUT_PIN;  
  Button_callback_ = CallbackEvent;

  pinMode(Button_pin, INPUT_PULLUP);
}


void Button_run()
{
  Button_input();
  if ((Button_short_press_count > 0 || Button_long_press_count > 0) && (Button_state == LOW))
  {
    if (((millis() - Button_last_off_time) > Button_PRESS_TIMEOUT) && (millis() - Button_last_on_time > Button_PRESS_TIMEOUT))
    {
      Button_callback(Button_short_press_count, Button_long_press_count);
      // Reset Press Counts
      Button_short_press_count = 0;
      Button_long_press_count = 0;
    }
  }
}
