#include "gimbal_lvgl_ui.h"
#include "lcd.h"
#include <stdio.h>
#include <string.h>

#define RAD2DEG_F 57.29578f
#define MENU_MAX_ITEMS 7U
#define MENU_DEPTH_MAX 3U

typedef enum {
  UI_PAGE_MAIN = 0,
  UI_PAGE_RUN,
  UI_PAGE_PARAM,
  UI_PAGE_MOTOR,
  UI_PAGE_DISPLAY,
  UI_PAGE_ABOUT,
  UI_PAGE_STATUS
} UiPage;

static GimbalControlState *s_ctrl;
static DM_Motor_Info_Typedef *s_yaw;
static DM_Motor_Info_Typedef *s_pitch;
static UiPage s_stack[MENU_DEPTH_MAX];
static uint8_t s_depth;
static uint8_t s_selected[MENU_DEPTH_MAX];
static uint8_t s_editing_param;
static uint8_t s_dirty;
static uint32_t s_last_status_ms;

static lv_obj_t *s_title;
static lv_obj_t *s_mode;
static lv_obj_t *s_body;
static lv_obj_t *s_hint;
static lv_obj_t *s_status;

static const char *main_items[] = {"Run Mode", "MIT Params", "Motor", "Display", "About"};
static const char *run_items[] = {"Stop Motors", "MIT Circle", "Manual Jog", "Center Home"};
static const char *motor_items[] = {"Enable Both", "Disable Both", "Enable YAW", "Enable Pitch", "Power OUT1/OUT2"};
static const char *display_items[] = {"LCD On/Off", "Orientation Info"};

static UiPage page(void)
{
  return s_stack[s_depth];
}

static uint8_t motor_online(const DM_Motor_Info_Typedef *motor)
{
  return (motor != 0 && motor->Data.State != 0) ? 1U : 0U;
}

static uint8_t item_count(UiPage p)
{
  if (p == UI_PAGE_MAIN) return (uint8_t)(sizeof(main_items) / sizeof(main_items[0]));
  if (p == UI_PAGE_RUN) return (uint8_t)(sizeof(run_items) / sizeof(run_items[0]));
  if (p == UI_PAGE_PARAM) return 7U;
  if (p == UI_PAGE_MOTOR) return (uint8_t)(sizeof(motor_items) / sizeof(motor_items[0]));
  if (p == UI_PAGE_DISPLAY) return (uint8_t)(sizeof(display_items) / sizeof(display_items[0]));
  return 0U;
}

static const char *page_title(UiPage p)
{
  if (p == UI_PAGE_RUN) return "RUN";
  if (p == UI_PAGE_PARAM) return "PARAM";
  if (p == UI_PAGE_MOTOR) return "MOTOR";
  if (p == UI_PAGE_DISPLAY) return "DISPLAY";
  if (p == UI_PAGE_ABOUT) return "ABOUT";
  if (p == UI_PAGE_STATUS) return "MANUAL";
  return "MENU";
}

static void push(UiPage p)
{
  if (s_depth < (MENU_DEPTH_MAX - 1U)) {
    s_depth++;
    s_stack[s_depth] = p;
    s_selected[s_depth] = 0U;
    s_editing_param = 0U;
    s_dirty = 1U;
  }
}

static void pop(void)
{
  if (s_depth > 0U) {
    s_depth--;
  }
  s_editing_param = 0U;
  s_dirty = 1U;
}

static void clamp_params(void)
{
  GimbalControl_ClampParams(s_ctrl);
}

static void add_param(uint8_t index, int8_t dir)
{
  if (index == 0U) s_ctrl->kp += 0.1f * (float)dir;
  else if (index == 1U) s_ctrl->kd += 0.01f * (float)dir;
  else if (index == 2U) s_ctrl->torque_ff += 0.05f * (float)dir;
  else if (index == 3U) s_ctrl->yaw_amp_deg += 1.0f * (float)dir;
  else if (index == 4U) s_ctrl->pitch_amp_deg += 1.0f * (float)dir;
  else if (index == 5U) {
    int32_t next = (int32_t)s_ctrl->period_ms + (int32_t)dir * 100;
    if (next < 500) next = 500;
    if (next > 5000) next = 5000;
    s_ctrl->period_ms = (uint16_t)next;
  }
  clamp_params();
  s_dirty = 1U;
}

static void run_action(uint8_t index, uint32_t now_ms)
{
  if (index == 0U) GimbalControl_SetMode(s_ctrl, CTRL_STOP, now_ms);
  else if (index == 1U) GimbalControl_SetMode(s_ctrl, CTRL_MIT_CIRCLE, now_ms);
  else if (index == 2U) {
    GimbalControl_SetMode(s_ctrl, CTRL_MANUAL, now_ms);
    s_depth = 0U;
    s_stack[0] = UI_PAGE_STATUS;
  } else if (index == 3U) {
    GimbalControl_RequestHome(s_ctrl, now_ms);
  }
  s_dirty = 1U;
}

static void motor_action(uint8_t index)
{
  if (index == 0U) {
    s_ctrl->req_enable_yaw = 1U;
    s_ctrl->req_enable_pitch = 1U;
    s_ctrl->motors_enabled = 1U;
  } else if (index == 1U) {
    s_ctrl->req_disable_yaw = 1U;
    s_ctrl->req_disable_pitch = 1U;
    s_ctrl->motors_enabled = 0U;
    s_ctrl->mode = CTRL_STOP;
  } else if (index == 2U) {
    s_ctrl->req_enable_yaw = 1U;
    s_ctrl->motors_enabled = 1U;
  } else if (index == 3U) {
    s_ctrl->req_enable_pitch = 1U;
    s_ctrl->motors_enabled = 1U;
  } else if (index == 4U) {
    s_ctrl->yaw_power_on = (uint8_t)!s_ctrl->yaw_power_on;
    s_ctrl->pitch_power_on = (uint8_t)!s_ctrl->pitch_power_on;
    s_ctrl->req_power_apply = 1U;
  }
  s_dirty = 1U;
}

static void save_zero_both(void)
{
  if (motor_online(s_yaw) == 0U || motor_online(s_pitch) == 0U) {
    return;
  }
  s_ctrl->mode = CTRL_STOP;
  s_ctrl->req_save_zero_yaw = 1U;
  s_ctrl->req_save_zero_pitch = 1U;
  s_dirty = 1U;
}

static void draw_menu(void)
{
  uint8_t i;
  uint8_t count = item_count(page());
  char text[512];
  char line[64];

  text[0] = '\0';
  lv_label_set_text(s_title, page_title(page()));
  lv_label_set_text(s_mode, GimbalControl_ModeName(s_ctrl->mode));

  if (page() == UI_PAGE_ABOUT) {
    lv_label_set_text(s_body, "YAW ID 01 CAN1 PD0/PD1\nPIT ID 02 CAN2 PB5/PB6\nLCD 280x240 SPI1\nKEY ADC PA5\nCENTER ok  LEFT back");
    lv_label_set_text(s_hint, "ABOUT");
    return;
  }

  for (i = 0U; i < count && i < MENU_MAX_ITEMS; i++) {
    const char *name = "";
    if (page() == UI_PAGE_MAIN) name = main_items[i];
    else if (page() == UI_PAGE_RUN) name = run_items[i];
    else if (page() == UI_PAGE_MOTOR) name = motor_items[i];
    else if (page() == UI_PAGE_DISPLAY) name = display_items[i];
    else if (page() == UI_PAGE_PARAM) {
      char marker = (i == s_selected[s_depth]) ? (s_editing_param ? '*' : '>') : ' ';
      if (i == 0U) snprintf(line, sizeof(line), "%c KP          %.2f\n", marker, s_ctrl->kp);
      else if (i == 1U) snprintf(line, sizeof(line), "%c KD          %.2f\n", marker, s_ctrl->kd);
      else if (i == 2U) snprintf(line, sizeof(line), "%c Torque      %.2f\n", marker, s_ctrl->torque_ff);
      else if (i == 3U) snprintf(line, sizeof(line), "%c Amp YAW     %.1f\n", marker, s_ctrl->yaw_amp_deg);
      else if (i == 4U) snprintf(line, sizeof(line), "%c Amp PIT     %.1f\n", marker, s_ctrl->pitch_amp_deg);
      else if (i == 5U) snprintf(line, sizeof(line), "%c Period      %ums\n", marker, s_ctrl->period_ms);
      else snprintf(line, sizeof(line), "%c Reset Default\n", marker);
      strncat(text, line, sizeof(text) - strlen(text) - 1U);
      continue;
    }
    snprintf(line, sizeof(line), "%c %s\n", i == s_selected[s_depth] ? '>' : ' ', name);
    strncat(text, line, sizeof(text) - strlen(text) - 1U);
  }
  lv_label_set_text(s_body, text);
  if (page() == UI_PAGE_PARAM && s_editing_param) {
    lv_label_set_text(s_hint, "EDIT  UP/RIGHT +  DN/LEFT -  CENTER done");
  } else if (page() == UI_PAGE_PARAM) {
    lv_label_set_text(s_hint, "UP/DN select  RIGHT edit  LEFT back");
  } else {
    lv_label_set_text(s_hint, "UP/DN select  CENTER/RIGHT ok  LEFT back");
  }
}

static void draw_status(uint16_t key_adc)
{
  char text[384];
  float yaw_deg = s_yaw != 0 ? s_yaw->Data.Position * RAD2DEG_F : 0.0f;
  float pit_deg = s_pitch != 0 ? s_pitch->Data.Position * RAD2DEG_F : 0.0f;

  lv_label_set_text(s_title, "MANUAL JOG");
  lv_label_set_text(s_mode, GimbalControl_ModeName(s_ctrl->mode));
  snprintf(text, sizeof(text),
           "YAW %s  P %6.1f\nPIT %s  P %6.1f\n\nKP %.2f  KD %.2f\nTarget Y %.1f  P %.1f\n\nKEY ADC %u",
           motor_online(s_yaw) ? "ON " : "OFF", yaw_deg,
           motor_online(s_pitch) ? "ON " : "OFF", pit_deg,
           s_ctrl->kp, s_ctrl->kd,
           s_ctrl->yaw_target_rad * RAD2DEG_F,
           s_ctrl->pitch_target_rad * RAD2DEG_F,
           key_adc);
  lv_label_set_text(s_body, text);
  lv_label_set_text(s_hint, "ARROWS jog  CENTER3S save zero  CENTER menu");
}

void GimbalLvglUi_Init(GimbalControlState *ctrl,
                       DM_Motor_Info_Typedef *yaw,
                       DM_Motor_Info_Typedef *pitch)
{
  s_ctrl = ctrl;
  s_yaw = yaw;
  s_pitch = pitch;
  s_stack[0] = UI_PAGE_MAIN;
  s_depth = 0U;
  s_editing_param = 0U;
  s_dirty = 1U;

  s_status = lv_obj_create(lv_screen_active());
  lv_obj_remove_style_all(s_status);
  lv_obj_set_size(s_status, LCD_W, LCD_H);
  lv_obj_set_style_bg_color(s_status, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(s_status, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(s_status, 8, 0);

  s_title = lv_label_create(s_status);
  lv_obj_set_style_text_color(s_title, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(s_title, &lv_font_montserrat_16, 0);
  lv_obj_align(s_title, LV_ALIGN_TOP_LEFT, 0, 0);

  s_mode = lv_label_create(s_status);
  lv_obj_set_style_text_color(s_mode, lv_color_hex(0xffd166), 0);
  lv_obj_align(s_mode, LV_ALIGN_TOP_RIGHT, 0, 2);

  s_body = lv_label_create(s_status);
  lv_obj_set_style_text_color(s_body, lv_color_hex(0xd8e2dc), 0);
  lv_obj_set_style_text_font(s_body, &lv_font_montserrat_14, 0);
  lv_obj_align(s_body, LV_ALIGN_TOP_LEFT, 0, 34);
  lv_obj_set_width(s_body, LCD_W - 16);

  s_hint = lv_label_create(s_status);
  lv_obj_set_style_text_color(s_hint, lv_color_hex(0x8ecae6), 0);
  lv_obj_set_style_text_font(s_hint, &lv_font_montserrat_14, 0);
  lv_obj_align(s_hint, LV_ALIGN_BOTTOM_LEFT, 0, 0);
}

void GimbalLvglUi_HandleEvent(const GimbalKeyEvent *event, uint32_t now_ms)
{
  uint8_t count;
  if (event == 0 || event->type == GKEY_EVENT_NONE || s_ctrl == 0) return;

  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_VERY_LONG) {
    GimbalControl_EStop(s_ctrl);
    s_editing_param = 0U;
    s_dirty = 1U;
    return;
  }
  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_ZERO) {
    if (page() == UI_PAGE_STATUS && s_ctrl->mode == CTRL_MANUAL) save_zero_both();
    return;
  }
  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_LONG) {
    if (page() != UI_PAGE_STATUS) {
      GimbalControl_RequestHome(s_ctrl, now_ms);
      s_stack[0] = UI_PAGE_MAIN;
      s_depth = 0U;
      s_editing_param = 0U;
      s_dirty = 1U;
    }
    return;
  }
  if (event->type != GKEY_EVENT_SHORT) return;

  if (page() == UI_PAGE_STATUS) {
    if (event->key == GKEY_CENTER) {
      s_stack[0] = UI_PAGE_MAIN;
      s_depth = 0U;
      s_editing_param = 0U;
      s_dirty = 1U;
    }
    return;
  }

  count = item_count(page());
  if (page() == UI_PAGE_PARAM && s_editing_param) {
    if (event->key == GKEY_RIGHT || event->key == GKEY_UP) {
      add_param(s_selected[s_depth], 1);
    } else if (event->key == GKEY_LEFT || event->key == GKEY_DOWN) {
      add_param(s_selected[s_depth], -1);
    } else if (event->key == GKEY_CENTER) {
      s_editing_param = 0U;
      s_dirty = 1U;
    }
    return;
  }

  if (event->key == GKEY_UP && count > 0U) {
    s_selected[s_depth] = (uint8_t)((s_selected[s_depth] + count - 1U) % count);
    s_dirty = 1U;
  } else if (event->key == GKEY_DOWN && count > 0U) {
    s_selected[s_depth] = (uint8_t)((s_selected[s_depth] + 1U) % count);
    s_dirty = 1U;
  } else if (event->key == GKEY_LEFT) {
    pop();
  } else if (event->key == GKEY_CENTER || event->key == GKEY_RIGHT) {
    uint8_t idx = s_selected[s_depth];
    if (page() == UI_PAGE_MAIN) {
      if (idx == 0U) push(UI_PAGE_RUN);
      else if (idx == 1U) push(UI_PAGE_PARAM);
      else if (idx == 2U) push(UI_PAGE_MOTOR);
      else if (idx == 3U) push(UI_PAGE_DISPLAY);
      else if (idx == 4U) push(UI_PAGE_ABOUT);
    } else if (page() == UI_PAGE_RUN) {
      run_action(idx, now_ms);
    } else if (page() == UI_PAGE_PARAM) {
      if (idx == 6U) GimbalControl_ResetDefaults(s_ctrl);
      else s_editing_param = 1U;
      s_dirty = 1U;
    } else if (page() == UI_PAGE_MOTOR) {
      motor_action(idx);
    } else if (page() == UI_PAGE_DISPLAY) {
      s_ctrl->lcd_on = (uint8_t)!s_ctrl->lcd_on;
      if (s_ctrl->lcd_on) LCD_BLK_Set(1); else LCD_BLK_Clr(0);
      s_dirty = 1U;
    } else if (page() == UI_PAGE_ABOUT) {
      pop();
    }
  }
}

void GimbalLvglUi_Task(uint32_t now_ms, uint16_t key_adc)
{
  if (page() == UI_PAGE_STATUS) {
    if (s_dirty || (now_ms - s_last_status_ms) >= 200U) {
      s_last_status_ms = now_ms;
      draw_status(key_adc);
      s_dirty = 0U;
    }
  } else if (s_dirty) {
    draw_menu();
    s_dirty = 0U;
  }
}

uint8_t GimbalLvglUi_IsStatusPage(void)
{
  return (page() == UI_PAGE_STATUS) ? 1U : 0U;
}
