#include <Arduino.h>
#include <M5StickCPlus.h>

#include "MutualM5.h"

MutualM5::MutualM5()
{
  xMutex_Wire1 = xSemaphoreCreateMutex();
}

void MutualM5::lockWire1()
{
  while (xSemaphoreTake(xMutex_Wire1, 10 / portTICK_PERIOD_MS) != pdTRUE)
  {
    delay(1);
  }
}

void MutualM5::unlockWire1()
{
  xSemaphoreGive(xMutex_Wire1);
}

void MutualM5::ScreenBreath(uint8_t brightness)
{
  lockWire1();
  M5.Axp.ScreenBreath(brightness);
  unlockWire1();
}

float MutualM5::GetBatVoltage()
{
  lockWire1();
  float vbat = M5.Axp.GetBatVoltage();
  unlockWire1();
  return vbat;
}

uint8_t MutualM5::GetBtnPress()
{
  lockWire1();
  uint8_t res = M5.Axp.GetBtnPress();
  unlockWire1();
  return res;
}

void MutualM5::SetDateAndTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  lockWire1();
  M5.Rtc.SetData(date);
  M5.Rtc.SetTime(time);
  unlockWire1();
}

void MutualM5::GetDateAndTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  lockWire1();
  M5.Rtc.GetData(date);
  M5.Rtc.GetTime(time);
  unlockWire1();
}

MutualM5 M5_Safe;
