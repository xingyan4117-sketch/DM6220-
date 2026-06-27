#include "Motor.h"

/* ================================================================
 * 电机1 — YAW 轴，走 FDCAN1（PD0/PD1），CAN ID = 0x01
 * ================================================================ */
DM_Motor_Info_Typedef DM_Motor_Yaw = {
    .CANFrameInfo = {
        .CAN_ID    = 0x01,
        .Master_ID = 0x00,
    },
};

/* ================================================================
 * 电机2 — PITCH 轴，走 FDCAN2（PB5/PB6），CAN ID = 0x02
 * （两条独立总线）
 * ================================================================ */
DM_Motor_Info_Typedef DM_Motor_Pitch = {
    .CANFrameInfo = {
        .CAN_ID    = 0x02,
        .Master_ID = 0x00,
    },
};

/* 兼容旧代码：DM_6220_Motor 指向 YAW 电机 */
DM_Motor_Info_Typedef *const p_DM_6220_Motor = &DM_Motor_Yaw;
#define DM_6220_Motor DM_Motor_Yaw

#define P_MAX 12.5f
#define V_MAX 45.f
#define T_MAX 10.f


DM_Motor_Control_Typedef DM_Motor_Control;

static HAL_StatusTypeDef motor_try_send(FDCAN_TxFrame_TypeDef *TxFrame)
{
    if (TxFrame == 0 || TxFrame->hcan == 0) {
        return HAL_ERROR;
    }

    if (HAL_FDCAN_GetState(TxFrame->hcan) != HAL_FDCAN_STATE_BUSY) {
        return HAL_ERROR;
    }

    if (HAL_FDCAN_GetTxFifoFreeLevel(TxFrame->hcan) == 0U) {
        return HAL_BUSY;
    }

    return HAL_FDCAN_AddMessageToTxFifoQ(TxFrame->hcan, &TxFrame->Header, TxFrame->Data);
}

static float uint_to_float(int X_int, float X_min, float X_max, int Bits){
    float span = X_max - X_min;
    float offset = X_min;
    return ((float)X_int)*span/((float)((1<<Bits)-1)) + offset;
}

static int float_to_uint(float X_float, float X_min, float X_max, int bits){
    if (X_float < X_min) {
        X_float = X_min;
    } else if (X_float > X_max) {
        X_float = X_max;
    }
    float span = X_max - X_min;
    float offset = X_min;
    return (int) ((X_float-offset)*((float)((1<<bits)-1))/span);
}

void DM_Motor_Command(FDCAN_TxFrame_TypeDef *TxFrame,uint16_t TxStdId,uint8_t CMD){

	 TxFrame->Header.Identifier = TxStdId;
	
	 TxFrame->Data[0] = 0xFF;
   TxFrame->Data[1] = 0xFF;
 	 TxFrame->Data[2] = 0xFF;
	 TxFrame->Data[3] = 0xFF;
	 TxFrame->Data[4] = 0xFF;
	 TxFrame->Data[5] = 0xFF;
	 TxFrame->Data[6] = 0xFF;
	
	 switch(CMD){
		 
		  case Motor_Enable :
	        TxFrame->Data[7] = 0xFC; 
	    break;
      
			case Motor_Disable :
	        TxFrame->Data[7] = 0xFD; 
      break;
      
			case Motor_Save_Zero_Position :
	        TxFrame->Data[7] = 0xFE; 
			break;
			
			default:
	    break;   
	}
	
   (void)motor_try_send(TxFrame);

}

void DM_Motor_CAN_TxMessage(FDCAN_TxFrame_TypeDef *TxFrame,DM_Motor_Info_Typedef *DM_Motor,uint8_t Mode,float Postion, float Velocity, float KP, float KD, float Torque){

	 if(Mode > Velocity_Mode) Mode = MIT_Mode;	 
	 
	 if(Mode == MIT_Mode) {
		
			 uint16_t Postion_Tmp,Velocity_Tmp,Torque_Tmp,KP_Tmp,KD_Tmp;
			 
			 Postion_Tmp  =  float_to_uint(Postion,-P_MAX,P_MAX,16) ;
			 Velocity_Tmp =  float_to_uint(Velocity,-V_MAX,V_MAX,12);
			 Torque_Tmp = float_to_uint(Torque,-T_MAX,T_MAX,12);
			 KP_Tmp = float_to_uint(KP,0,500,12);
			 KD_Tmp = float_to_uint(KD,0,5,12);

			 TxFrame->Header.Identifier = DM_Motor->CANFrameInfo.CAN_ID;
			 
			 TxFrame->Data[0] = (uint8_t)(Postion_Tmp>>8);
			 TxFrame->Data[1] = (uint8_t)(Postion_Tmp);
			 TxFrame->Data[2] = (uint8_t)(Velocity_Tmp>>4);
			 TxFrame->Data[3] = (uint8_t)((Velocity_Tmp&0x0F)<<4) | (uint8_t)(KP_Tmp>>8);
			 TxFrame->Data[4] = (uint8_t)(KP_Tmp);
			 TxFrame->Data[5] = (uint8_t)(KD_Tmp>>4);
			 TxFrame->Data[6] = (uint8_t)((KD_Tmp&0x0F)<<4) | (uint8_t)(Torque_Tmp>>8);
			 TxFrame->Data[7] = (uint8_t)(Torque_Tmp);
			
			 (void)motor_try_send(TxFrame);
	 }else if(Mode == Position_Velocity_Mode){
	 
		   KP = 0; KD = 0; Torque = 0;
		 
       uint8_t *Postion_Tmp,*Velocity_Tmp;
		   
		   Postion_Tmp = (uint8_t *)&Postion; 
		   Velocity_Tmp = (uint8_t *)&Velocity; 
		 
	     TxFrame->Header.Identifier = DM_Motor->CANFrameInfo.CAN_ID + 0x100;
			 
			 TxFrame->Data[0] = *(Postion_Tmp);
			 TxFrame->Data[1] = *(Postion_Tmp + 1);
			 TxFrame->Data[2] = *(Postion_Tmp + 2);
			 TxFrame->Data[3] = *(Postion_Tmp + 3);
			 TxFrame->Data[4] = *(Velocity_Tmp);
			 TxFrame->Data[5] = *(Velocity_Tmp + 1);
			 TxFrame->Data[6] = *(Velocity_Tmp + 2);
			 TxFrame->Data[7] = *(Velocity_Tmp + 3);
			
			 (void)motor_try_send(TxFrame);
	 
	 }else if(Mode == Velocity_Mode){
	 
	     Postion = 0;KP = 0; KD = 0; Torque = 0;
		 
       uint8_t *Velocity_Tmp;
		   
		   Velocity_Tmp = (uint8_t *)&Velocity; 
		 
	     TxFrame->Header.Identifier = DM_Motor->CANFrameInfo.CAN_ID + 0x200;
			 
			 TxFrame->Data[0] = *(Velocity_Tmp);
			 TxFrame->Data[1] = *(Velocity_Tmp + 1);
			 TxFrame->Data[2] = *(Velocity_Tmp + 2);
			 TxFrame->Data[3] = *(Velocity_Tmp + 3);
			 TxFrame->Data[4] = 0;
			 TxFrame->Data[5] = 0;
			 TxFrame->Data[6] = 0;
			 TxFrame->Data[7] = 0;
			
			 (void)motor_try_send(TxFrame);
	 
	 }
	 
	 
}



void DM_Motor_Info_Update(uint8_t *Data,DM_Motor_Info_Typedef *DM_Motor)
{
		
	  DM_Motor->Data.State = Data[0]>>4;
		DM_Motor->Data.P_int = ((uint16_t)(Data[1]) <<8) | ((uint16_t)(Data[2]));
		DM_Motor->Data.V_int = ((uint16_t)(Data[3]) <<4) | ((uint16_t)(Data[4])>>4);
		DM_Motor->Data.T_int = ((uint16_t)(Data[4]&0xF) <<8) | ((uint16_t)(Data[5]));
		DM_Motor->Data.Torque=  uint_to_float(DM_Motor->Data.T_int,-T_MAX,T_MAX,12);
		DM_Motor->Data.Position=uint_to_float(DM_Motor->Data.P_int,-P_MAX,P_MAX,16);
    DM_Motor->Data.Velocity=uint_to_float(DM_Motor->Data.V_int,-V_MAX,V_MAX,12);
    DM_Motor->Data.Temperature_MOS   = (float)(Data[6]);
		DM_Motor->Data.Temperature_Rotor = (float)(Data[7]);


}	 
