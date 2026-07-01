#include "gimbal_menu.h"
#include "lcd.h"
#include "gimbal_laser.h"
#include <stdio.h>
#include <string.h>

#define RAD2DEG          57.29578f
#define MENU_STACK_MAX   4U
#define ITEMS_MAX        9U
#define CLEAR_BAND_H     20U
#define LASER_BRIGHTNESS_STEP 10U
#define MENU_NAV_INTERVAL_MS  180U
#define MENU_EDIT_INTERVAL_MS 150U

typedef enum {
  PAGE_STATUS = 0,
  PAGE_MAIN,
  PAGE_RUN,
  PAGE_TRAJECTORY,
  PAGE_CIRCLE,
  PAGE_SETTINGS,
  PAGE_PARAM,
  PAGE_ZERO,
  PAGE_SQUARE,
  PAGE_LASER,
  PAGE_MOTOR,
  PAGE_DISPLAY,
  PAGE_ABOUT
} MenuPage;

typedef enum {
  EDIT_NONE = 0,
  EDIT_KP,
  EDIT_KD,
  EDIT_TORQUE,
  EDIT_YAW_AMP,
  EDIT_PITCH_AMP,
  EDIT_PERIOD,
  EDIT_CIRCLE_PERIOD,
  EDIT_CIRCLE_YAW_RADIUS,
  EDIT_CIRCLE_PITCH_RADIUS,
  EDIT_SQUARE_EDGE
} EditTarget;

typedef enum {
  CONFIRM_NONE = 0,
  CONFIRM_SAVE_YAW,
  CONFIRM_SAVE_PITCH,
  CONFIRM_SAVE_BOTH,
  CONFIRM_CLEAR_SQUARE,
  CONFIRM_DISABLE_BOTH,
  CONFIRM_RESET_DEFAULTS
} ConfirmAction;

static MenuPage page_stack[MENU_STACK_MAX];
static uint8_t page_depth;
static uint8_t selected[MENU_STACK_MAX];
static EditTarget editing;
static ConfirmAction confirm;
static char status_msg[24];
static uint8_t redraw_needed;
static uint8_t line_dirty;
static uint8_t dirty_old;
static uint8_t dirty_new;
static uint8_t render_step;
static uint8_t status_update_step;
static uint16_t clear_y;
static uint8_t square_calib_active;
static uint8_t square_calib_index;
static uint32_t last_nav_ms;
static uint32_t last_edit_ms;
static GimbalMenu_ServiceCallback service_callback;

static const char *main_items[] = {"Run", "Trajectory", "Laser", "Settings"};
static const char *run_items[] = {"Stop", "Manual Jog", "Home"};
static const char *trajectory_items[] = {"Circle", "Square"};
static const char *circle_items[] = {"Circle Run", "Circle Speed", "Radius YAW", "Radius PIT"};
static const char *param_items[] = {"KP", "KD", "Torque FF", "Amp YAW", "Amp Pitch", "Period", "Reset Default"};
static const char *zero_items[] = {"Set Soft Zero", "Save Zero YAW", "Save Zero Pitch", "Save Both Zero"};
static const char *square_items[] = {"Start Calib", "Edge Time", "Square Run", "Pause/Resume", "Reset to LB", "Clear Calib"};
static const char *motor_items[] = {"Enable Both", "Disable Both", "Enable YAW", "Enable Pitch", "Power OUT1/OUT2"};
static const char *display_items[] = {"Brightness", "Orientation Info"};
static const char *settings_items[] = {"MIT Params", "Motor Power", "Display", "About"};

static void menu_service(void)
{
  if (service_callback != 0) {
    service_callback();
  }
}

static uint8_t item_count(MenuPage page)
{
  switch (page) {
    case PAGE_MAIN:
      return (uint8_t)(sizeof(main_items) / sizeof(main_items[0]));
    case PAGE_RUN:
      return (uint8_t)(sizeof(run_items) / sizeof(run_items[0]));
    case PAGE_TRAJECTORY:
      return (uint8_t)(sizeof(trajectory_items) / sizeof(trajectory_items[0]));
    case PAGE_CIRCLE:
      return (uint8_t)(sizeof(circle_items) / sizeof(circle_items[0]));
    case PAGE_SETTINGS:
      return (uint8_t)(sizeof(settings_items) / sizeof(settings_items[0]));
    case PAGE_PARAM:
      return (uint8_t)(sizeof(param_items) / sizeof(param_items[0]));
    case PAGE_ZERO:
      return (uint8_t)(sizeof(zero_items) / sizeof(zero_items[0]));
    case PAGE_SQUARE:
      return (uint8_t)(sizeof(square_items) / sizeof(square_items[0]));
    case PAGE_LASER:
      return 0U;
    case PAGE_MOTOR:
      return (uint8_t)(sizeof(motor_items) / sizeof(motor_items[0]));
    case PAGE_DISPLAY:
      return (uint8_t)(sizeof(display_items) / sizeof(display_items[0]));
    default:
      return 0U;
  }
}

static const char *item_text(MenuPage page, uint8_t index)
{
  switch (page) {
    case PAGE_MAIN:
      return main_items[index];
    case PAGE_RUN:
      return run_items[index];
    case PAGE_TRAJECTORY:
      return trajectory_items[index];
    case PAGE_CIRCLE:
      return circle_items[index];
    case PAGE_SETTINGS:
      return settings_items[index];
    case PAGE_PARAM:
      return param_items[index];
    case PAGE_ZERO:
      return zero_items[index];
    case PAGE_SQUARE:
      return square_items[index];
    case PAGE_LASER:
      return "";
    case PAGE_MOTOR:
      return motor_items[index];
    case PAGE_DISPLAY:
      return display_items[index];
    default:
      return "";
  }
}

static MenuPage current_page(void)
{
  return page_stack[page_depth];
}

static uint8_t current_selected(void)
{
  return selected[page_depth];
}

static void push_page(MenuPage page)
{
  if (page_depth < (MENU_STACK_MAX - 1U)) {
    page_depth++;
    page_stack[page_depth] = page;
    selected[page_depth] = 0U;
    redraw_needed = 1U;
    render_step = 0U;
    clear_y = 0U;
  }
}

static void pop_page(void)
{
  if (page_depth > 0U) {
    page_depth--;
  } else {
    page_stack[0] = PAGE_STATUS;
  }
  editing = EDIT_NONE;
  confirm = CONFIRM_NONE;
  square_calib_active = 0U;
  redraw_needed = 1U;
  render_step = 0U;
  clear_y = 0U;
}

static void set_msg(const char *msg)
{
  strncpy(status_msg, msg, sizeof(status_msg) - 1U);
  status_msg[sizeof(status_msg) - 1U] = '\0';
  redraw_needed = 1U;
  render_step = 0U;
  status_update_step = 0U;
  clear_y = 0U;
}

static uint8_t motor_online(const DM_Motor_Info_Typedef *motor)
{
  return (motor != 0 && motor->Data.State != 0) ? 1U : 0U;
}

static void begin_confirm(ConfirmAction action,
                          GimbalControlState *ctrl,
                          const DM_Motor_Info_Typedef *yaw,
                          const DM_Motor_Info_Typedef *pitch)
{
  if (ctrl == 0) {
    return;
  }
  if ((action == CONFIRM_SAVE_YAW || action == CONFIRM_SAVE_BOTH) && motor_online(yaw) == 0U) {
    set_msg("YAW offline");
    return;
  }
  if ((action == CONFIRM_SAVE_PITCH || action == CONFIRM_SAVE_BOTH) && motor_online(pitch) == 0U) {
    set_msg("PIT offline");
    return;
  }
  ctrl->mode = CTRL_CALIB_CONFIRM;
  confirm = action;
  set_msg("Center mech first");
}

static void confirm_yes(GimbalControlState *ctrl)
{
  if (ctrl == 0) {
    return;
  }
  if (confirm == CONFIRM_SAVE_YAW || confirm == CONFIRM_SAVE_BOTH) {
    ctrl->mode = CTRL_STOP;
    ctrl->req_save_zero_yaw = 1U;
  }
  if (confirm == CONFIRM_SAVE_PITCH || confirm == CONFIRM_SAVE_BOTH) {
    ctrl->mode = CTRL_STOP;
    ctrl->req_save_zero_pitch = 1U;
  }
  if (confirm == CONFIRM_CLEAR_SQUARE) {
    GimbalControl_SquareClear(ctrl);
    square_calib_active = 0U;
    set_msg("Square clear");
  } else if (confirm == CONFIRM_DISABLE_BOTH) {
    ctrl->req_disable_yaw = 1U;
    ctrl->req_disable_pitch = 1U;
    ctrl->motors_enabled = 0U;
    ctrl->mode = CTRL_STOP;
    set_msg("Disable both");
  } else if (confirm == CONFIRM_RESET_DEFAULTS) {
    GimbalControl_ResetDefaults(ctrl);
    set_msg("Defaults loaded");
  } else {
    set_msg("Save requested");
  }
  confirm = CONFIRM_NONE;
}

static float *edit_float(GimbalControlState *ctrl, EditTarget target)
{
  switch (target) {
    case EDIT_KP:
      return &ctrl->kp;
    case EDIT_KD:
      return &ctrl->kd;
    case EDIT_TORQUE:
      return &ctrl->torque_ff;
    case EDIT_YAW_AMP:
      return &ctrl->yaw_amp_deg;
    case EDIT_PITCH_AMP:
      return &ctrl->pitch_amp_deg;
    default:
      return 0;
  }
}

static float edit_step(EditTarget target)
{
  switch (target) {
    case EDIT_KP:
      return 0.1f;
    case EDIT_KD:
      return 0.01f;
    case EDIT_TORQUE:
      return 0.05f;
    case EDIT_YAW_AMP:
    case EDIT_PITCH_AMP:
      return 1.0f;
    default:
      return 0.0f;
  }
}

static void edit_change(GimbalControlState *ctrl, int8_t dir)
{
  float *value;
  if (ctrl == 0 || editing == EDIT_NONE) {
    return;
  }
  if (editing == EDIT_PERIOD || editing == EDIT_CIRCLE_PERIOD || editing == EDIT_SQUARE_EDGE) {
    uint16_t *duration = (editing == EDIT_SQUARE_EDGE) ? &ctrl->square_edge_ms : &ctrl->period_ms;
    int32_t next = (int32_t)(*duration) + (int32_t)dir * 100;
    if (next < 500) {
      next = 500;
    } else if (next > 5000) {
      next = 5000;
    }
    *duration = (uint16_t)next;
  } else if (editing == EDIT_CIRCLE_YAW_RADIUS) {
    ctrl->yaw_amp_deg += 1.0f * (float)dir;
  } else if (editing == EDIT_CIRCLE_PITCH_RADIUS) {
    ctrl->pitch_amp_deg += 1.0f * (float)dir;
  } else {
    value = edit_float(ctrl, editing);
    if (value != 0) {
      *value += edit_step(editing) * (float)dir;
    }
  }
  GimbalControl_ClampParams(ctrl);
}

static void run_action(GimbalControlState *ctrl, uint8_t index, uint32_t now_ms)
{
  if (ctrl == 0) {
    return;
  }
  if (index == 0U) {
    GimbalControl_SetMode(ctrl, CTRL_STOP, now_ms);
    set_msg("Mode STOP");
  } else if (index == 1U) {
    GimbalControl_SetMode(ctrl, CTRL_MANUAL, now_ms);
    set_msg("Mode MANUAL");
    page_stack[0] = PAGE_STATUS;
    page_depth = 0U;
  } else if (index == 2U) {
    GimbalControl_RequestHome(ctrl, now_ms);
    set_msg("Homing");
  }
}

static void circle_action(GimbalControlState *ctrl, uint8_t index, uint32_t now_ms)
{
  if (ctrl == 0) {
    return;
  }

  if (index == 0U) {
    GimbalControl_SetMode(ctrl, CTRL_MIT_CIRCLE, now_ms);
    set_msg("Mode CIRCLE");
  } else if (index == 1U) {
    GimbalControl_SetMode(ctrl, CTRL_STOP, now_ms);
    editing = EDIT_CIRCLE_PERIOD;
    set_msg("Circle stop edit");
  } else if (index == 2U) {
    GimbalControl_SetMode(ctrl, CTRL_STOP, now_ms);
    editing = EDIT_CIRCLE_YAW_RADIUS;
    set_msg("Yaw radius edit");
  } else if (index == 3U) {
    GimbalControl_SetMode(ctrl, CTRL_STOP, now_ms);
    editing = EDIT_CIRCLE_PITCH_RADIUS;
    set_msg("Pit radius edit");
  }
}

static void param_action(GimbalControlState *ctrl, uint8_t index)
{
  if (ctrl == 0) {
    return;
  }
  if (index == 0U) {
    editing = EDIT_KP;
  } else if (index == 1U) {
    editing = EDIT_KD;
  } else if (index == 2U) {
    editing = EDIT_TORQUE;
  } else if (index == 3U) {
    editing = EDIT_YAW_AMP;
  } else if (index == 4U) {
    editing = EDIT_PITCH_AMP;
  } else if (index == 5U) {
    editing = EDIT_PERIOD;
  } else if (index == 6U) {
    confirm = CONFIRM_RESET_DEFAULTS;
    set_msg("Confirm defaults");
  }
}

static void zero_action(GimbalControlState *ctrl,
                        uint8_t index,
                        const DM_Motor_Info_Typedef *yaw,
                        const DM_Motor_Info_Typedef *pitch)
{
  if (ctrl == 0) {
    return;
  }
  if (index == 0U) {
    float yaw_pos = (yaw != 0) ? yaw->Data.Position : 0.0f;
    float pitch_pos = (pitch != 0) ? pitch->Data.Position : 0.0f;
    GimbalControl_SetSoftZero(ctrl, yaw_pos, pitch_pos);
    set_msg("Soft zero set");
  } else if (index == 1U) {
    begin_confirm(CONFIRM_SAVE_YAW, ctrl, yaw, pitch);
  } else if (index == 2U) {
    begin_confirm(CONFIRM_SAVE_PITCH, ctrl, yaw, pitch);
  } else if (index == 3U) {
    begin_confirm(CONFIRM_SAVE_BOTH, ctrl, yaw, pitch);
  }
}

static void square_begin_calib(GimbalControlState *ctrl, uint32_t now_ms)
{
  if (ctrl == 0) {
    return;
  }

  square_calib_active = 1U;
  square_calib_index = 0U;
  ctrl->square_calib_index = 0U;
  GimbalControl_SetMode(ctrl, CTRL_SQUARE_CALIB, now_ms);
  set_msg("Move P0 CENTER");
}

static void square_action(GimbalControlState *ctrl, uint8_t index, uint32_t now_ms)
{
  if (ctrl == 0) {
    return;
  }

  if (index == 0U) {
    GimbalControl_SquareClear(ctrl);
    square_begin_calib(ctrl, now_ms);
  } else if (index == 1U) {
    editing = EDIT_SQUARE_EDGE;
  } else if (index == 2U) {
    if (GimbalControl_SquareCanRun(ctrl)) {
      GimbalControl_SquareRun(ctrl, now_ms);
      page_stack[0] = PAGE_STATUS;
      page_depth = 0U;
      set_msg("Square run");
    } else {
      set_msg("Calib 4 pts");
    }
  } else if (index == 3U) {
    if (ctrl->mode == CTRL_SQUARE_RUN) {
      GimbalControl_SquarePause(ctrl);
      set_msg("Square pause");
    } else if (ctrl->mode == CTRL_SQUARE_PAUSE) {
      GimbalControl_SquareResume(ctrl, now_ms);
      set_msg("Square resume");
    } else {
      set_msg("Not running");
    }
  } else if (index == 4U) {
    GimbalControl_SquareReset(ctrl, now_ms);
    set_msg("Reset to LB");
  } else if (index == 5U) {
    confirm = CONFIRM_CLEAR_SQUARE;
    set_msg("Confirm clear");
  }
}

static void laser_change_brightness(int8_t dir)
{
  int16_t next = (int16_t)GimbalLaser_GetBrightness() + (int16_t)dir * (int16_t)LASER_BRIGHTNESS_STEP;

  if (next < 0) {
    next = 0;
  } else if (next > 100) {
    next = 100;
  }

  GimbalLaser_SetBrightness((uint8_t)next);
}

static void motor_action(GimbalControlState *ctrl, uint8_t index)
{
  if (ctrl == 0) {
    return;
  }
  if (index == 0U) {
    ctrl->req_enable_yaw = 1U;
    ctrl->req_enable_pitch = 1U;
    ctrl->motors_enabled = 1U;
    set_msg("Enable both");
  } else if (index == 1U) {
    confirm = CONFIRM_DISABLE_BOTH;
    set_msg("Confirm disable");
  } else if (index == 2U) {
    ctrl->req_enable_yaw = 1U;
    ctrl->motors_enabled = 1U;
    set_msg("Enable YAW");
  } else if (index == 3U) {
    ctrl->req_enable_pitch = 1U;
    ctrl->motors_enabled = 1U;
    set_msg("Enable PIT");
  } else if (index == 4U) {
    ctrl->yaw_power_on = (uint8_t)!ctrl->yaw_power_on;
    ctrl->pitch_power_on = (uint8_t)!ctrl->pitch_power_on;
    ctrl->req_power_apply = 1U;
    set_msg("Power toggle");
  }
}

static void display_action(GimbalControlState *ctrl, uint8_t index)
{
  if (ctrl == 0) {
    return;
  }
  if (index == 0U) {
    ctrl->lcd_on = (uint8_t)!ctrl->lcd_on;
    if (ctrl->lcd_on) {
      LCD_BLK_Set(1);
      set_msg("LCD on");
    } else {
      LCD_BLK_Clr(0);
      set_msg("LCD off");
    }
  }
}

void GimbalMenu_Init(GimbalMenu *menu)
{
  (void)menu;
  page_stack[0] = PAGE_MAIN;
  selected[0] = 0U;
  page_depth = 0U;
  editing = EDIT_NONE;
  confirm = CONFIRM_NONE;
  line_dirty = 0U;
  dirty_old = 0U;
  dirty_new = 0U;
  render_step = 0U;
  status_update_step = 0U;
  clear_y = 0U;
  last_nav_ms = 0U;
  last_edit_ms = 0U;
  set_msg("Ready");
  redraw_needed = 1U;
}

void GimbalMenu_SetServiceCallback(GimbalMenu_ServiceCallback callback)
{
  service_callback = callback;
}

void GimbalMenu_RequestRedraw(void)
{
  redraw_needed = 1U;
  render_step = 0U;
  status_update_step = 0U;
  clear_y = 0U;
}

uint8_t GimbalMenu_NeedsRedraw(void)
{
  return (redraw_needed || line_dirty) ? 1U : 0U;
}

uint8_t GimbalMenu_IsStatusPage(void)
{
  return (current_page() == PAGE_STATUS) ? 1U : 0U;
}

uint8_t GimbalMenu_IsSquareCalibActive(void)
{
  MenuPage page = current_page();
  return (page == PAGE_SQUARE && square_calib_active) ? 1U : 0U;
}

void GimbalMenu_MarkRendered(void)
{
  redraw_needed = 0U;
  render_step = 0U;
  clear_y = 0U;
}

void GimbalMenu_HandleEvent(GimbalMenu *menu,
                            GimbalControlState *ctrl,
                            const GimbalKeyEvent *event,
                            uint32_t now_ms,
                            const DM_Motor_Info_Typedef *yaw,
                            const DM_Motor_Info_Typedef *pitch)
{
  MenuPage page;
  uint8_t count;
  (void)menu;

  if (event == 0 || event->type == GKEY_EVENT_NONE || ctrl == 0) {
    return;
  }

  page = current_page();

  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_VERY_LONG) {
    GimbalControl_EStop(ctrl);
    set_msg("E-STOP");
    return;
  }
  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_ZERO) {
    if (ctrl->mode == CTRL_MANUAL && page == PAGE_STATUS) {
      if (motor_online(yaw) == 0U || motor_online(pitch) == 0U) {
        set_msg("Motor offline");
        return;
      }
      ctrl->mode = CTRL_STOP;
      ctrl->req_save_zero_yaw = 1U;
      ctrl->req_save_zero_pitch = 1U;
      set_msg("Save both zero");
    }
    return;
  }
  if (event->key == GKEY_CENTER && event->type == GKEY_EVENT_LONG) {
    if (ctrl->mode == CTRL_MANUAL && page == PAGE_STATUS) {
      set_msg("Hold 3s zero");
      return;
    }
    GimbalControl_RequestHome(ctrl, now_ms);
    page_stack[0] = PAGE_MAIN;
    page_depth = 0U;
    set_msg("Long: HOME");
    return;
  }

  if (confirm != CONFIRM_NONE) {
    if (event->type == GKEY_EVENT_SHORT && event->key == GKEY_CENTER) {
      confirm_yes(ctrl);
    } else if (event->type == GKEY_EVENT_SHORT && event->key == GKEY_LEFT) {
      confirm = CONFIRM_NONE;
      ctrl->mode = CTRL_STOP;
      set_msg("Canceled");
    }
    return;
  }

  if (square_calib_active) {
    if (event->type != GKEY_EVENT_SHORT) {
      return;
    }
    if (event->key == GKEY_CENTER) {
      GimbalControl_SquareCaptureVertex(ctrl, square_calib_index);
      square_calib_index++;
      if (square_calib_index >= GIMBAL_SQUARE_VERTEX_COUNT) {
        square_calib_active = 0U;
        GimbalControl_SetMode(ctrl, CTRL_STOP, now_ms);
        set_msg("4 vertices saved");
      } else {
        ctrl->square_calib_index = square_calib_index;
        GimbalControl_SetMode(ctrl, CTRL_SQUARE_CALIB, now_ms);
        set_msg("Move next point");
      }
    } else if (event->key == GKEY_LEFT) {
      GimbalMenu_RequestRedraw();
    } else if (event->key == GKEY_RIGHT) {
      GimbalMenu_RequestRedraw();
    } else if (event->key == GKEY_UP) {
      GimbalMenu_RequestRedraw();
    } else if (event->key == GKEY_DOWN) {
      GimbalMenu_RequestRedraw();
    }
    return;
  }

  if (editing != EDIT_NONE) {
    if (event->type != GKEY_EVENT_SHORT) {
      return;
    }
    if (event->key == GKEY_UP || event->key == GKEY_RIGHT) {
      if ((now_ms - last_edit_ms) < MENU_EDIT_INTERVAL_MS) {
        return;
      }
      last_edit_ms = now_ms;
      edit_change(ctrl, 1);
      dirty_old = current_selected();
      dirty_new = current_selected();
      line_dirty = 1U;
    } else if (event->key == GKEY_DOWN || event->key == GKEY_LEFT) {
      if ((now_ms - last_edit_ms) < MENU_EDIT_INTERVAL_MS) {
        return;
      }
      last_edit_ms = now_ms;
      edit_change(ctrl, -1);
      dirty_old = current_selected();
      dirty_new = current_selected();
      line_dirty = 1U;
    } else if (event->key == GKEY_CENTER) {
      uint8_t trajectory_edit = (editing == EDIT_CIRCLE_PERIOD ||
                                 editing == EDIT_CIRCLE_YAW_RADIUS ||
                                 editing == EDIT_CIRCLE_PITCH_RADIUS ||
                                 editing == EDIT_SQUARE_EDGE) ? 1U : 0U;
      editing = EDIT_NONE;
      set_msg(trajectory_edit ? "Trajectory set" : "Param set");
    }
    return;
  }

  if (ctrl->mode == CTRL_MANUAL && page == PAGE_STATUS && event->type == GKEY_EVENT_SHORT) {
    if (event->key == GKEY_LEFT) {
      GimbalControl_AddManualYaw(ctrl, 1.0f);
      return;
    } else if (event->key == GKEY_RIGHT) {
      GimbalControl_AddManualYaw(ctrl, -1.0f);
      return;
    } else if (event->key == GKEY_UP) {
      GimbalControl_AddManualPitch(ctrl, -1.0f);
      return;
    } else if (event->key == GKEY_DOWN) {
      GimbalControl_AddManualPitch(ctrl, 1.0f);
      return;
    }
  }

  if (page == PAGE_STATUS) {
    if (event->type == GKEY_EVENT_SHORT && event->key == GKEY_CENTER) {
      page_stack[0] = PAGE_MAIN;
      selected[0] = 0U;
      redraw_needed = 1U;
      render_step = 0U;
      clear_y = 0U;
    }
    return;
  }

  if (event->type != GKEY_EVENT_SHORT) {
    return;
  }

  if (page == PAGE_LASER) {
    if (event->key == GKEY_LEFT) {
      GimbalLaser_SetEnabled(0U);
      set_msg("Laser off");
    } else if (event->key == GKEY_RIGHT) {
      GimbalLaser_SetEnabled(1U);
      set_msg("Laser on");
    } else if (event->key == GKEY_UP) {
      laser_change_brightness(1);
      set_msg("Brightness +");
    } else if (event->key == GKEY_DOWN) {
      laser_change_brightness(-1);
      set_msg("Brightness -");
    } else if (event->key == GKEY_CENTER) {
      pop_page();
    }
    return;
  }

  count = item_count(page);
  if (event->key == GKEY_UP && count > 0U) {
    if ((now_ms - last_nav_ms) < MENU_NAV_INTERVAL_MS) {
      return;
    }
    last_nav_ms = now_ms;
    dirty_old = current_selected();
    selected[page_depth] = (uint8_t)((current_selected() + count - 1U) % count);
    dirty_new = current_selected();
    line_dirty = 1U;
  } else if (event->key == GKEY_DOWN && count > 0U) {
    if ((now_ms - last_nav_ms) < MENU_NAV_INTERVAL_MS) {
      return;
    }
    last_nav_ms = now_ms;
    dirty_old = current_selected();
    selected[page_depth] = (uint8_t)((current_selected() + 1U) % count);
    dirty_new = current_selected();
    line_dirty = 1U;
  } else if (event->key == GKEY_LEFT) {
    pop_page();
  } else if (event->key == GKEY_RIGHT || event->key == GKEY_CENTER) {
    uint8_t index = current_selected();
    if (page == PAGE_MAIN) {
      if (index == 0U) {
        push_page(PAGE_RUN);
      } else if (index == 1U) {
        push_page(PAGE_TRAJECTORY);
      } else if (index == 2U) {
        push_page(PAGE_LASER);
      } else if (index == 3U) {
        push_page(PAGE_SETTINGS);
      }
    } else if (page == PAGE_RUN) {
      run_action(ctrl, index, now_ms);
    } else if (page == PAGE_TRAJECTORY) {
      if (index == 0U) {
        push_page(PAGE_CIRCLE);
      } else if (index == 1U) {
        push_page(PAGE_SQUARE);
      }
    } else if (page == PAGE_CIRCLE) {
      circle_action(ctrl, index, now_ms);
    } else if (page == PAGE_SETTINGS) {
      if (index == 0U) {
        push_page(PAGE_PARAM);
      } else if (index == 1U) {
        push_page(PAGE_MOTOR);
      } else if (index == 2U) {
        push_page(PAGE_DISPLAY);
      } else if (index == 3U) {
        push_page(PAGE_ABOUT);
      }
    } else if (page == PAGE_PARAM) {
      param_action(ctrl, index);
    } else if (page == PAGE_ZERO) {
      zero_action(ctrl, index, yaw, pitch);
    } else if (page == PAGE_SQUARE) {
      square_action(ctrl, index, now_ms);
    } else if (page == PAGE_MOTOR) {
      motor_action(ctrl, index);
    } else if (page == PAGE_DISPLAY) {
      display_action(ctrl, index);
    } else if (page == PAGE_ABOUT) {
      pop_page();
    }
  }
}

static uint8_t clear_screen_step(void)
{
  uint16_t y_end;

  if (clear_y >= LCD_H) {
    clear_y = 0U;
    return 1U;
  }

  y_end = (uint16_t)(clear_y + CLEAR_BAND_H);
  if (y_end > LCD_H) {
    y_end = LCD_H;
  }
  LCD_Fill(0, clear_y, LCD_W, y_end, BLACK);
  clear_y = y_end;
  menu_service();
  return 0U;
}

static void draw_text(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t size)
{
  LCD_ShowString(x, y, (const uint8_t *)s, fg, bg, size, 0);
}

static void draw_float(uint16_t x, uint16_t y, float v, uint16_t fg)
{
  LCD_ShowFloatNum(x, y, v, 6, 1, fg, BLACK, 16);
}

static void draw_status_group(uint8_t group,
                              const GimbalControlState *ctrl,
                              const DM_Motor_Info_Typedef *yaw,
                              const DM_Motor_Info_Typedef *pitch,
                              uint16_t key_adc,
                              uint8_t clear_bg)
{
  float yaw_d = 0.0f;
  float yaw_v = 0.0f;
  float pit_d = 0.0f;
  float pit_v = 0.0f;

  if (yaw != 0) {
    yaw_d = yaw->Data.Position * RAD2DEG;
    yaw_v = yaw->Data.Velocity * RAD2DEG;
  }
  if (pitch != 0) {
    pit_d = pitch->Data.Position * RAD2DEG;
    pit_v = pitch->Data.Velocity * RAD2DEG;
  }

  switch (group) {
    case 0U:
      if (clear_bg) {
        LCD_Fill(0, 0, LCD_W, 28, BLACK);
      }
      draw_text(0, 0, "GIMBAL MIT", WHITE, BLACK, 24);
      draw_text(180, 4, GimbalControl_ModeName(ctrl->mode), YELLOW, BLACK, 16);
      break;
    case 1U:
      if (clear_bg) {
        LCD_Fill(0, 32, LCD_W, 52, BLACK);
      }
      draw_text(0, 32, "YAW", CYAN, BLACK, 16);
      draw_text(40, 32, motor_online(yaw) ? "ON " : "OFF", motor_online(yaw) ? GREEN : RED, BLACK, 16);
      draw_text(86, 32, "P", WHITE, BLACK, 16);
      draw_float(102, 32, yaw_d, CYAN);
      break;
    case 2U:
      if (clear_bg) {
        LCD_Fill(184, 32, LCD_W, 52, BLACK);
      }
      draw_text(184, 32, "V", WHITE, BLACK, 16);
      draw_float(200, 32, yaw_v, WHITE);
      break;
    case 3U:
      if (clear_bg) {
        LCD_Fill(0, 55, LCD_W, 75, BLACK);
      }
      draw_text(0, 55, "PIT", MAGENTA, BLACK, 16);
      draw_text(40, 55, motor_online(pitch) ? "ON " : "OFF", motor_online(pitch) ? GREEN : RED, BLACK, 16);
      draw_text(86, 55, "P", WHITE, BLACK, 16);
      draw_float(102, 55, pit_d, MAGENTA);
      break;
    case 4U:
      if (clear_bg) {
        LCD_Fill(184, 55, LCD_W, 75, BLACK);
      }
      draw_text(184, 55, "V", WHITE, BLACK, 16);
      draw_float(200, 55, pit_v, WHITE);
      break;
    case 5U:
      if (clear_bg) {
        LCD_Fill(0, 84, LCD_W, 104, BLACK);
      }
      draw_text(0, 84, "KP", WHITE, BLACK, 16);
      draw_float(24, 84, ctrl->kp, YELLOW);
      draw_text(96, 84, "KD", WHITE, BLACK, 16);
      draw_float(120, 84, ctrl->kd, YELLOW);
      draw_text(192, 84, "T", WHITE, BLACK, 16);
      draw_float(208, 84, ctrl->torque_ff, YELLOW);
      break;
    case 6U:
      if (clear_bg) {
        LCD_Fill(0, 108, LCD_W, 128, BLACK);
      }
      draw_text(0, 108, "AMP", WHITE, BLACK, 16);
      draw_float(40, 108, ctrl->yaw_amp_deg, CYAN);
      draw_float(112, 108, ctrl->pitch_amp_deg, MAGENTA);
      draw_text(188, 108, "PER", WHITE, BLACK, 16);
      LCD_ShowIntNum(224, 108, ctrl->period_ms, 4, YELLOW, BLACK, 16);
      break;
    case 7U:
      if (clear_bg) {
        LCD_Fill(0, 138, LCD_W, 208, BLACK);
      }
      if (ctrl->mode == CTRL_MANUAL) {
        draw_text(0, 138, "CENTER3S SAVE ZERO", GRAY, BLACK, 16);
      } else {
        draw_text(0, 138, "CENTER:MENU  LONG:HOME", GRAY, BLACK, 16);
      }
      draw_text(0, 164, "MSG:", WHITE, BLACK, 16);
      draw_text(42, 164, status_msg, LBBLUE, BLACK, 16);
      draw_text(0, 188, "KEY ADC", GRAY, BLACK, 16);
      LCD_ShowIntNum(72, 188, key_adc, 4, GRAY, BLACK, 16);
      break;
    default:
      break;
  }
  menu_service();
}

static void draw_status_incremental(const GimbalControlState *ctrl,
                                    const DM_Motor_Info_Typedef *yaw,
                                    const DM_Motor_Info_Typedef *pitch,
                                    uint16_t key_adc)
{
  if (redraw_needed) {
    if (render_step == 0U) {
      if (clear_screen_step() == 0U) {
        return;
      }
      render_step++;
      return;
    }

    if (render_step >= 1U && render_step <= 8U) {
      draw_status_group((uint8_t)(render_step - 1U), ctrl, yaw, pitch, key_adc, 1U);
      render_step++;
      return;
    }

    redraw_needed = 0U;
    render_step = 0U;
    status_update_step = 0U;
    return;
  }

  draw_status_group(status_update_step, ctrl, yaw, pitch, key_adc, 0U);
  status_update_step++;
  if (status_update_step >= 8U) {
    status_update_step = 0U;
  }
}

static void draw_menu_line(MenuPage page, const GimbalControlState *ctrl, uint8_t index)
{
  char line[28];
  uint16_t y;
  uint16_t fg;
  uint16_t bg;
  const char *text;

  if (index >= item_count(page) || index >= ITEMS_MAX) {
    return;
  }

  y = (uint16_t)(34U + index * 23U);
  fg = (index == current_selected()) ? BLACK : WHITE;
  bg = (index == current_selected()) ? LBBLUE : BLACK;
  text = item_text(page, index);

  if (page == PAGE_PARAM) {
    if (index == 0U) {
      snprintf(line, sizeof(line), "KP        %.2f", ctrl->kp);
      text = line;
    } else if (index == 1U) {
      snprintf(line, sizeof(line), "KD        %.2f", ctrl->kd);
      text = line;
    } else if (index == 2U) {
      snprintf(line, sizeof(line), "Torque    %.2f", ctrl->torque_ff);
      text = line;
    } else if (index == 3U) {
      snprintf(line, sizeof(line), "Amp YAW   %.1f", ctrl->yaw_amp_deg);
      text = line;
    } else if (index == 4U) {
      snprintf(line, sizeof(line), "Amp PIT   %.1f", ctrl->pitch_amp_deg);
      text = line;
    } else if (index == 5U) {
      snprintf(line, sizeof(line), "Period    %ums", ctrl->period_ms);
      text = line;
    }
  } else if (page == PAGE_CIRCLE) {
    if (index == 1U) {
      snprintf(line, sizeof(line), "Circle Speed %ums", ctrl->period_ms);
      text = line;
    } else if (index == 2U) {
      snprintf(line, sizeof(line), "Radius YAW   %.1f", ctrl->yaw_amp_deg);
      text = line;
    } else if (index == 3U) {
      snprintf(line, sizeof(line), "Radius PIT   %.1f", ctrl->pitch_amp_deg);
      text = line;
    }
  } else if (page == PAGE_SQUARE) {
    if (index == 0U) {
      snprintf(line, sizeof(line), "%s  %u/4", text,
               (uint8_t)(ctrl->square_vertex_valid[0] +
                         ctrl->square_vertex_valid[1] +
                         ctrl->square_vertex_valid[2] +
                         ctrl->square_vertex_valid[3]));
      text = line;
    } else if (index == 1U) {
      snprintf(line, sizeof(line), "Edge Time    %ums", ctrl->square_edge_ms);
      text = line;
    }
  }

  LCD_Fill(0, y, LCD_W, (uint16_t)(y + 20U), bg);
  draw_text(4, y + 2U, text, fg, bg, 16);
  if (index == current_selected()) {
    draw_text(250, y + 2U, ">", fg, bg, 16);
  }
  menu_service();
}

static const char *page_title(MenuPage page)
{
  if (page == PAGE_RUN) {
    return "RUN";
  } else if (page == PAGE_TRAJECTORY) {
    return "TRAJECTORY";
  } else if (page == PAGE_CIRCLE) {
    return "CIRCLE";
  } else if (page == PAGE_SETTINGS) {
    return "SETTINGS";
  } else if (page == PAGE_PARAM) {
    return "PARAM";
  } else if (page == PAGE_ZERO) {
    return "ZERO";
  } else if (page == PAGE_SQUARE) {
    return "SQUARE";
  } else if (page == PAGE_LASER) {
    return "LASER";
  } else if (page == PAGE_MOTOR) {
    return "MOTOR";
  } else if (page == PAGE_DISPLAY) {
    return "DISPLAY";
  } else if (page == PAGE_ABOUT) {
    return "ABOUT";
  }
  return "MENU";
}

static void draw_menu_incremental(MenuPage page, const GimbalControlState *ctrl)
{
  uint8_t count = item_count(page);

  if (render_step == 0U) {
    if (clear_screen_step() == 0U) {
      return;
    }
    render_step++;
    return;
  }

  if (render_step == 1U) {
    draw_text(0, 0, page_title(page), WHITE, BLACK, 24);
    draw_text(170, 4, GimbalControl_ModeName(ctrl->mode), YELLOW, BLACK, 16);
    render_step++;
    return;
  }

  if (page == PAGE_ABOUT) {
    switch (render_step) {
      case 2U:
        draw_text(0, 34, "YAW ID 01 CAN1 PD0/PD1", CYAN, BLACK, 16);
        break;
      case 3U:
        draw_text(0, 58, "PIT ID 02 CAN2 PB5/PB6", MAGENTA, BLACK, 16);
        break;
      case 4U:
        draw_text(0, 82, "LCD 280x240 landscape", WHITE, BLACK, 16);
        break;
      case 5U:
        draw_text(0, 106, "KEY ADC PA5 rank2", WHITE, BLACK, 16);
        break;
      case 6U:
        draw_text(0, 130, "OUT1 PC14 OUT2 PC13", WHITE, BLACK, 16);
        break;
      case 7U:
        draw_text(0, 180, "LEFT back", GRAY, BLACK, 16);
        break;
      default:
        redraw_needed = 0U;
        render_step = 0U;
        return;
    }
    render_step++;
    return;
  }

  if (page == PAGE_LASER) {
    char line[28];
    uint8_t brightness = GimbalLaser_GetBrightness();
    switch (render_step) {
      case 2U:
        draw_text(0, 34, "Laser Control", YELLOW, BLACK, 24);
        break;
      case 3U:
        snprintf(line, sizeof(line), "STATE  %s", GimbalLaser_IsEnabled() ? "ON" : "OFF");
        draw_text(0, 70, line, GimbalLaser_IsEnabled() ? GREEN : RED, BLACK, 16);
        break;
      case 4U:
        snprintf(line, sizeof(line), "BRIGHT %u%%", brightness);
        draw_text(0, 96, line, CYAN, BLACK, 16);
        break;
      case 5U:
        LCD_DrawRectangle(0, 124, 202, 144, WHITE);
        LCD_Fill(2, 126, (uint16_t)(2U + brightness * 2U), 142, GREEN);
        break;
      case 6U:
        draw_text(0, 166, "LEFT OFF  RIGHT ON", WHITE, BLACK, 16);
        break;
      case 7U:
        draw_text(0, 190, "UP/DOWN BRIGHTNESS", WHITE, BLACK, 16);
        break;
      case 8U:
        draw_text(0, 214, "CENTER EXIT", GRAY, BLACK, 16);
        break;
      default:
        redraw_needed = 0U;
        render_step = 0U;
        return;
    }
    render_step++;
    return;
  }

  if (page == PAGE_SQUARE && square_calib_active) {
    char line[28];
    static const char *vertex_names[GIMBAL_SQUARE_VERTEX_COUNT] = {"LB", "RB", "RT", "LT"};
    switch (render_step) {
      case 2U:
        snprintf(line, sizeof(line), "SAVE %s P%u", vertex_names[square_calib_index], square_calib_index);
        draw_text(0, 34, line, YELLOW, BLACK, 16);
        break;
      case 3U:
        snprintf(line, sizeof(line), "YAW %.1f", ctrl->yaw_target_rad * RAD2DEG);
        draw_text(0, 58, line, CYAN, BLACK, 16);
        break;
      case 4U:
        snprintf(line, sizeof(line), "PIT %.1f", ctrl->pitch_target_rad * RAD2DEG);
        draw_text(0, 82, line, MAGENTA, BLACK, 16);
        break;
      case 5U:
        draw_text(0, 118, "ARROWS MOVE GIMBAL", WHITE, BLACK, 16);
        break;
      case 6U:
        draw_text(0, 142, "CENTER SAVE CURRENT", GREEN, BLACK, 16);
        break;
      case 7U:
        draw_text(0, 166, "ORDER LB RB RT LT", WHITE, BLACK, 16);
        break;
      default:
        redraw_needed = 0U;
        render_step = 0U;
        return;
    }
    render_step++;
    return;
  }

  if (render_step >= 2U && render_step < (uint8_t)(2U + count) && (render_step - 2U) < ITEMS_MAX) {
    draw_menu_line(page, ctrl, (uint8_t)(render_step - 2U));
    render_step++;
    return;
  }

  if (render_step == (uint8_t)(2U + count)) {
    if (editing != EDIT_NONE) {
      draw_text(0, 218, "EDIT UP/DN OK=CENTER", YELLOW, BLACK, 16);
    } else {
      draw_text(0, 218, "LEFT back  CENTER ok", GRAY, BLACK, 16);
    }
    render_step++;
    return;
  }

  redraw_needed = 0U;
  render_step = 0U;
}

void GimbalMenu_Render(GimbalMenu *menu,
                       const GimbalControlState *ctrl,
                       const DM_Motor_Info_Typedef *yaw,
                       const DM_Motor_Info_Typedef *pitch,
                       uint16_t key_adc)
{
  (void)menu;
  if (ctrl == 0) {
    return;
  }

  if (confirm != CONFIRM_NONE) {
    if (redraw_needed) {
      if (clear_screen_step() == 0U) {
        return;
      }
    }
    draw_text(0, 0, "SAVE ZERO?", YELLOW, BLACK, 24);
    draw_text(0, 40, "Stop trajectory first.", WHITE, BLACK, 16);
    draw_text(0, 64, "Place gimbal center.", WHITE, BLACK, 16);
    draw_text(0, 96, "CENTER = YES", GREEN, BLACK, 16);
    draw_text(0, 120, "LEFT   = NO", RED, BLACK, 16);
    draw_text(0, 170, status_msg, LBBLUE, BLACK, 16);
    redraw_needed = 0U;
    render_step = 0U;
    clear_y = 0U;
    line_dirty = 0U;
    return;
  }

  if (line_dirty && !redraw_needed && current_page() != PAGE_STATUS && confirm == CONFIRM_NONE) {
    draw_menu_line(current_page(), ctrl, dirty_old);
    if (dirty_new != dirty_old) {
      draw_menu_line(current_page(), ctrl, dirty_new);
    }
    line_dirty = 0U;
    return;
  }

  if (current_page() == PAGE_STATUS) {
    draw_status_incremental(ctrl, yaw, pitch, key_adc);
  } else {
    if (redraw_needed) {
      draw_menu_incremental(current_page(), ctrl);
      return;
    }
    return;
  }
  line_dirty = 0U;
}
