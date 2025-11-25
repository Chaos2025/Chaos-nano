#include "stm8l15x.h"

#include "task.h"
#include "time.h"

#include "handle.h"

static bool flag = false;
static uint16_t LowTvoc=0x0CF0;
static uint16_t eCO2PPM;
static uint16_t Tvocmg_L;

#define DELAY_MS        1000  

#define A       7
#define TVOC_MIN        450

static unsigned int Get_PPM_RC(unsigned int dv)
{
  static unsigned int LastADC = TVOC_MIN;
  unsigned int NowADC = dv;
  
  LastADC=((10 - A) * NowADC + A * LastADC) / 10;
  return LastADC; 
  
}

void Get_Tvoc_1(void)
{
  FunctionalState FlagCalibration =DISABLE;
  uint16_t   TempTvoc;
  
  ADC_ChannelCmd (ADC1,ADC_Channel_4,DISABLE);
  ADC_ChannelCmd (ADC1,ADC_Channel_18,ENABLE);
  ADC_SoftwareStartConv (ADC1);
  while(!ADC_GetFlagStatus (ADC1,ADC_FLAG_EOC));
  ADC_ClearFlag (ADC1,ADC_FLAG_EOC);
  uint16_t TvocVoltage=ADC_GetConversionValue (ADC1);
  
  if(FlagCalibration == ENABLE)
  {
    LowTvoc=TvocVoltage;
    FlagCalibration =DISABLE;
  }
  
  if(TvocVoltage>=LowTvoc)
  {
    TempTvoc = (TvocVoltage-LowTvoc)*2 + TVOC_MIN;
  }
  else
  {
    LowTvoc = TvocVoltage;
    TempTvoc = TVOC_MIN;
  }
  
  eCO2PPM = Get_PPM_RC(TempTvoc);
  Tvocmg_L = eCO2PPM * 7 / 100 - 29;
}

void Get_Tvoc(void)
{
  if(flag){
    flag = false;
    Get_Tvoc_1();
    update_disp(DISP_UPDATE_TVOC);
    time_create(TASK_ID_TVOC, DELAY_MS, &flag, true);
  }
}

void TVOC_Init(void)
{
  flag = false;
  eCO2PPM = 0;
  Tvocmg_L = 0;
  time_create(TASK_ID_TVOC, 5000, &flag, true);
}

uint16_t get_tvoc(void)
{
  return Tvocmg_L;
}

uint16_t get_eco2(void)
{
  return eCO2PPM;
}