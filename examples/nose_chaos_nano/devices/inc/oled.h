#ifndef _OLED_H
#define _OLED_H
#include "stm8l15x.h"
#include "pin.h"

//-----------------OLED¶Ë¿Ú¶¨Òå----------------  					   

#define OLED_SCL_CLR()	GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define OLED_SCL_SET()	GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define OLED_SDA_CLR()	GPIO_ResetBits(GPIOB,GPIO_Pin_4)
#define OLED_SDA_SET()	GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define OLED_RST_CLR()	GPIO_ResetBits(GPIOB,GPIO_Pin_7)
#define OLED_RST_SET()	GPIO_SetBits(GPIOB,GPIO_Pin_7)

#define OLED_DC_CLR()	GPIO_ResetBits(GPIOB,GPIO_Pin_6)
#define OLED_DC_SET()	GPIO_SetBits(GPIOB,GPIO_Pin_6)

extern const u8 *numb[];
extern const u8 *pow_bat[];
extern const u8 WARM_UP[];
extern const u8 eCO2[];
extern const u8 TVOC[];
extern const u8 air_quality[];
//extern const u8 Temp[];
//extern const u8 Humi[];
//extern const u8 A_TVOC[];

extern const u8 Unit_C[];
extern const u8 Unit_RH[];
extern const u8 My_Unit_RH[];//my
extern const u8 My_Unit_cc[];//my
extern const u8 Unit_PPM[];
extern const u8 Unit_mg_l[];
extern const u8 hum_ch[];
extern const u8 temp_ch[];
extern const u8 My_you[];
extern const u8 My_liang[];
extern const u8 My_cha[];

//extern const u8 BAT[];

//extern long	my_timers;

extern void OLED_Init(void);
extern void OLED_CLS(void); 
extern void OLED_Fill(u8 dat);
extern void write_number_1632(u8 page,u8 col,const u8 *hzk,u8 inverse);
void write_number_1632_clr(u8 page,u8 col, u8 inverse);
extern void hz_disp1216(u8 page,u8 col,const u8 *hzk);
extern void hz_disp1216_5(u8 page,u8 col,const u8 *hzk);
extern void hz_disp1216_2(u8 page,u8 col,const u8 *hzk);

extern void My_Show_Uint(u8 page,u8 col,const u8 hzk);//my
//extern void Warm_up(u8 page,u8 col, const u8 *hzk);
extern void write_number(u8 page,u8 col,u8 x,u8 inverse);
extern void My_write_number_Big(u8 page,u8 col,u8 x,u8 inverse);//my
extern void hz_disp8_16_4(u8 page,u8 col,const u8  *hzk);
extern void hz_disp8_16_5(u8 page,u8 col,const u8  *hzk);
extern void bat(u8 page,u8 col, const u8  *hzk);
extern void bat_pic(u8 page,u8 col, const u8  *hzk);

extern long GetMyTimers(void);

void OLED_Flash(int state);
void OLED_Close(void);
void OLED_Open(void);

#endif

