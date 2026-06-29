#include "lv_port_indev.h"
#include "gimbal_keys.h"

static lv_indev_t *s_keypad;
static uint32_t s_last_key;

static uint32_t key_to_lv(GimbalKeyId key)
{
  switch (key) {
    case GKEY_UP:
      return LV_KEY_UP;
    case GKEY_DOWN:
      return LV_KEY_DOWN;
    case GKEY_LEFT:
      return LV_KEY_LEFT;
    case GKEY_RIGHT:
      return LV_KEY_RIGHT;
    case GKEY_CENTER:
      return LV_KEY_ENTER;
    default:
      return 0U;
  }
}

static void lv_port_key_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  GimbalKeyId held = GimbalKeys_GetHeldKey();
  uint32_t key = key_to_lv(held);
  (void)indev;

  if (key != 0U) {
    s_last_key = key;
    data->key = key;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->key = s_last_key;
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void LvPortIndev_Init(void)
{
  s_keypad = lv_indev_create();
  lv_indev_set_type(s_keypad, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(s_keypad, lv_port_key_read);
}
