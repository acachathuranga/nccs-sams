#ifndef AND_SCALE_H
#define AND_SCALE_H

enum class Scale_PacketState
{
  WAITING_FOR_PKT,
  HEADER_RECEIVED,
  SEPERATOR_RECEIVED,
  RECEIVING_DATA,
  DATA_RECEIVED,
  TL_BT1_RECEIVED,
  TL_BT2_RECEIVED
};

enum class Scale_DataType : uint8_t 
{
  STABLE_WEIGHING_DATA,
  STABLE_COUNTING_DATA,
  UNSTABLE_WEIGHING_DATA,
  OUT_OF_RANGE_DATA,
  INVALID_DATA
};

typedef struct ScaleData
{
  Scale_DataType type;
  float value;  
};

#endif //AND_SCALE_H
