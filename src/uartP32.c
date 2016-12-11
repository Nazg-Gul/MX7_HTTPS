#include "uartP32.h"

//Variable global donde se almacena el UART seleccionado
//con la funcion openUART
UART_MODULE uartSeleccionado;

////////////////////////////////////////////////////////////////////
// Funcion: openUART
//
// Inicializa el UART seleccionado
// Entradas:
//			uart_id = UART seleccionado. Ej: UART1, UART2, etc
//			Fpb : velocidad del PB clock. Ej: 80000000
//			velocidad: tasa de transferencia en baudios Ej: 9600
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
void openUART(UART_MODULE uart_id, int Fpb, int velocidad)
{
	uartSeleccionado = uart_id;
	UARTConfigure(uart_id, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetLineControl(uart_id, UART_DATA_SIZE_8_BITS |
    					UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(uart_id, Fpb, velocidad);
    UARTEnable(uart_id, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}

////////////////////////////////////////////////////////////////////
// Funcion: putcUART
//
// Transmite un byte por puerto serie
// Entrada:
//			dato: dato de 8 bits a enviar
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
void putcUART(unsigned char dato)
{
	while(!UARTTransmitterIsReady(uartSeleccionado));

	UARTSendDataByte(uartSeleccionado, dato);

}


////////////////////////////////////////////////////////////////////
// Funcion: putsUART
//
// Transmite una cadena por puerto serie
// Entrada:
//			s: cadena terminada en \0 a enviar. Ej: "Hola"
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
// Envia una cadena terminada en cero por el UART seleccionado
void putsUART(char *s)
{
	while(*s)			// cicla hasta que *s == '\0'
		putcUART( *s++);	// envia el caracter y apunta al siguiente
}

////////////////////////////////////////////////////////////////////
// Funcion: putnbytesUART
//
// Transmite un numero de bytes n por puerto serie
// Entrada:
//			s: cadena terminada en \0 a enviar. Ej: "Hola"
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
// Envia una cadena terminada en cero por el UART seleccionado
void putnbytesUART(unsigned char * s , int length)
{
	for(;length>0; length--)
            putcUART( *s++);	// envia el byte y apunta al siguiente
}

////////////////////////////////////////////////////////////////////
// Funcion: getUART
//
// Lee un dato del buffer de recepción del UART
//
// Entrada: Nada
//
// Salida:  dato de 8 bits recibido
////////////////////////////////////////////////////////////////////
unsigned char getUART(void)
{
	unsigned char dato;

	//Revisa si hay un dato en buffer de recepción
	if(UARTReceivedDataIsAvailable(uartSeleccionado))
		//Si hay dato, lo lee
		dato = UARTGetDataByte(uartSeleccionado);
	else
		//Si no hay dato regresará -1
		dato = -1;

	//Regresa dato leido o -1
	return dato;
}

////////////////////////////////////////////////////////////////////
// Funcion: getWaitUART
//
// Espera a que llegue un dato por puerto serie
// y lo regresa
//
// Entrada: Nada
//
// Salida:  dato de 8 bits recibido
////////////////////////////////////////////////////////////////////
unsigned char getWaitUART(void)
{
	unsigned char dato;

	//Espera a que llegue un dato del puerto serie
	while(!UARTReceivedDataIsAvailable(uartSeleccionado));

	//Lee el dato
    dato = UARTGetDataByte(uartSeleccionado);

	//Regresa dato leido
	return dato;
}


////////////////////////////////////////////////////////////////////
// Funcion: clearConsoleUART
//
// Limpia la pantalla del Hyperteminal
//
// Entrada: Nada
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
void clearConsoleUART(void)
{
	putsUART( "\x1b[2J");
}

////////////////////////////////////////////////////////////////////
// Funcion: returnHomeUART
//
// Regresa cursor al inicio de la pantalla del Hyperterminal
//
// Entrada: Nada
//
// Salida:  Nada
////////////////////////////////////////////////////////////////////
void returnHomeUART(void)
{
   	putsUART( "\x1b[1H");
}

void closeUART(UART_MODULE uart_id)
{
  UARTEnable(uart_id, UART_DISABLE);
}
