#ifndef GIMBAL_KEYS_H
#define GIMBAL_KEYS_H

#include "main.h"
#include <stdint.h>

typedef enum {
  GKEY_NONE = 0,
  GKEY_UP,
  GKEY_DOWN,
  GKEY_LEFT,
  GKEY_RIGHT,
  GKEY_CENTER
} GimbalKeyId;

typedef enum {
  GKEY_EVENT_NONE = 0,
  GKEY_EVENT_SHORT,
  GKEY_EVENT_LONG,
  GKEY_EVENT_VERY_LONG
} GimbalKeyEventType;

typedef struct {
  GimbalKeyId key;
  GimbalKeyEventType type;
} GimbalKeyEvent;

void GimbalKeys_Init(void);
void GimbalKeys_Task(uint32_t now_ms);
uint8_t GimbalKeys_GetEvent(GimbalKeyEvent *event);
GimbalKeyId GimbalKeys_GetHeldKey(void);
uint16_t GimbalKeys_GetAdcRaw(void);

#endif
