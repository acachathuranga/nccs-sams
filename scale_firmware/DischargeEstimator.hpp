#ifndef DISCHARGE_ESTIMATOR_H
#define DISCHARGE_ESTIMATOR_H

enum class DE_ValueState
{
  STABLE,
  UNSTABLE,
  TIMEOUT
};

enum class DE_State
{
  UNSTABLE,
  INITIALIZING,
  STABLE
};

#endif // DISCHARGE_ESTIMATOR_H