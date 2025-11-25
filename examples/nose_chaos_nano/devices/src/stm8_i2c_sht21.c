/**
******************************************************************************
*     +-----------------------------------------------------------------+
*     |                        Pin assignment                           |
*     +---------------------------------------+-----------+-------------+
*     |  STM8 I2C Pins                       |  SHT21 |   Pin       |
*     +---------------------------------------+-----------+-------------+
*     | PC0_I2C_SDA_PIN/ SDA                 |   SDA     |    15        |
*     | PC1_I2C_SCL_PIN/ SCL                 |   SCL     |    16        |
*     +---------------------------------------+-----------+-------------+
******************************************************************************
******************************************************************************
*/ 
/* Includes ------------------------------------------------------------------*/
#include "stm8_i2c_sht21.h"

#include "pin.h"

#include "time.h"
#include "handle.h"

static uint16_t   SHT21_T=0;
static uint16_t   SHT21_RH=0;
static bool flag = false;
  
#define DELAY_MS        1000    
#define Dt      300  //7b
#define Dh      700  //7b

/**
* @brief  Initializes the SHT21_I2C..
* @param  None
* @retval None
*/
static inline void SHT21_LowLevel_Init(void)
{
  /*!< SHT21_I2C Periph clock enable */
  CLK_PeripheralClockConfig(SHT21_I2C_CLK, ENABLE);
}
/**
* @brief  Initializes the SHT21_I2C.
* @param  None
* @retval None
*/
void SHT21_Init(void)
{
  /* I2C DeInit */
  //SHT21_LowLevel_DeInit();
  SHT21_LowLevel_Init();
  
  /* I2C configuration */
  I2C_Init(SHT21_I2C, SHT21_I2C_SPEED, SHT21_ADDR, I2C_Mode_I2C,
           I2C_DutyCycle_2, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);
  
  /*!< SHT21_I2C Init */
  I2C_Cmd(SHT21_I2C, ENABLE);
  
  flag = false;
  time_create(TASK_ID_SHT21, DELAY_MS, &flag, true);
}

/**
* @brief  Read Temperature register of SHT21: double temperature value.
* @param  None
* @retval SHT21 measured temperature value.
*/
static I2C_Status SHT21_ReadTempHum(unsigned char command,uint8_t *read_Buffer)
{
  uint32_t I2C_Timeout= I2C_TIMEOUT;
  
  /* Enable SHT21_I2C acknowledgement if it is already disabled by other function */
  I2C_AcknowledgeConfig(SHT21_I2C, ENABLE);
  
  /*------------------------------------- Transmission Phase ------------------*/
  /* Send SHT21_I2C START condition */
  I2C_GenerateSTART(SHT21_I2C, ENABLE);
  
  /* Test on SHT21_I2C EV5 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_MODE_SELECT))  /* EV5 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  /* Send STSHT21 slave address for write */
  I2C_Send7bitAddress(SHT21_I2C, SHT21_ADDR, I2C_Direction_Transmitter);
  
  /* Test on SHT21_I2C EV6 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) /* EV6 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  
  /* Send the temperature register data pointer */
  I2C_SendData(SHT21_I2C, command);
  
  /* Test on SHT21_I2C EV8 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)) /* EV8 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  
  /*-------------------------------- Reception Phase --------------------------*/
  /* Send Re-STRAT condition */
  I2C_GenerateSTART(SHT21_I2C, ENABLE);
  
  /* Test on EV5 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_MODE_SELECT))  /* EV5 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  
  /* Send STSHT21 slave address for read */
  I2C_Send7bitAddress(SHT21_I2C, SHT21_ADDR, I2C_Direction_Receiver);
  
  /* Test on EV6 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))  /* EV6 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  
  /* Test on EV7 and clear it */
  I2C_Timeout = I2C_TIMEOUT;
  while (!I2C_CheckEvent(SHT21_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED))  /* EV7 */
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }  
  /* Store SHT21_I2C received data */
  read_Buffer[0] = I2C_ReceiveData(SHT21_I2C);
  
  /* Disable SHT21_I2C acknowledgement */
  I2C_AcknowledgeConfig(SHT21_I2C, DISABLE);
  /* Send SHT21_I2C STOP Condition */
  I2C_GenerateSTOP(SHT21_I2C, ENABLE); 
  
  /* Test on RXNE flag */
  while (I2C_GetFlagStatus(SHT21_I2C, I2C_FLAG_RXNE) == RESET)
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }
  /* Store SHT21_I2C received data */
  read_Buffer[1] = I2C_ReceiveData(SHT21_I2C);
  
  /* Return Temperature value */
  return I2C_OK;
}

static inline void Get_TempHum_1(void)
{
  uint8_t Status_t,Status_h;
  uint32_t   SHT21_Srh = 0;
  uint32_t   SHT21_St = 0;
  uint8_t read_Buffer_sht21_T[2];
  uint8_t read_Buffer_sht21_RH[2];
  
  Status_t = SHT21_ReadTempHum(SHT21_REG_TEMP,read_Buffer_sht21_T);
  Status_h = SHT21_ReadTempHum(SHT21_REG_HUM,read_Buffer_sht21_RH);
  
  if(Status_t+Status_h==2)
  {
    SHT21_Srh=read_Buffer_sht21_RH[0];
    SHT21_Srh=SHT21_Srh<<8;
    SHT21_Srh=SHT21_Srh+read_Buffer_sht21_RH[1];
    SHT21_Srh=SHT21_Srh & 0xFFFFFFFC;
    SHT21_Srh=12500*SHT21_Srh;
    SHT21_Srh=SHT21_Srh>>16;
    SHT21_RH= (uint16_t)(SHT21_Srh-600);
    //修正湿度
    SHT21_RH= SHT21_RH+Dh; 
    
    SHT21_St=read_Buffer_sht21_T[0];
    SHT21_St=SHT21_St<<8;
    SHT21_St=SHT21_St+read_Buffer_sht21_T[1];
    SHT21_St=SHT21_St & 0xFFFFFFFC;
    SHT21_St=17572*SHT21_St;
    SHT21_St=SHT21_St>>16;
    
    if(SHT21_St>=4685)
    {
      SHT21_T = (uint16_t)(SHT21_St-4685);
      SHT21_T = SHT21_T- Dt;
    }
    else
    {
      SHT21_T = (uint16_t)(4685-SHT21_St);
      SHT21_T = SHT21_T |0x8000;                //若为负温，最高位为1，正温度最高位为0
    }
  }
  else
  {
    SHT21_Init();
  }
}

uint16_t get_temperature(void)
{
  return SHT21_T;
}

uint16_t get_humidity(void)
{
  return SHT21_RH;
}

void Get_TempHum(void)
{
  if(flag){
    flag = false;
    Get_TempHum_1();
    update_disp(DISP_UPDATE_SHT21);
    time_create(TASK_ID_SHT21, DELAY_MS, &flag, true);
  }
  
  task_setBlock(TASK_ID_SHT21);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/