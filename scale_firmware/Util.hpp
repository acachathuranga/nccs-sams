#ifndef UTIL_H
#define UTIL_H

/*
  time should be at least 9 elements in size
*/
void SecondsToHMS(unsigned long int time_stamp, char* time)
{
  int seconds, minutes, hours;
  seconds = time_stamp%60;
  time_stamp /= 60;
  minutes = time_stamp%60;
  hours = time_stamp/60;
  
  // Int to Char ASCII convertion +48
  time[0] = hours/10 + 48;
  time[1] = hours%10 + 48;
  time[2] = ':';
  time[3] = minutes/10 + 48;
  time[4] = minutes%10 + 48;
  time[5] = ':';
  time[6] = seconds/10 + 48;
  time[7] = seconds%10 + 48;
  time[8] = '\0';
}

String SecondsToHMS(unsigned long int time_stamp)
{
  char time[8];
  SecondsToHMS(time_stamp, time);
  return String(time);
}

void toChars(String string, char *buf, int len)
{
  for (int i = 0; i < len; i++)
  {
    buf[i] = string.charAt(i);
  }
}

#endif // UTIL_H