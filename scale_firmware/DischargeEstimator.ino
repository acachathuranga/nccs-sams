#include "DischargeEstimator.hpp"

// Constants & Definitions
const int DE_RAW_BUFFER_SIZE = 10;
const int DE_FILTERED_BUFFER_SIZE = 10;
const int DE_SAMPLE_BUFFER_SIZE = 10;
const float DE_STABLE_MAX_VARIANCE = 5.0; // grams
const int DE_TIMEOUT = 3000; // milliseconds
const int DE_RUN_INTERVAL = 1000; // milliseconds
const int DE_MIN_SAMPLES = 10;
DE_ValueState DE_rawBufferAverage(float &weight);

// Variables
float DE_raw_buffer_[DE_RAW_BUFFER_SIZE] = {0.0};
int DE_raw_buffer_index_ = 0;
unsigned long int DE_raw_buffer_update_time_ = 0;

float DE_sample_buffer_[DE_SAMPLE_BUFFER_SIZE] = {0.0};
bool DE_sample_buffer_complete_ = false;
int DE_sample_buffer_index_ = 0;

unsigned long int DE_last_run_time_ = 0;
DE_State DE_state_ = DE_State::UNSTABLE;
float DE_tare_weight_ = 0.0;
float DE_volume_ = 0.0;
float DE_flow_rate_ = 0.0;

void DE_init()
{
  
}

void DE_run()
{
  if ( (millis() - DE_last_run_time_) > DE_RUN_INTERVAL )
  {
    float weight;
    DisplayParameters param = TFT_getParameters();
    DE_ValueState value_state = DE_rawBufferAverage(weight);
    
    if (value_state == DE_ValueState::TIMEOUT) {
      param.volume_ml = TFT_Error::NO_DATA;
      param.flow_rate_mlps = TFT_Error::NO_DATA;
      DE_state_ = DE_State::UNSTABLE; // Reset State if data connection is lost
    } else if (value_state == DE_ValueState::UNSTABLE) {
      param.volume_ml = TFT_Error::UNSTABLE;
      param.flow_rate_mlps = TFT_Error::UNSTABLE;
      DE_state_ = DE_State::UNSTABLE; // Set state to unstable
    }
    else
    {
      DE_sampleBufferInsert(weight);
    }
    
    
    switch(DE_state_)
    {
      case DE_State::UNSTABLE: {
        if ( (value_state != DE_ValueState::TIMEOUT) && (value_state != DE_ValueState::UNSTABLE) ) {
          DE_state_ = DE_State::INITIALIZING;
          param.volume_ml = TFT_Error::PROCESSING;
          param.flow_rate_mlps = TFT_Error::PROCESSING;
          DE_sampleBufferClear();
        }
        break;
      }
      
      case DE_State::INITIALIZING: {
        float flow_rate;
        if (DE_getFlowRate(flow_rate) == DE_ValueState::STABLE) {
          Serial.print("Init Flow: ");
          Serial.println(flow_rate);
          if (flow_rate >= -0.2) {
            // Set Tare Weight
            DE_tare_weight_ = weight - DE_volume_;
            Serial.print("Tare Weight: ");
            Serial.println(DE_tare_weight_);
            DE_state_ = DE_State::STABLE;
          }
        }
        break;
      }
      
      case DE_State::STABLE: {
        if ( abs(weight - DE_tare_weight_ - DE_volume_) > 5.0 ) { // Sudden variation detected
          param.volume_ml = TFT_Error::UNSTABLE;
          param.flow_rate_mlps = TFT_Error::UNSTABLE;
          DE_state_ = DE_State::UNSTABLE; // Set state to unstable
        } else {
          float flow_rate;
          DE_getFlowRate(flow_rate);
          DE_volume_ = weight - DE_tare_weight_;
          param.volume_ml = DE_volume_;
          param.flow_rate_mlps = flow_rate;
          Serial.print("Volume: ");
          Serial.print(DE_volume_);
          Serial.print("   Flow: ");
          Serial.println(flow_rate);
          
          WiFi_sendFlowRate(param.flow_rate_mlps);
          WiFi_sendVolume(param.volume_ml);
          WiFi_sendTime(SecondsToHMS(param.time));
          WiFi_sendPercentageFill(param.volume_ml * 100 / container_volumes[param.container_type]);
          WiFi_sendContainerType(container_volumes[param.container_type]);
        }
        break;
      }
      
      default: DE_State::UNSTABLE;
    }
    
    TFT_setParameters(param);
    DE_last_run_time_ = millis();
  }
}

DE_ValueState DE_getFlowRate(float &value)
{
  if ( (!DE_sample_buffer_complete_) && (DE_sample_buffer_index_ < DE_MIN_SAMPLES) ) 
    return DE_ValueState::UNSTABLE;
    
  int index = DE_sample_buffer_index_ - 1;
  index = (index < 0) ? DE_SAMPLE_BUFFER_SIZE - 1 : index;
  
  int element_count = (DE_sample_buffer_complete_) ? DE_SAMPLE_BUFFER_SIZE : DE_sample_buffer_index_; 
  float sigma_step_weight = 0.0;
  float sigma_step = 0.0;
  float sigma_weight = 0.0;
  float sigma_step_sqrd = 0.0;
  
  
  for (int element = 0; element < element_count; element++)
  {
    sigma_step_weight += (element_count - element) * DE_sample_buffer_[index];
    sigma_step += (element_count - element);
    sigma_weight += DE_sample_buffer_[index];
    sigma_step_sqrd += pow(element_count - element, 2);
    
    index--;
    index = (index < 0) ? DE_SAMPLE_BUFFER_SIZE - 1 : index;
  }
  
  value = (element_count * sigma_step_weight - sigma_step * sigma_weight) / (element_count * sigma_step_sqrd - pow(sigma_step, 2));
  return DE_ValueState::STABLE;
}

void DE_sampleBufferClear()
{
  DE_sample_buffer_complete_ = false;
  DE_sample_buffer_index_ = 0;
  for (float &value : DE_sample_buffer_) {
    value = 0.0;
  }
}

void DE_sampleBufferInsert(float weight)
{
  DE_sample_buffer_[DE_sample_buffer_index_] = weight;
  DE_sample_buffer_index_++;
  if (DE_sample_buffer_index_ >= DE_SAMPLE_BUFFER_SIZE) {
    DE_sample_buffer_complete_ = true;
    DE_sample_buffer_index_ = DE_sample_buffer_index_ % DE_SAMPLE_BUFFER_SIZE;
  }
}

void DE_rawBufferInsert(float weight)
{
  DE_raw_buffer_[DE_raw_buffer_index_] = weight;
  DE_raw_buffer_index_++;
  if (DE_raw_buffer_index_ >= DE_RAW_BUFFER_SIZE)
  {
    DE_raw_buffer_index_ = DE_raw_buffer_index_ % DE_RAW_BUFFER_SIZE;
    DE_raw_buffer_update_time_ = millis();
  }
}

DE_ValueState DE_rawBufferAverage(float &weight)
{
  if ( (millis() - DE_raw_buffer_update_time_) > DE_TIMEOUT) {
    return DE_ValueState::TIMEOUT;
  } 
  
  float prev_val = DE_raw_buffer_[0];
  float variance = 0.0;
  float total = 0.0;
  for (float value :  DE_raw_buffer_)
  {
    total += value;
    variance += abs(value - prev_val);
    prev_val = value;
  }
  
  variance /= DE_RAW_BUFFER_SIZE;
  if (variance > DE_STABLE_MAX_VARIANCE) {
    return DE_ValueState::UNSTABLE;
  } else {
    weight = total / DE_RAW_BUFFER_SIZE;
    return DE_ValueState::STABLE;
  }
}

void DE_weightCallback(float weight)
{
  DE_rawBufferInsert(weight);
}