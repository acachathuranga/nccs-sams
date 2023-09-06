#include "AnD_Scale.hpp"

// Constants & Definitions
const int SCALE_SERIAL_BAUD_RATE = 2400;
Scale_DataType Scale_decodeHeader(char header[]);
void Scale_processData(ScaleData data);

// Variables
typedef void (*ANDScaleCallback)(float);
Scale_PacketState scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
ScaleData scale_data_;
char scale_header_data_[2] = {'\0'};
char scale_packet_data_[9];
int scale_data_idx_ = 0;

ANDScaleCallback stableWeighingDataCallback_;
ANDScaleCallback stableCountingDataCallback_;
ANDScaleCallback unstableWeighingDataCallback_;
ANDScaleCallback outOfRangeCallback_;

void Scale_setZero()
{
  char packet[3];
  packet[0] = 'P';
  packet[1] = 13; // CR
  packet[2] = 10; // LF
  Serial1.print(packet);
}

void Scale_tare()
{
  char packet[3];
  packet[0] = 'Z';
  packet[1] = 13; // CR
  packet[2] = 10; // LF
  Serial1.print(packet);
}

void Scale_init()
{
  // initialize serial:
  Serial1.begin(SCALE_SERIAL_BAUD_RATE, SERIAL_7E1);
}

void Scale_registerStableWeighingDataCallback(void(*callback)(float))
{
  stableWeighingDataCallback_ = callback;
}

void Scale_registerStableCountingDataCallback(void(*callback)(float))
{
  stableCountingDataCallback_ = callback;
}

void Scale_registerUnstableWeighingDataCallback(void(*callback)(float))
{
  unstableWeighingDataCallback_ = callback;
}

void Scale_registerOutOfRangeCallback(void(*callback)(float))
{
  outOfRangeCallback_ = callback;
}

void Scale_processData(ScaleData data)
{
  switch (data.type)
  {
    case Scale_DataType::STABLE_WEIGHING_DATA: Scale_callback(stableWeighingDataCallback_, data.value); break;
    case Scale_DataType::STABLE_COUNTING_DATA: Scale_callback(stableCountingDataCallback_, data.value); break;
    case Scale_DataType::UNSTABLE_WEIGHING_DATA: Scale_callback(unstableWeighingDataCallback_, data.value); break;
    case Scale_DataType::OUT_OF_RANGE_DATA: Scale_callback(outOfRangeCallback_, data.value); break;
    default:;
  }
}

void Scale_callback(void(*callback)(float), float data)
{
  if (callback != NULL)
  {
    callback(data);
  }
}

Scale_DataType Scale_decodeHeader(char header[])
{
  if ((header[0] == 'S') &&(header[1] == 'T')) return Scale_DataType::STABLE_WEIGHING_DATA;
  if ((header[0] == 'Q') &&(header[1] == 'T')) return Scale_DataType::STABLE_COUNTING_DATA;
  if ((header[0] == 'U') &&(header[1] == 'S')) return Scale_DataType::UNSTABLE_WEIGHING_DATA;
  if ((header[0] == 'O') &&(header[1] == 'L')) return Scale_DataType::OUT_OF_RANGE_DATA;
  return Scale_DataType::INVALID_DATA;
}

void Scale_packetDecoder(int rx)
{
  switch(scale_packet_state_)
  {
    case Scale_PacketState::WAITING_FOR_PKT: {  
      // Cyclic header buffer
      scale_header_data_[0] = scale_header_data_[1];
      scale_header_data_[1] = rx;
      
      scale_data_.type = (Scale_DataType)Scale_decodeHeader(scale_header_data_);
      if (scale_data_.type != Scale_DataType::INVALID_DATA) {
        scale_packet_state_ = Scale_PacketState::HEADER_RECEIVED;
      }
      break;
    }
    
    case Scale_PacketState::HEADER_RECEIVED: {
      if (rx == ',') {
        scale_packet_state_ = Scale_PacketState::RECEIVING_DATA;
        scale_data_idx_ = 0;
      } else {
        scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
      }
      break;
    }

    case Scale_PacketState::RECEIVING_DATA: {
      scale_packet_data_[scale_data_idx_] = rx;
      scale_data_idx_++;
      if (scale_data_idx_ == 9) {
        scale_packet_state_ = Scale_PacketState::DATA_RECEIVED;
      }
      break;
    }

    case Scale_PacketState::DATA_RECEIVED: {
      if (rx == ' ') {
        scale_packet_state_ = Scale_PacketState::TL_BT1_RECEIVED;
      } else {
        scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
      }
      break;
    }

    case Scale_PacketState::TL_BT1_RECEIVED: {
      if (rx == ' ') {
        scale_packet_state_ = Scale_PacketState::TL_BT2_RECEIVED;
      } else {
        scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
      }
      break;
    }

    case Scale_PacketState::TL_BT2_RECEIVED: {
      if (rx == 'g') {
        // Packet Received successfully
        scale_data_.value = atof(scale_packet_data_);
        Scale_processData(scale_data_);
      }
      scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
      break;
    }

    default: {
      scale_packet_state_ = Scale_PacketState::WAITING_FOR_PKT;
    }
  }  
}

void Scale_run()
{
  while(Serial1.available())
  {
    Scale_packetDecoder(Serial1.read());
  }
}
