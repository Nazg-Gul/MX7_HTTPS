/* 
 * File:   uartP32.h
 * Author: ppacheco
 *
 * Created on 7 de febrero de 2013, 16:41
 */

#ifndef UARTP32_H
#define	UARTP32_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <plib.h>

void openUART(UART_MODULE, int, int);
void putcUART(unsigned char);
void putsUART(char*);
unsigned char getWaitUART(void);
unsigned char getUART(void);
void putnbytesUART(unsigned char * s , int length);
void clearConsoleUART(void);
void returnHomeUART(void);
void closeUART(UART_MODULE uart_id);

#ifdef	__cplusplus
}
#endif

#endif	/* UARTP32_H */

