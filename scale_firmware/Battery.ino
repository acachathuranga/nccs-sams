// Definitions and Constants
#define BATTERY_ENABLE_PIN 7
#define BATTERY_VOLTAGE_PIN 1
#define SUPPLY_VOLTAGE_PIN 2

// Variables
float Battery_soc_[21] = {3.27, 3.61, 3.69, 3.71, 3.73, 3.75, 3.77, 3.79, 3.8, 3.82, 3.84, 3.85, 3.87, 3.91, 3.95, 3.98, 4.02, 4.08, 4.11, 4.15, 4.2};

void Battery_init()
{
  pinMode(BATTERY_ENABLE_PIN, OUTPUT);
  Battery_enable();
}

void Battery_run()
{
  // Serial.print("Bat: ");
  // Serial.print(Battery_voltage());
  // Serial.print("  Vs: ");
  // Serial.println(Battery_supplyVoltage());
  DisplayParameters param = TFT_getParameters();
  
  float battery_voltage = Battery_voltage();
  if (Battery_supplyVoltage() > (battery_voltage + 0.5)) {
    param.battery = -1;
  } else {
    int soc_id = 1;
    for (; soc_id < 21; soc_id++ ) {
      if (Battery_soc_[soc_id] > battery_voltage) break;
    }
    param.battery = (soc_id - 1) * 5;
  }
  TFT_setParameters(param);
}

void Battery_enable()
{
  digitalWrite(BATTERY_ENABLE_PIN, HIGH);
}

void Battery_disable()
{
  digitalWrite(BATTERY_ENABLE_PIN, LOW);
}

float Battery_voltage()
{
  return (float)analogRead(BATTERY_VOLTAGE_PIN) / 1024.0 * 3.3 * 2.0;
}

float Battery_supplyVoltage()
{
  return (float)analogRead(SUPPLY_VOLTAGE_PIN) / 1024.0 * 3.3 * 2.0;
}