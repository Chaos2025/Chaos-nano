#include "bat.h"
#include "pin.h"

#include "time.h"
#include "oled.h"
#include "handle.h"

#define LowBat          0x03DE   //990
#define batlevel1       0x41A
#define batlevel2       0x44C
#define batlevel3       0X47e
#define FullBat         0x04B0    //1100
#define RangeBat        (FullBat - LowBat)

#define DELAY_MS        2000 

static u8       Bat_cha_sta;   //0 未充电，1，在充，2 充满了
static u8       Bat_Level;
static bool flag = false;

void BAT_Init(void)
{
  Bat_cha_sta=0;
  Bat_Level = 0;
  
  flag = false;
  time_create(TASK_ID_BAT, DELAY_MS, &flag, true);
}

static void Check_Charge_Bat_1(void)
{
  uint16_t   BatVoltage;
  
  //检测USB给电池充电
  if((GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,CHGSTAT_GPIO_PINS)==RESET) && 
     (GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,PGSTAT_GPIO_PINS)!=RESET)) {  //读GPA2,3输入状态，充电中
       
    delayUs(0x3FFF);  //软件防抖
    if((GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,CHGSTAT_GPIO_PINS)==RESET) && 
       (GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,PGSTAT_GPIO_PINS)!=RESET)) {//读GPA2,3输入状态

      Bat_cha_sta=1;
    }
  } else if((GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,CHGSTAT_GPIO_PINS)!=RESET) && 
          (GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,PGSTAT_GPIO_PINS)==RESET)) {//读GPA2,3输入状态，充电完成

    delayUs(0x3FFF);  //软件防抖
    if((GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,CHGSTAT_GPIO_PINS)!=RESET) && 
       (GPIO_ReadInputDataBit(CHARGE_GPIO_PORT,PGSTAT_GPIO_PINS)==RESET))//读GPA2,3输入状态
      Bat_cha_sta=2;
  } else
    Bat_cha_sta=0;
  
  //检测电池电量
  ADC_ChannelCmd (ADC1,ADC_Channel_18,DISABLE);//ADC_Channel_18
  ADC_ChannelCmd (ADC1,ADC_Channel_4,ENABLE);
  delayUs(0x3FFF);
  ADC_SoftwareStartConv (ADC1);
  while(!ADC_GetFlagStatus (ADC1,ADC_FLAG_EOC));
  ADC_ClearFlag (ADC1,ADC_FLAG_EOC);
  BatVoltage = ADC_GetConversionValue (ADC1);
  
  if((BatVoltage<=FullBat)&&(BatVoltage>=LowBat)){ 
    if((BatVoltage>LowBat)&&(BatVoltage<=batlevel1))
    {Bat_Level=25;}
    if((BatVoltage>batlevel1)&&(BatVoltage<=batlevel2))
    {Bat_Level=50;}
    if((BatVoltage>batlevel2)&&(BatVoltage<=batlevel3))
    {Bat_Level=75;}
  } else if(BatVoltage>FullBat) {
    Bat_Level=100;
  } else {
    Bat_Level=0;
  }
}

void Check_Charge_Bat(void)
{
  if(flag){
    flag = false;
    Check_Charge_Bat_1();
    update_disp(DISP_UPDATE_BAT);
    time_create(TASK_ID_BAT, DELAY_MS, &flag, true);
  }
}


u8 getBatStatus(void)
{
  return Bat_cha_sta;
}

u8 getBatLevel(void)
{
  return Bat_Level;
}