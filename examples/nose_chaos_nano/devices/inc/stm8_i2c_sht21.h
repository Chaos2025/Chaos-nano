/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM8_I2C_SHT21_H
#define __STM8_I2C_SHT21_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
  
/** @addtogroup STM8_EVAL_I2C_TSENSOR
  * @{
  */  

/* Private typedef -----------------------------------------------------------*/

#define SHT21_I2C                         I2C1
#define SHT21_I2C_CLK                     CLK_Peripheral_I2C1
#define SHT21_I2C_SCL_PIN                 GPIO_Pin_1                  /* PC.01 */
#define SHT21_I2C_SCL_GPIO_PORT           GPIOC                       /* GPIOC */
#define SHT21_I2C_SDA_PIN                 GPIO_Pin_0                  /* PC.00 */
#define SHT21_I2C_SDA_GPIO_PORT           GPIOC                       /* GPIOC */

/* Private define ------------------------------------------------------------*/
/**
  * @brief  Block Size
  */
#define SHT21_REG_TEMP        0xe3  /* Temperature Register of SHT21 */
#define SHT21_REG_HUM         0xe5  /* Humidity Register of SHT21 */
#define I2C_TIMEOUT           (uint32_t)0x8FFFF /*!< I2C Time out */
#define SHT21_ADDR            0x80   /*!< SHT21 address */
#define SHT21_I2C_SPEED       20000 /*!< I2C Speed */
#define I2C_FAIL              0
#define I2C_OK                1

typedef unsigned char         I2C_Status;
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SHT21_Init(void);
void Get_TempHum(void);
uint16_t get_temperature(void);
uint16_t get_humidity(void);

#endif /* __STM8_EVAL_I2C_TSENSOR_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
