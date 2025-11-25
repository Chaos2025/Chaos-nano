/*******************************************************************************
*******************************蓝牙电子鼻系统***********************************
******************************http://ed-l.com***********************************
********************************************************************************
******作者:宋利华
********QQ:277421000
****版本号:V1.0 (2016-01-21)
*******************************************************************************/

#include "stm8l15x.h"
#include "stm8l15x_it.h"
#include "stm8l15x_gpio.h"
#include "stm8l15x_tim4.h"

#include "pin.h"

#include "stm8_i2c_sht21.h"
#include "oled.h"
#include "tvoc.h"
#include "bat.h"
#include "handle.h"

/*

#include "key.h"
*/

/* Private typedef -----------------------------------------------------------*/
extern void start_kernel();

void board_init(void);

/* Private define ------------------------------------------------------------*/


/*******************************************************************************
****入口参数：无
****出口参数：无
****函数备注：PC5 TX,PC6 RX
****版权信息：ed-l.com  for Madison
*******************************************************************************/
int main( void )
{  
 //oard_init();
  
  start_kernel();
}

void SysCLKInit()//16M
{
  /* Select HSI as system clock source */
//  CLK_SYSCLKSourceSwitchCmd(ENABLE);
//  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
  /* High speed internal clock prescaler: 1*/
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
//  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
//  {}
}
      
/*****************************************************************************/
/*****************************************************************************/
/*******************************************************************************
****入口参数：无
****出口参数：无
****函数备注：外设初始化函数
*******************************************************************************/
void Periph_Init()
{
   //传感器加热输出管脚
   GPIO_Init(HEATCTL_GPIO_PORT, HEATCTL_GPIO_PINS, GPIO_Mode_Out_PP_Low_Slow);                 //VOC传感器加热控制，GPIO输出
   
   //OLED输出管脚
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, OLED_SPI_SCK_GPIO_PINS , GPIO_Mode_Out_PP_Low_Slow);   //OLED 管脚 ,SPI_D/C，GPIO输出
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, OLED_SPI_MOSI_GPIO_PINS, GPIO_Mode_Out_PP_Low_Slow);   //OLED 管脚,SPI_RST，GPIO输出
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, OLED_SPI_MISO_GPIO_PINS, GPIO_Mode_Out_PP_Low_Slow);   //OLED 管脚,SPI_SDA，GPIO输出
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, OLED_SPI_NSS_GPIO_PINS, GPIO_Mode_Out_PP_Low_Slow);    //OLED 管脚，SPI_SCK,GPIO输出
   
   //输入管脚
   GPIO_Init(CHARGE_GPIO_PORT , CHGSTAT_GPIO_PINS  ,GPIO_Mode_In_PU_No_IT);                     //电池充电进行指示，GPIO输入
   GPIO_Init(CHARGE_GPIO_PORT , PGSTAT_GPIO_PINS ,GPIO_Mode_In_PU_No_IT);                       //电池充满指示，GPIO输入

   GPIO_Init(KEY_POWER_GPIO_PORT, CTL_POWER_GPIO_PINS, GPIO_Mode_Out_PP_Low_Slow);      //电源软开关输出控制，GPIO输出
   GPIO_Init(KEY_POWER_GPIO_PORT, KEY_POWER_GPIO_PINS , GPIO_Mode_In_FL_IT);            //电源软开关按键检测，中断输入
   
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, KEY_DIS_GPIO_PINS , GPIO_Mode_In_FL_IT);              //切屏按键检测，中断输入
   
   GPIO_Init(KEY_CTL_ADC_OLED_GPIO_PORT, ADC_SENSOR_GPIO_PINS  ,GPIO_Mode_In_FL_No_IT);        //VOC输入检测，ADC功能
   GPIO_Init(BAT_GPIO_PORT , ADC_BAT_GPIO_PINS ,GPIO_Mode_In_FL_No_IT);                        //锂电池电量检测，ADC功能
   
   //预设端口状态
//  GPIO_SetBits(KEY_CTL_ADC_OLED_GPIO_PORT, CTL_POWER_GPIO_PINS);                          //开电源  
//  GPIO_ResetBits(HEATCTL_GPIO_PORT, HEATCTL_GPIO_PINS);                                   //开加热
   
   //2016-12-12
     GPIO_ResetBits(KEY_CTL_ADC_OLED_GPIO_PORT, CTL_POWER_GPIO_PINS);                          //关电源
     GPIO_SetBits(HEATCTL_GPIO_PORT, HEATCTL_GPIO_PINS);                                   //关加热
     GPIO_ResetBits(KEY_CTL_ADC_OLED_GPIO_PORT, OLED_SPI_MOSI_GPIO_PINS); //OLED RST
   
   //按键中断设置
   EXTI_DeInit ();                                                               //恢复中断的所有设置 
//   EXTI_SetPinSensitivity (EXTI_Pin_3,EXTI_Trigger_Rising_Falling);                     //外部中断3，上升沿触发和下升沿触发
//   EXTI_SetPinSensitivity (EXTI_Pin_1,EXTI_Trigger_Rising_Falling);                     //外部中断0，上升沿触发和下升沿触发
   EXTI_SetPinSensitivity (EXTI_Pin_3,EXTI_Trigger_Rising_Falling);                     //外部中断3，上升沿 & 下降沿触发
   EXTI_SetPinSensitivity (EXTI_Pin_1,EXTI_Trigger_Rising);                     //外部中断0，上升沿触发
  
  //2016-12-12
//  EXTI_SetPinSensitivity (EXTI_Pin_3,EXTI_Trigger_Falling); 
//  EXTI_SetPinSensitivity (EXTI_Pin_1,EXTI_Trigger_Falling);
   
   //串口外设初始化
   CLK_PeripheralClockConfig (CLK_Peripheral_USART1,ENABLE);//开启USART时钟
   USART_Init(USART1,19200,USART_WordLength_8b,USART_StopBits_1,USART_Parity_No,USART_Mode_Tx);//USART初始化，9600，8N1
   USART_Cmd (USART1,ENABLE);//使能USART
   
   CLK_PeripheralClockConfig (CLK_Peripheral_ADC1,ENABLE);  //开启ADC/USART时钟
   ADC_Init (ADC1,ADC_ConversionMode_Single,ADC_Resolution_12Bit,ADC_Prescaler_1);
   ADC_Cmd(ADC1,ENABLE);
   //ADC_ChannelCmd (ADC1,ADC_Channel_18,ENABLE);
   
    /* Enable I2C1 clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, ENABLE);
  
  
  /*定时器 计算加热时间*/
  /*
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);
  TIM3_SetCounter(0);  
  TIM3_TimeBaseInit(TIM3_Prescaler_128,TIM3_CounterMode_Up,0x3D09); //时钟频率=16M/128=125000  中断时间= 12500/12500=1s                                       
  TIM3_ARRPreloadConfig(ENABLE);
  TIM3_PrescalerConfig(TIM3_Prescaler_128,TIM3_PSCReloadMode_Update);
  TIM3_ITConfig(TIM3_IT_Update, ENABLE);
  TIM3_Cmd(ENABLE);
*/
  
  /* 定时用定时器（tim4），用于系统的延迟 */
  TIM4_DeInit();
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
  TIM4_SetCounter(0);
  TIM4_TimeBaseInit(TIM4_Prescaler_64, 250-1); // 64分频，16M/64/250 = 1ms
  TIM4_ARRPreloadConfig(ENABLE); // 使能自动重装
  TIM4_ITConfig(TIM4_IT_Update, ENABLE); // 数据更新中断
}

void board_init(void)
{
  SysCLKInit();
  Periph_Init();
  
  OLED_Init();
  
  enableInterrupts();
}

void device_on(void)
{
  SHT21_Init();
  TVOC_Init();
  BAT_Init();
  
  update_disp(DISP_UPDATE_POWER_OFF);
}

void sleep_cpu(void)
{
}

unsigned int enter_critical_section(void)
{
  disableInterrupts();
  return 0;
}

void exit_critical_section(unsigned int primask)
{
  enableInterrupts();
}

void start_timer(void)
{
  TIM4_Cmd(ENABLE);
}

void stop_timer(void)
{
    TIM4_Cmd(DISABLE);
}

void delayUs(__IO uint16_t us)
{
    /* Decrement nCount value */
    while (us != 0)
    {
        us--;
    }	
}
