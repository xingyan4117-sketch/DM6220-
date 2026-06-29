#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#include "lvgl.h"

typedef void (*LvPortDisp_ServiceCallback)(void);

void LvPortDisp_Init(void);
void LvPortDisp_SetServiceCallback(LvPortDisp_ServiceCallback callback);

#endif
