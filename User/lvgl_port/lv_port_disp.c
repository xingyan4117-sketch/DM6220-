#include "lv_port_disp.h"
#include "lcd.h"
#include <string.h>

#define LV_PORT_BUF_LINES 24U
#define LV_PORT_TX_CHUNK_PIXELS 280U

static lv_display_t *s_display;
static uint16_t s_draw_buf[LCD_W * LV_PORT_BUF_LINES];
static uint8_t s_tx_buf[LV_PORT_TX_CHUNK_PIXELS * 2U];
static LvPortDisp_ServiceCallback s_service_callback;

static void service_during_flush(void)
{
  if (s_service_callback != 0) {
    s_service_callback();
  }
}

static void lv_port_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  int32_t x;
  int32_t y;
  uint32_t tx_count = 0U;
  uint16_t *src = (uint16_t *)px_map;

  if (area->x2 < 0 || area->y2 < 0 || area->x1 >= LCD_W || area->y1 >= LCD_H) {
    lv_display_flush_ready(disp);
    return;
  }

  LCD_Address_Set((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)area->x2, (uint16_t)area->y2);
  for (y = area->y1; y <= area->y2; y++) {
    for (x = area->x1; x <= area->x2; x++) {
      uint16_t c = *src++;
      s_tx_buf[tx_count++] = (uint8_t)(c >> 8);
      s_tx_buf[tx_count++] = (uint8_t)(c & 0xffU);
      if (tx_count >= sizeof(s_tx_buf)) {
        LCD_Write_Data_Buffer(s_tx_buf, tx_count);
        tx_count = 0U;
        service_during_flush();
      }
    }
  }
  if (tx_count > 0U) {
    LCD_Write_Data_Buffer(s_tx_buf, tx_count);
    service_during_flush();
  }

  lv_display_flush_ready(disp);
}

void LvPortDisp_SetServiceCallback(LvPortDisp_ServiceCallback callback)
{
  s_service_callback = callback;
}

void LvPortDisp_Init(void)
{
  s_display = lv_display_create(LCD_W, LCD_H);
  lv_display_set_flush_cb(s_display, lv_port_flush_cb);
  lv_display_set_buffers(s_display, s_draw_buf, NULL, sizeof(s_draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
}
