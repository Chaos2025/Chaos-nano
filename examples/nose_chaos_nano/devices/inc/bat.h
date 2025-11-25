#ifndef _BAT_H
#define _BAT_H

#include "stm8l15x.h"

void BAT_Init(void);
void Check_Charge_Bat(void);
u8 getBatStatus(void);
u8 getBatLevel(void);

#endif