#include <stdio.h>
#include "LQ_STM.h"
#include "LQ_UART.h"
#include "PID.h"
#include "IfxAsclin_Asc.h"

//ï¿½ï¿½ï¿½Ô·ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
void BT_Test(void)
{
    if(UART_GetChar(UART0) == 'w') {
        Flag_Status = 0;
    }
    else if(UART_GetChar(UART0) == 'a') {
        Flag_Status = 2;
    }
    else if(UART_GetChar(UART0) == 'd') {
        Flag_Status = 4;
    }
    else if(UART_GetChar(UART0) == 's') {
        Flag_Status = 8;
    }
    else
        Flag_Status = 8;

}

//void UART0_RX_IRQHandler(void)
//{
//    IfxAsclin_Asc_isrReceive(&g_UartConfig[0]);
//
//    /* ÓÃ»§´úÂë */
//    BT_Test();
//    UART_PutChar(UART0, UART_GetChar(UART0));
//
//}










