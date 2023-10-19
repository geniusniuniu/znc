#ifndef SRC_MYCODE_MYPWM_H_
#define SRC_MYCODE_MYPWM_H_

#include "LQ_MotorServo.h"
#include "LQ_GPIO.h"

extern char Stop_Flag;

void Motor_Init (void);
void Motor_SetDuty(IfxGtm_Atom_ToutMap MOTORx_P,int Duty);
void Motor_SetDir(GPIO_Name_t pin,uint8 Direction);
void Motor_Stop(void);



#endif /* SRC_MYCODE_MYPWM_H_ */
