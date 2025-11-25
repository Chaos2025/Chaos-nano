#ifndef _PIN_H
#define _PIN_H

#include "stm8l15x.h"

#define KEY_CTL_ADC_OLED_GPIO_PORT        GPIOB
#define CHARGE_GPIO_PORT                  GPIOB
#define OLED_SPI_SCK_GPIO_PINS            GPIO_Pin_6
#define OLED_SPI_MOSI_GPIO_PINS           GPIO_Pin_7
#define OLED_SPI_MISO_GPIO_PINS           GPIO_Pin_4
#define OLED_SPI_NSS_GPIO_PINS            GPIO_Pin_5
#define KEY_DIS_GPIO_PINS                 GPIO_Pin_1
#define ADC_SENSOR_GPIO_PINS              GPIO_Pin_0
#define CHGSTAT_GPIO_PINS                 GPIO_Pin_2
#define PGSTAT_GPIO_PINS                  GPIO_Pin_3

#define KEY_POWER_GPIO_PORT               GPIOA
#define CTL_POWER_GPIO_PINS               GPIO_Pin_2
#define KEY_POWER_GPIO_PINS               GPIO_Pin_3

#define BAT_GPIO_PORT                     GPIOC
#define ADC_BAT_GPIO_PINS                 GPIO_Pin_4

#define HEATCTL_GPIO_PORT                 GPIOD
#define HEATCTL_GPIO_PINS                 GPIO_Pin_0

void delayUs(__IO uint16_t us);

#endif