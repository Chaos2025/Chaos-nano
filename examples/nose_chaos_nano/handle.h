#ifndef _KEY_H
#define _KEY_H

enum disp_update_t{
  DISP_UPDATE_POWER_OFF = 0,
  DISP_UPDATE_SHT21,
  DISP_UPDATE_TVOC,
  DISP_UPDATE_BAT,
};

void powerOnHandle(void);
void keyHandle(void);
void update_disp(enum disp_update_t disp_update_number);

void handle(void);

#endif