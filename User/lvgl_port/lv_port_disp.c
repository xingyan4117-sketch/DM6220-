#include "lv_port_disp.h"
#include "lcd.h"
#include <string.h>

#define LV_PORT_BUF_LINES 24U

/* Keep LVGL on the same byte-write path as the original LCD UI first.
 * The 1.69 in module is sensitive to long CS-low SPI bursts on this board. */
#define LV_PORT_USE_BULK_SPI_FLUSH 0U

#if LV_PORT_USE_BULK_SPI_FLUSH
#define LV_PORT_TX_CHUNK_PIXELS 280U
#endif

static lv_display_t *s_display;
LV_DRAW_BUF_DEFINE_STATIC(s_lv_draw_buf, LCD_W, LV_PORT_BUF_LINES, LV_COLOR_FORMAT_RGB565);
#if LV_PORT_USE_BULK_SPI_FLUSH
static uint8_t s_tx_buf[LV_PORT_TX_CHUNK_PIXELS * 2U];
#endif
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
#if LV_PORT_USE_BULK_SPI_FLUSH
  uint32_t tx_count = 0U;
#endif
  uint8_t *src = px_map;

  if (area->x2 < 0 || area->y2 < 0 || area->x1 >= LCD_W || area->y1 >= LCD_H) {
    lv_display_flush_ready(disp);
    return;
  }

  LCD_Address_Set((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)area->x2, (uint16_t)area->y2);
  for (y = area->y1; y <= area->y2; y++) {
    for (x = area->x1; x <= area->x2; x++) {
      uint8_t lo = *src++;
      uint8_t hi = *src++;
#if LV_PORT_USE_BULK_SPI_FLUSH
      s_tx_buf[tx_count++] = hi;
      s_tx_buf[tx_count++] = lo;
      if (tx_count >= sizeof(s_tx_buf)) {
        LCD_Write_Data_Buffer(s_tx_buf, tx_count);
        tx_count = 0U;
        service_during_flush();
      }
#else
      LCD_WR_DATA8(hi);
      LCD_WR_DATA8(lo);
      if ((((uint32_t)(x - area->x1)) & 0x0FU) == 0U) {
        service_during_flush();
      }
#endif
    }
  }
#if LV_PORT_USE_BULK_SPI_FLUSH
  if (tx_count > 0U) {
    LCD_Write_Data_Buffer(s_tx_buf, tx_count);
    service_during_flush();
  }
#else
  service_during_flush();
#endif

  lv_display_flush_ready(disp);
}

void LvPortDisp_SetServiceCallback(LvPortDisp_ServiceCallback callback)
{
  s_service_callback = callback;
}

void LvPortDisp_Init(void)
{
  s_display = lv_display_create(LCD_W, LCD_H);
  lv_display_set_color_format(s_display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(s_display, lv_port_flush_cb);
  LV_DRAW_BUF_INIT_STATIC(s_lv_draw_buf);
  lv_display_set_draw_buffers(s_display, &s_lv_draw_buf, NULL);
  lv_display_set_render_mode(s_display, LV_DISPLAY_RENDER_MODE_PARTIAL);
}
