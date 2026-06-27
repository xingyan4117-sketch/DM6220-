#ifndef Motor_H
#define Motor_H

#include "bsp_can.h"
#include "stdbool.h"

typedef enum{

  Motor_Enable,
  Motor_Disable,
  Motor_Save_Zero_Position,
  DM_Motor_CMD_Type_Num,

}DM_Motor_CMD_e;

typedef enum{

  MIT_Mode,
  Position_Velocity_Mode,
  Velocity_Mode,
  DM_Motor_Mode_Type_Num,

}DM_Motor_Mode_e;


typedef struct 
{
  int16_t  State; 	
  uint16_t  P_int;
  uint16_t  V_int;
  uint16_t  T_int;
  float  Position;  
  float  Velocity;  
  float  Torque;  
  float  Temperature_MOS;   
  float  Temperature_Rotor;  
}DM_Motor_Data_Typedef;

typedef struct
{
  uint32_t Master_ID;   
  uint32_t CAN_ID;  
}Motor_CANFrameInfo_typedef;

typedef struct
{
	uint16_t ID;
  Motor_CANFrameInfo_typedef CANFrameInfo;
	DM_Motor_Data_Typedef Data;  
}DM_Motor_Info_Typedef;

typedef struct
{
	float  KP;
	float  KD;
	float  Position; 
  float  Velocity;  	
  float  Torque;  

}DM_Motor_Control_Typedef;

extern DM_Motor_Info_Typedef DM_Motor_Yaw;    /* 电机1 YAW   FDCAN1 CAN_ID=0x01 */
extern DM_Motor_Info_Typedef DM_Motor_Pitch;  /* 电机2 PITCH FDCAN2 CAN_ID=0x02 */
/* 兼容旧代码 */
#define DM_6220_Motor DM_Motor_Yaw

/* Axis direction mapping.
 * YAW is mechanically installed in reverse, so commands are negated here.
 * Keep trajectory/PID code in physical gimbal coordinates.
 */
#define YAW_INVERTED    1
#define PITCH_INVERTED  0

#define MOTOR_SEND_YAW(frame, motor, mode, pos, vel, kp, kd, trq) \
  DM_Motor_CAN_TxMessage((frame), (motor), (mode), \
                         (YAW_INVERTED ? -(pos) : (pos)), \
                         (YAW_INVERTED ? -(vel) : (vel)), \
                         (kp), (kd), \
                         (YAW_INVERTED ? -(trq) : (trq)))

#define MOTOR_SEND_PITCH(frame, motor, mode, pos, vel, kp, kd, trq) \
  DM_Motor_CAN_TxMessage((frame), (motor), (mode), \
                         (PITCH_INVERTED ? -(pos) : (pos)), \
                         (PITCH_INVERTED ? -(vel) : (vel)), \
                         (kp), (kd), \
                         (PITCH_INVERTED ? -(trq) : (trq)))

extern DM_Motor_Control_Typedef DM_Motor_Control;

extern void DM_Motor_Info_Update(uint8_t *rxBuf,DM_Motor_Info_Typedef *DM_Motor);

extern void DM_Motor_Multi_Info_Update(uint8_t *Data,DM_Motor_Info_Typedef *DM_Motor);

extern void DM_Motor_Command(FDCAN_TxFrame_TypeDef *TxFrame,uint16_t TxStdId,uint8_t CMD);

extern void DM_Motor_CAN_TxMessage(FDCAN_TxFrame_TypeDef *TxFrame,DM_Motor_Info_Typedef *DM_Motor,uint8_t Mode,
	                                             float Postion, float Velocity, float KP, float KD, float Torque);

#endif
