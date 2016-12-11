#include <plib.h>
#include <stdint.h>
#include "Delay.h"
#include "chipKITEthernetAPI.h"
#include "uartP32.h"

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>

#define ETHERNET_DEFAULT_TIMEOUT   5   // in seconds

/*
 *  Main Clock       -> SYSCLK = (Crystal_Freq / FPLLIDIV) * FPLLMUL / FPLLODIV
 *  Peripheral Clock -> PBCLK  = SYSCLK / FPBDIV
 *
 *  SYSCLK = 80MHz, PBCLK = 40MHz
 */
#pragma config FNOSC     = PRIPLL            // Oscillator Selection
#pragma config FPLLIDIV  = DIV_2             // PLL Input Divider
#pragma config FPLLMUL   = MUL_20            // PLL Multiplier
#pragma config FPLLODIV  = DIV_1             // PLL Output Divider
#pragma config FPBDIV    = DIV_2             // Peripheral Clock divisor
#pragma config FWDTEN    = OFF               // Watchdog Timer
#pragma config WDTPS     = PS32768           // Watchdog Timer Postscale
#pragma config FCKSM     = CSECMD            // Clock Switching & Fail Safe Clock Monitor
#pragma config OSCIOFNC  = OFF               // CLKO Enable
#pragma config POSCMOD   = HS                // Primary Oscillator
#pragma config IESO      = OFF               // Internal/External Switch-over
#pragma config FSOSCEN   = OFF               // Secondary Oscillator Enable
#pragma config CP        = OFF               // Code Protect
#pragma config BWP       = OFF               // Boot Flash Write Protect
#pragma config PWP       = OFF               // Program Flash Write Protect
/* Ethernet configuration. */
#pragma config FETHIO=ON
#pragma config FMIIEN=OFF

/* Configure the microcontroller for use with the on-board (MX7) licensed
 * debugger circuit.
 */
#pragma config ICESEL = ICS_PGx1

/* Microcontroller MIPs (FCY)/ */
#define SYS_FREQ 80000000L  //80 Mhz
#define FCY      SYS_FREQ

#define PB_DIV                 2

uint8_t mac[] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x24 };
uint8_t ip[] = { 192,168,3,9 };
uint8_t gateway[] = { 192,168,3, 1 };
uint8_t subnet[] = { 255, 255, 255, 0 };
uint8_t dns1[] = { 192, 168, 3, 14 };
uint8_t dns2[] = { 8, 8, 8, 8 };

uint8_t server_ip[] = { 82,94,226,100 };
uint16_t server_port = 443;

unsigned char cert_data1[] =  {
 48,130,  4, 29, 48,130,  3,  5,160,  3,  2,  1,  2,  2, 16, 78,129, 45,138,130,101,224, 11,  2,238, 62, 53,  2, 70,229, 61, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  5,  5,  0, 48,129,129, 49, 11, 48,  9,  6,  3, 85,  4,  6, 19,  2, 71, 66, 49, 27, 48, 25,  6,  3, 85,  4,  8, 19, 18, 71,114,101, 97,116,101,114, 32, 77, 97,110, 99,104,101,115,116,101,114, 49, 16, 48, 14,  6,  3, 85,  4,  7, 19,  7, 83, 97,108,102,111,114,100, 49, 26, 48, 24,  6,  3, 85,  4, 10, 19, 17, 67, 79, 77, 79, 68, 79, 32, 67, 65, 32, 76,105,109,105,116,101,100, 49, 39, 48, 37,  6,  3, 85,  4,  3, 19, 30, 67, 79, 77, 79, 68, 79, 32, 67,101,114,116,105,102,105, 99, 97,116,105,111,110, 32, 65,117,116,104,111,114,105,116,121, 48, 30, 23, 13, 48, 54, 49, 50, 48, 49, 48, 48, 48, 48, 48, 48, 90, 23, 13, 50, 57, 49, 50, 51, 49, 50, 51, 53, 57, 53, 57, 90, 48,129,129, 49, 11, 48,  9,  6,  3, 85,  4,  6, 19,  2, 71, 66, 49, 27, 48, 25,  6,  3, 85,  4,  8, 19, 18, 71,114,101, 97,116,101,114, 32, 77, 97,110, 99,104,101,115,116,101,114, 49,
 16, 48, 14,  6,  3, 85,  4,  7, 19,  7, 83, 97,108,102,111,114,100, 49, 26, 48, 24,  6,  3, 85,  4, 10, 19, 17, 67, 79, 77, 79, 68, 79, 32, 67, 65, 32, 76,105,109,105,116,101,100, 49, 39, 48, 37,  6,  3, 85,  4,  3, 19, 30, 67, 79, 77, 79, 68, 79, 32, 67,101,114,116,105,102,105, 99, 97,116,105,111,110, 32, 65,117,116,104,111,114,105,116,121, 48,130,  1, 34, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  1,  5,  0,  3,130,  1, 15,  0, 48,130,  1, 10,  2,130,  1,  1,  0,208, 64,139,139,114,227,145, 27,247, 81,193, 27, 84,  4,152,211,169,191,193,230,138, 93, 59,135,251,187,136,206, 13,227, 47, 63,  6,150,240,162, 41, 80,153,174,219, 59,161, 87,176,116, 81,113,205,237, 66,145, 77, 65,254,169,200,216,106,134,119, 68,187, 89,102,151, 80, 94,180,212, 44,112, 68,207,218, 55,149, 66,105, 60, 48,196,113,179, 82,240, 33, 77,161,216,186, 57,124, 28,158,163, 36,157,242,131, 22,152,170, 22,124, 67,155, 21, 91,183,174, 52,145,254,212, 98, 38, 24, 70,154, 63,235,193,249,241,144, 87,235,172,122, 13,139,219,114, 48,106,102,
213,224, 70,163,112,220,104,217,255,  4, 72,137,119,222,181,233,251,103,109, 65,233,188, 57,189, 50,217, 98,  2,241,177,168, 61,110, 55,156,226, 47,226,211,162, 38,139,198,184, 85, 67,136,225, 35, 62,165,210, 36, 57,106, 71,171,  0,212,161,179,169, 37,254, 13, 63,167, 29,186,211, 81,193, 11,164,218,172, 56,239, 85, 80, 36,  5,101, 70,147, 52, 79, 45,141,173,198,212, 33, 25,210,142,202,  5, 97,113,  7,115, 71,229,138, 25, 18,189,  4, 77,206, 78,156,165, 72,172,187, 38,247,  2,  3,  1,  0,  1,163,129,142, 48,129,139, 48, 29,  6,  3, 85, 29, 14,  4, 22,  4, 20, 11, 88,229,139,198, 76, 21, 55,164, 64,169, 48,169, 33,190, 71, 54, 90, 86,255, 48, 14,  6,  3, 85, 29, 15,  1,  1,255,  4,  4,  3,  2,  1,  6, 48, 15,  6,  3, 85, 29, 19,  1,  1,255,  4,  5, 48,  3,  1,  1,255, 48, 73,  6,  3, 85, 29, 31,  4, 66, 48, 64, 48, 62,160, 60,160, 58,134, 56,104,116,116,112, 58, 47, 47, 99,114,108, 46, 99,111,109,111,100,111, 99, 97, 46, 99,111,109, 47, 67, 79, 77, 79, 68, 79, 67,101,114,116,105,102,105, 99, 97,116,105,111,110,
 65,117,116,104,111,114,105,116,121, 46, 99,114,108, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  5,  5,  0,  3,130,  1,  1,  0, 62,152,158,155,246, 27,233,215, 57,183,120,174, 29,114, 24, 73,211,135,228, 67,130,235, 63,201,170,245,168,181,239, 85,124, 33, 82,101,249,213, 13,225,108,244, 62,140,147,115,145, 46,  2,196, 78,  7,113,111,192,143, 56, 97,  8,168, 30,129, 10,192, 47, 32, 47, 65,139,145,220, 72, 69,188,241,198,222,186,118,107, 51,200,  0, 45, 49, 70, 76,237,231,157,207,136,148,255, 51,192, 86,232, 36,134, 38,184,216, 56, 56,223, 42,107,221, 18,204,199, 63, 71, 23, 76,162,194,  6,150,  9,214,219,254, 63, 60, 70, 65,223, 88,226, 86, 15, 60, 59,193, 28,147, 53,217, 56, 82,172,238,200,236, 46, 48, 78,148, 53,180, 36, 31, 75,120,105,218,242,  2, 56,204,149, 82,147,240,112, 37, 89,156, 32,103,196,238,249,139, 87, 97,244,146,118,125, 63,132,141, 85,183,232,229,172,213,241,245, 25, 86,166, 90,251,144, 28,175,147,235,229, 28,212,103,151, 93,  4, 14,190, 11,131,166, 23,131,185, 48, 18,160,197, 51, 21,  5,
185, 13,251,199,  5,118,227,216, 74,141,252, 52, 23,163,198, 33, 40,190, 48, 69, 49, 30,199,120,190, 88, 97, 56,172, 59,226,  1,101,0
};

unsigned char cert_data2[] = {
 48,130,  4, 54, 48,130,  3, 30,160,  3,  2,  1,  2,  2,  1,  1, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  5,  5,  0, 48,111, 49, 11, 48,  9,  6,  3, 85,  4,  6, 19,  2, 83, 69, 49, 20, 48, 18,  6,  3, 85,  4, 10, 19, 11, 65,100,100, 84,114,117,115,116, 32, 65, 66, 49, 38, 48, 36,  6,  3, 85,  4, 11, 19, 29, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 84, 84, 80, 32, 78,101,116,119,111,114,107, 49, 34, 48, 32,  6,  3, 85,  4,  3, 19, 25, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 67, 65, 32, 82,111,111,116, 48, 30, 23, 13, 48, 48, 48, 53, 51, 48, 49, 48, 52, 56, 51, 56, 90, 23, 13, 50, 48, 48, 53, 51, 48, 49, 48, 52, 56, 51, 56, 90, 48,111, 49, 11, 48,  9,  6,  3, 85,  4,  6, 19,  2, 83, 69, 49, 20, 48, 18,  6,  3, 85,  4, 10, 19, 11, 65,100,100, 84,114,117,115,116, 32, 65, 66, 49, 38, 48, 36,  6,  3, 85,  4, 11, 19, 29, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 84, 84, 80, 32, 78,101,116,119,111,114,107, 49, 34, 48,
 32,  6,  3, 85,  4,  3, 19, 25, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 67, 65, 32, 82,111,111,116, 48,130,  1, 34, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  1,  5,  0,  3,130,  1, 15,  0, 48,130,  1, 10,  2,130,  1,  1,  0,183,247, 26, 51,230,242,  0,  4, 45, 57,224, 78, 91,237, 31,188,108, 15,205,181,250, 35,182,206,222,155, 17, 51,151,164, 41, 76,125,147,159,189, 74,188,147,237,  3, 26,227,143,207,229,109, 80, 90,214,151, 41,148, 90,128,176, 73,122,219, 46,149,253,184,202,191, 55, 56, 45, 30, 62,145, 65,173,112, 86,199,240, 79, 63,232, 50,158,116,202,200,144, 84,233,198, 95, 15,120,157,154, 64, 60, 14,172, 97,170, 94, 20,143,158,135,161,106, 80,220,215,154, 78,175,  5,179,166,113,148,156,113,179, 80, 96, 10,199, 19,157, 56,  7,134,  2,168,233,168,105, 38, 24,144,171, 76,176, 79, 35,171, 58, 79,132,216,223,206,159,225,105,111,187,215, 66,215,107, 68,228,199,173,238,109, 65, 95,114, 90,113,  8, 55,179,121,101,164, 89,160,148, 55,247,  0, 47, 13,194,146,114,218,208, 56,
114,219, 20,168, 69,196, 93, 42,125,183,180,214,196,238,172,205, 19, 68,183,201, 43,221, 67,  0, 37,250, 97,185,105,106, 88, 35, 17,183,167, 51,143, 86,117, 89,245,205, 41,215, 70,183, 10, 43,101,182,211, 66,111, 21,178,184,123,251,239,233, 93, 83,213, 52, 90, 39,  2,  3,  1,  0,  1,163,129,220, 48,129,217, 48, 29,  6,  3, 85, 29, 14,  4, 22,  4, 20,173,189,152,122, 52,180, 38,247,250,196, 38, 84,239,  3,189,224, 36,203, 84, 26, 48, 11,  6,  3, 85, 29, 15,  4,  4,  3,  2,  1,  6, 48, 15,  6,  3, 85, 29, 19,  1,  1,255,  4,  5, 48,  3,  1,  1,255, 48,129,153,  6,  3, 85, 29, 35,  4,129,145, 48,129,142,128, 20,173,189,152,122, 52,180, 38,247,250,196, 38, 84,239,  3,189,224, 36,203, 84, 26,161,115,164,113, 48,111, 49, 11, 48,  9,  6,  3, 85,  4,  6, 19,  2, 83, 69, 49, 20, 48, 18,  6,  3, 85,  4, 10, 19, 11, 65,100,100, 84,114,117,115,116, 32, 65, 66, 49, 38, 48, 36,  6,  3, 85,  4, 11, 19, 29, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 84, 84, 80, 32, 78,101,116,119,111,114,107, 49,
 34, 48, 32,  6,  3, 85,  4,  3, 19, 25, 65,100,100, 84,114,117,115,116, 32, 69,120,116,101,114,110, 97,108, 32, 67, 65, 32, 82,111,111,116,130,  1,  1, 48, 13,  6,  9, 42,134, 72,134,247, 13,  1,  1,  5,  5,  0,  3,130,  1,  1,  0,176,155,224,133, 37,194,214, 35,226, 15,150,  6,146,157, 65,152,156,217,132,121,129,217, 30, 91, 20,  7, 35, 54,101,143,176,216,119,187,172, 65,108, 71, 96,131, 81,176,249, 50, 61,231,252,246, 38, 19,199,128, 22,165,191, 90,252,135,207,120,121,137, 33,154,226, 76,  7, 10,134, 53,188,242,222, 81,196,210,150,183,220,126, 78,238,112,253, 28, 57,235, 12,  2, 81, 20, 45,142,189, 22,224,193,223, 70,117,231, 36,173,236,244, 66,180,133,147,112, 16,103,186,157,  6, 53, 74, 24,211, 43,122,204, 81, 66,161,122, 99,209,230,187,161,197, 43,194, 54,190, 19, 13,230,189, 99,126,121,123,167,  9, 13, 64,171,106,221,143,138,195,246,246,140, 26, 66,  5, 81,212, 69,245,159,167, 98, 33,104, 21, 32, 67, 60,153,231,124,189, 36,216,169,145, 23,115,136, 63, 86, 27, 49, 56, 24,180,113, 15,154,205,200, 14,158,
142, 46, 27,225,140,152,131,203, 31, 49,241, 68, 76,198,  4,115, 73,118, 96, 15,199,248,189, 23,128,107, 46,233,204, 76, 14, 90,154,121, 15, 32, 10, 46,213,158, 99, 38, 30, 85,146,148,216,130, 23, 90,123,208,188,199,143, 78,134,  4,0
};

uint8_t client_connect(uint8_t *ip, uint16_t port)  {
  //ChipKITClientConnectDNS(_szURL, _port, _cSecTimeout);
  return ChipKITClientConnectIP(ip, port, ETHERNET_DEFAULT_TIMEOUT);
}

void logging_cb(const int level, const char *const message)
{
  putsUART(message);
  putsUART("\r\n");
}

int sock_recv(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
  int sock = *(int*)ctx;
  if(!ChipKITClientAvailable(sock)) {
    //putsUART("Waiting data to come from server\r\n");
    while (!ChipKITClientAvailable(sock)) {
    }
  }
  //putsUART("Receiving buffer from server\r\n");
  return ChipKITClientGetBuff(sock, buf, sz);
}

int sock_send(WOLFSSL *ssl, char *buf, int sz, void *ctx)
{
  int sock = *(int*)ctx;
  int sent;
  //putsUART("Sending buffer to server\r\n");
  sent = ChipKITClientPutBuff(sock, buf, sz, ETHERNET_DEFAULT_TIMEOUT);
  if (sent != sz) {
    //putsUART("Error sending buffer!\r\n");
  }
  return sent;
}

void error() {
	unsigned int tcfg;
	tcfg = T1_ON|T1_IDLE_CON|T1_SOURCE_INT|T1_PS_1_8|T1_GATE_OFF|T1_SYNC_EXT_OFF;
	OpenTimer1(tcfg, 0xffff);
	while (1) 	{
    PORTWrite(IOPORT_G, 0);
		DelayMs(100);
		PORTWrite(IOPORT_G, BIT_12|BIT_13|BIT_14|BIT_15);
    DelayMs(100);
	}
}

void happy() {
	unsigned int tcfg;
	tcfg = T1_ON|T1_IDLE_CON|T1_SOURCE_INT|T1_PS_1_8|T1_GATE_OFF|T1_SYNC_EXT_OFF;
	OpenTimer1(tcfg, 0xffff);
	while (1) 	{
		//drive LD1 high
		PORTWrite (IOPORT_G, BIT_12);
		DelayMs(10);
		//drive LD2 high
		PORTWrite (IOPORT_G, BIT_13);
		DelayMs(10);
		//drive LD3 high
		PORTWrite (IOPORT_G, BIT_14);
		DelayMs(10);
		//drive LD4 high
		PORTWrite (IOPORT_G, BIT_15);
		DelayMs(10);
	}
}

void disable_modules() {
  C1CONbits.ON = 0; /* CAN */
  PMCONbits.ON = 0;  /* Parallel Master Port */
  AD1CON1bits.ON = 0;  /* ADC */
  CM1CONbits.ON = 0;  /* COMPARATOR REGISTER MAP */
  CM2CONbits.ON = 0;

  /* UART Modules */
  U1MODEbits.ON = 0;
  U2MODEbits.ON = 0;
  U3MODEbits.ON = 0;
  U4MODEbits.ON = 0;
  U5MODEbits.ON = 0;
  U6MODEbits.ON = 0;

  /* Serial Peripheral Interface */
  SPI1CONbits.ON = 0;
  SPI2CONbits.ON = 0;
  SPI3CONbits.ON = 0;
  SPI4CONbits.ON = 0;

  /* Inter-Integrated Circuit */
  I2C1CONbits.ON = 0;
  I2C2CONbits.ON = 0;
  I2C3CONbits.ON = 0;
  I2C4CONbits.ON = 0;
  I2C5CONbits.ON = 0;

  /* USB */
  U1PWRCbits.USUSPEND = 1;  /* put USB in suspend mode */
  U1PWRCbits.USBPWR   = 0;  /* disable USB power */
  U1CONbits.USBEN     = 0;  /* disable entire USB module */

  DMACONbits.ON = 0;  /* Direct Memory Access functions. */

  /* Programming and Diagnostics */
  DDPCONbits.JTAGEN = 0;
  DDPCONbits.TDOEN = 0;
  DDPCONbits.TROEN = 0;

  /* Pull all output pins low. */
  //TRISA = 0x0000;
  //TRISB = 0x0000;
  //TRISC = 0x0000;
  //TRISD = 0x0000;
  //TRISE = 0x0000;
  //TRISG = 0x0000;

  //PORTA = 0x0000;
  //PORTB = 0x0000;
  //PORTC = 0x0000;
  //PORTD = 0x0000;
  //PORTE = 0x0000;
  //PORTF = 0x0000;
  PORTG = 0x0000;
}

void device_sleep() {
  disable_modules();
  WDTCONbits.WDTCLR = 1;
  asm volatile("wait");
}

int main() {
  const char get_request[] =
      "GET /maniphest/project/2/type/Bug/query/open/ HTTP/1.0\r\n"
      "Host: developer.blender.org\r\n"
      "User-Agent: Hackerman\r\n"
      "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
      "Accept-Language: en-US,en;q=0.5\r\n"
      "Connection: keep-close\r\n"
      "Pragma: no-cache\r\n"
      "Cache-Control: no-cache\r\n"
      "\r\n";
  char buffer[1024];
  TCP_SOCKET sock;
  WOLFSSL_CTX *ctx;
  WOLFSSL *ssl;
  int led = 0;
  unsigned int ret;
  int i;

  /* General initialization. */
  DelayMs(100);
  INTEnableSystemMultiVectoredInt();
  PORTSetPinsDigitalOut(IOPORT_G, BIT_12|BIT_13| BIT_14|BIT_15);
  PORTWrite(IOPORT_G, BIT_15);

  /* open the serial port. */
  openUART(UART1,SYS_FREQ/PB_DIV , 115200);
  putsUART("====================\r\n");
  putsUART("Welcome!\r\n");

  /* Initialize SSL. */
  wolfSSL_SetLoggingCb(logging_cb);
  //wolfSSL_Debugging_ON();
  wolfSSL_Init();

  /* Create and initiLize WOLFSSL_CTX structure. */
  if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method())) == NULL) {
    putsUART("Error creating wolfSSL context\r\n");
    error();
  }
  putsUART("wolfSSL initialized\r\n");
  wolfSSL_SetIORecv(ctx, sock_recv);
  wolfSSL_SetIOSend(ctx, sock_send);

  /* Load CA certificates into wolfSSL_CTX. which will verify the server. */
  // if (wolfSSL_CTX_load_verify_locations(ctx, cert, "/etc/ssl/certs") != SSL_SUCCESS) {
  //  error()
  //}
  if (wolfSSL_CTX_load_verify_buffer(ctx, cert_data1, sizeof(cert_data1), SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
    putsUART("Error loading certificate.\r\n");
    error();
  }
  if (wolfSSL_CTX_load_verify_buffer(ctx, cert_data2, sizeof(cert_data2), SSL_FILETYPE_ASN1) != SSL_SUCCESS) {
    putsUART("Error loading certificate.\r\n");
    error();
  }
  if ((ssl = wolfSSL_new(ctx)) == NULL) {
    putsUART("Error creating wolfSSL\r\n");
    error();
  }
  putsUART("wolfSSL successfully created\r\n");

  /* Initialize the Ethernet controller and wait for it to come up. */
  putsUART("Initializing ethernet controller\r\n");
  PORTWrite(IOPORT_G, BIT_15);
  ChipKITEthernetBegin(mac, ip, gateway, subnet, dns1, dns2);
  PORTWrite(IOPORT_G, BIT_14);
  putsUART("Ethernet controller initialized, waiting a bit...\r\n");
  /* TODO(sergey): Find a nicer way to ensure controller is up and running. */
  DelayMs(1000);

  /* Connect to the server. */
    PORTWrite(IOPORT_G, BIT_13);
  for (i = 0; i < 5; ++i) {
    putsUART("Connecting to server...\r\n");
    sock = client_connect(server_ip, server_port);
    if (sock != INVALID_SOCKET) {
      break;
    }
  }
  if (sock == INVALID_SOCKET) {
    putsUART("Error connecting to server\r\n");
    error();
  }
  putsUART("Connected to server\r\n");

  /* Setup SSL over the connected socket. */
  wolfSSL_set_fd(ssl, sock);

  putsUART("Establishing secure connection...\r\n");
  ret = wolfSSL_connect(ssl);
  if (ret != SSL_SUCCESS) {
    char buf[10];
    itoa(buf, ret, 10);
    putsUART("Error establishing SSL: ");
    putsUART(buf);
    putsUART("\r\n");
    error();
  }
  putsUART("SSL is ready!\r\n");

  /* Send HTTP request to the server. */
  led = BIT_12;
  PORTWrite(IOPORT_G, led);
  putsUART("Sending GET request...\r\n");
  if (wolfSSL_write(ssl, get_request, strlen(get_request)) != strlen(get_request)) {
    putsUART("Request send failed\r\n");
    error();
  }
  putsUART("Request sent, waiting respond\r\n");
  led |= BIT_13;
  PORTWrite(IOPORT_G, led);
  while ((ret = wolfSSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
    char *tasks_token;
    buffer[ret] = '\0';
    tasks_token = strstr(buffer, "total tasks");
    if(tasks_token >= buffer - 5) {
      char *start = strchr(tasks_token - 5, '>');
      char *end = strchr(tasks_token - 5, ' ');
      char count[5];
      strncpy(count, start + 1, end - start - 1);
      count[end - start - 1] = '\0';
      putsUART("Number of tasks: ");
      putsUART(count);
      putsUART("\r\n");
    }
    //putsUART(buffer);
    led |= BIT_14;
    PORTWrite(IOPORT_G, led);
  }
  //putsUART("\r\n");
  led |= BIT_15;
  PORTWrite(IOPORT_G, led);
  putsUART("Only doing periodic tasks now\r\n");
  while(!UARTTransmissionHasCompleted(UART1));
  DelayMs(100);
  closeUART(UART1);
	LATAbits.LATA6 = 0;  // disable the phy
  disable_modules();
  while (1) {
   //ChipKITPeriodicTasks();
    INTDisableInterrupts();
    OSCConfig(OSC_FRC, 0, 0, 0);
    INTEnableInterrupts();
   device_sleep();
  }
}