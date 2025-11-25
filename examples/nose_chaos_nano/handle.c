#include "stm8l15x.h"
#include "pin.h"

#include "task.h"
#include "time.h"

#include "oled.h"
#include "handle.h"
#include "stm8_i2c_sht21.h"
#include "bat.h"
#include "tvoc.h"

static enum disp_update_t curDispPageNumber = DISP_UPDATE_POWER_OFF;
static bool powerOn = false;

void powerOnHandle(void)
{
  static bool flag = false;
  
  if(!powerOn){
    if(!flag){
      if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)!=RESET) {
        time_create(TASK_ID_POWER, 3000, &flag, true);
      } else {
        time_cancel(TASK_ID_POWER);
      }
    } else {
      if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)!=RESET) {
        powerOn = true;
        GPIO_ResetBits(GPIOD, GPIO_Pin_0); //开加热
        GPIO_SetBits(GPIOA, GPIO_Pin_2);   //开电源
        curDispPageNumber = DISP_UPDATE_SHT21;
        update_disp(curDispPageNumber);
      }
    }
  } else {
    if(!flag){
      if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)!=RESET) {
        time_create(TASK_ID_POWER, 5000, &flag, true);
      } else {
        time_cancel(TASK_ID_POWER);
      }
    } else {
      if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)!=RESET) {
        powerOn = false;
        GPIO_SetBits(GPIOD, GPIO_Pin_0);          //关加热
        GPIO_ResetBits(GPIOA, GPIO_Pin_2);        //关电源
        curDispPageNumber = DISP_UPDATE_POWER_OFF;
        update_disp(curDispPageNumber);
      }
    }
  }
  
  task_setBlock(TASK_ID_POWER);
  flag = false;
  
  return;
}

void keyHandle(void)
{
  if(!powerOn)
    return;
  
  curDispPageNumber = (curDispPageNumber == DISP_UPDATE_SHT21) ? DISP_UPDATE_TVOC : DISP_UPDATE_SHT21;
  OLED_CLS();
  update_disp(DISP_UPDATE_BAT);
  update_disp(curDispPageNumber);
  
  task_setBlock(TASK_ID_KEY);
}

static void Dis_Bat(bool powerOn)
{
  u8 Bat_cha_sta = getBatStatus();
  u8 Bat_Level = getBatLevel();
  
  bat_pic(0,100, pow_bat[Bat_Level/23]);//23
  if(Bat_cha_sta) //雷电
  {
    write_number(0,90,12,0);
  } else if(Bat_cha_sta==0) //雷电
  {
    //清雷电
    write_number(0,90,10,0);
  }
}

void update_disp(enum disp_update_t disp_update_number)
{
  switch(disp_update_number){
  case DISP_UPDATE_SHT21:
    {
      if(disp_update_number != curDispPageNumber)
        return;
      
      // temperature
      uint16_t SHT21_T = get_temperature();
      uint16_t SHT21_RH = get_humidity();
      
      hz_disp1216_2(3,0,temp_ch);
      if(SHT21_T>=1000)
      {
        //write_number_1632(2,30,numb[SHT21_T/1000],0);
        write_number(3,50,SHT21_T/1000,0);
        
      }else{
        //write_number_1632_clr(2,30,0);
        write_number(3,50,10,0);
      }
      //write_number_1632(2,50,numb[SHT21_T/100%10],0);
      write_number(3,60,SHT21_T/100%10,0);
      write_number(3,70,11,0);
      write_number(3,80,SHT21_T/10%10,0);
      //write_number_1632(2,80,numb[SHT21_T/10%10],0);
      hz_disp8_16_4(3,90,Unit_C);
      
      // humidity
      hz_disp1216_2(6,0,hum_ch);
      if(SHT21_RH>=1000)
      {
        //write_number_1632(2,30,numb[SHT21_RH/1000],0);
        write_number(6,50,SHT21_RH/1000, 0);
      }
      else
      {
        //write_number_1632_clr(2,30,0);
        write_number(6,50,10, 0);
      }
      //write_number_1632(2,50,numb[SHT21_RH/100%10],0);
      write_number(6,60,SHT21_RH/100%10,0);
      write_number(6,70,11,0);
      write_number(6,80,SHT21_RH%10,0);
      //write_number_1632(2,80,numb[SHT21_RH%10],0);
      hz_disp8_16_4(6,90,My_Unit_cc);
    }
    break;
    
  case DISP_UPDATE_TVOC:
    {
      uint16_t Tvocmg_L = 0;
      uint16_t eCO2PPM = 0;
      
      if(disp_update_number != curDispPageNumber)
        return;
      
      // 甲醛
      //      Tvocmg_L = get_tvoc();
      //      eCO2PPM = get_eco2();
      
      //hz_disp8_16_4(6,0,TVOC);
      hz_disp1216_2(3,0,TVOC);
      if(Tvocmg_L<100)
      {
        //write_number_1632(2,30,numb[0],0);
        write_number(3,40,0,0);
      }else{
        //write_number_1632(2,30,numb[1],0);
        write_number(3,40,1,0);
      }
      write_number(3,50,11,0);
      write_number(3,60,Tvocmg_L/10%100,0);
      write_number(3,70,Tvocmg_L%10,0);
      //write_number_1632(2,60,numb[Tvocmg_L/10%10],0);
      //write_number_1632(2,80,numb[Tvocmg_L%10],0);
      hz_disp8_16_4(3,90,Unit_mg_l);
      
      // 空气质量
      hz_disp1216_5(6,0,air_quality);
      if(eCO2PPM>10449)
        eCO2PPM = 10449;
      
      //eCO2PPM = eCO2PPM - 450;
      
      if(eCO2PPM <(150 + 450))
        // eco2flag = 0;
        hz_disp8_16_4(6,95,My_you);
      else if(eCO2PPM < (300 + 450) )
        //eco2flag = 1;
        hz_disp8_16_4(6,95,My_liang);
      else
        //eco2flag = 2;
        hz_disp8_16_4(6,95,My_cha); 
    }
    break;
    
  case DISP_UPDATE_BAT:
    {
      if( (curDispPageNumber == DISP_UPDATE_SHT21) || (curDispPageNumber == DISP_UPDATE_TVOC) ){
        Dis_Bat(true);
      } else {
        Dis_Bat(false);
      }
    }
    break;
    
  case DISP_UPDATE_POWER_OFF:
    {
      OLED_CLS();
      Dis_Bat(true);
    }
    break;
  }
}