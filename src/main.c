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
#define SYS_FREQ 80000000L  /* 80 Mhz */
#define FCY      SYS_FREQ

#define PB_DIV 2

uint8_t mac[] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x24 };
uint8_t ip[] = { 192,168,3,9 };
uint8_t gateway[] = { 192,168,3, 1 };
uint8_t subnet[] = { 255, 255, 255, 0 };
uint8_t dns1[] = { 192, 168, 3, 14 };
uint8_t dns2[] = { 8, 8, 8, 8 };

uint8_t server_ip[] = { 82,94,226,100 };
uint16_t server_port = 443;

//  Let's Encrypt Authority X3
unsigned char cert_data1[] = {
    48, 130,   5,  37,  48, 130,   4,  13, 160,   3,   2,   1,   2,   2,  18,
    3, 185, 111, 215,  58, 127, 225,  77, 163,  13, 202, 126,  48, 230, 144,
  172, 220, 214,  48,  13,   6,   9,  42, 134,  72, 134, 247,  13,   1,   1,
    11,   5,   0,  48,  74,  49,  11,  48,   9,   6,   3,  85,   4,   6,  19,
    2,  85,  83,  49,  22,  48,  20,   6,   3,  85,   4,  10,  19,  13,  76,
  101, 116,  39, 115,  32,  69, 110,  99, 114, 121, 112, 116,  49,  35,  48,
    33,   6,   3,  85,   4,   3,  19,  26,  76, 101, 116,  39, 115,  32,  69,
  110,  99, 114, 121, 112, 116,  32,  65, 117, 116, 104, 111, 114, 105, 116,
  121,  32,  88,  51,  48,  30,  23,  13,  49,  54,  49,  50,  49,  55,  50,
    50,  48,  49,  48,  48,  90,  23,  13,  49,  55,  48,  51,  49,  55,  50,
    50,  48,  49,  48,  48,  90,  48,  32,  49,  30,  48,  28,   6,   3,  85,
    4,   3,  19,  21, 100, 101, 118, 101, 108, 111, 112, 101, 114,  46,  98,
  108, 101, 110, 100, 101, 114,  46, 111, 114, 103,  48, 130,   1,  34,  48,
    13,   6,   9,  42, 134,  72, 134, 247,  13,   1,   1,   1,   5,   0,   3,
  130,   1,  15,   0,  48, 130,   1,  10,   2, 130,   1,   1,   0, 233, 158,
  244, 170,  74,  22, 254, 188, 168,  97,  19, 122,  46, 185,  35, 216, 155,
  243, 226, 194, 213,   1, 224,  79,  67, 176,   6, 149, 199,  67, 101, 105,
    29,  10, 131,  22,  85, 176, 228, 174, 233, 186,  90, 130,  40,   1, 159,
  192,  83,  23, 249,  99,  56,  39,  30, 215, 251,  73,  86, 162, 239,  85,
  238, 214, 230,  93, 225,  99, 130, 222, 200,  66, 241, 200, 150, 206, 228,
    54, 139, 183, 168,  36, 221,  40,  24, 180,  91,  65,   2,  28,  33, 103,
  204, 172,  87, 133,  54, 254, 234, 236,  95,  54,  84, 177,  22,  80,  67,
    65, 122, 239, 190, 140,  98, 191, 225, 235,  26, 202,  47,  48,  54,  38,
  145, 196, 169, 255, 254, 102, 235, 197, 174, 241, 214, 161,  66, 158, 109,
    96, 184, 103, 251,  29, 112,  12, 128, 113,  11, 212,  67,  91, 145, 150,
  160, 144,   4, 255,  22, 124, 246, 162, 178,  74, 211, 251,  52,   8,  80,
  133, 243, 119, 192, 101,  10,  23, 161, 160, 215, 209, 126, 237, 108, 233,
  120,  86, 120, 116, 167,  45, 180, 191, 201,  48,  54,  83, 116, 142, 182,
    94, 104, 156,  82, 255, 227,  10, 143, 113, 176,  81,  44,  22,  86, 215,
  218, 211,  13, 186,  29, 236, 213, 108, 138,  37, 189,  85, 203, 111,  73,
    5, 143,  72, 230,  60, 106, 142, 220, 164,  59, 254,  62,  24, 188,  86,
  187,  11,  29,  71, 247,  33,  46,  90, 233,  60,  83, 195, 158, 151,   2,
    3,   1,   0,   1, 163, 130,   2,  45,  48, 130,   2,  41,  48,  14,   6,
    3,  85,  29,  15,   1,   1, 255,   4,   4,   3,   2,   5, 160,  48,  29,
    6,   3,  85,  29,  37,   4,  22,  48,  20,   6,   8,  43,   6,   1,   5,
    5,   7,   3,   1,   6,   8,  43,   6,   1,   5,   5,   7,   3,   2,  48,
    12,   6,   3,  85,  29,  19,   1,   1, 255,   4,   2,  48,   0,  48,  29,
    6,   3,  85,  29,  14,   4,  22,   4,  20, 187, 227, 143,  59,  99,  15,
    4,  67, 199, 125, 139,   2, 224, 107, 246,  76, 111, 220, 181,  12,  48,
    31,   6,   3,  85,  29,  35,   4,  24,  48,  22, 128,  20, 168,  74, 106,
    99,   4, 125, 221, 186, 230, 209,  57, 183, 166,  69, 101, 239, 243, 168,
  236, 161,  48, 112,   6,   8,  43,   6,   1,   5,   5,   7,   1,   1,   4,
  100,  48,  98,  48,  47,   6,   8,  43,   6,   1,   5,   5,   7,  48,   1,
  134,  35, 104, 116, 116, 112,  58,  47,  47, 111,  99, 115, 112,  46, 105,
  110, 116,  45, 120,  51,  46, 108, 101, 116, 115, 101, 110,  99, 114, 121,
  112, 116,  46, 111, 114, 103,  47,  48,  47,   6,   8,  43,   6,   1,   5,
    5,   7,  48,   2, 134,  35, 104, 116, 116, 112,  58,  47,  47,  99, 101,
  114, 116,  46, 105, 110, 116,  45, 120,  51,  46, 108, 101, 116, 115, 101,
  110,  99, 114, 121, 112, 116,  46, 111, 114, 103,  47,  48,  55,   6,   3,
    85,  29,  17,   4,  48,  48,  46, 130,  21, 100, 101, 118,  45, 102, 105,
  108, 101, 115,  46,  98, 108, 101, 110, 100, 101, 114,  46, 111, 114, 103,
  130,  21, 100, 101, 118, 101, 108, 111, 112, 101, 114,  46,  98, 108, 101,
  110, 100, 101, 114,  46, 111, 114, 103,  48, 129, 254,   6,   3,  85,  29,
    32,   4, 129, 246,  48, 129, 243,  48,   8,   6,   6, 103, 129,  12,   1,
    2,   1,  48, 129, 230,   6,  11,  43,   6,   1,   4,   1, 130, 223,  19,
    1,   1,   1,  48, 129, 214,  48,  38,   6,   8,  43,   6,   1,   5,   5,
    7,   2,   1,  22,  26, 104, 116, 116, 112,  58,  47,  47,  99, 112, 115,
    46, 108, 101, 116, 115, 101, 110,  99, 114, 121, 112, 116,  46, 111, 114,
  103,  48, 129, 171,   6,   8,  43,   6,   1,   5,   5,   7,   2,   2,  48,
  129, 158,  12, 129, 155,  84, 104, 105, 115,  32,  67, 101, 114, 116, 105,
  102, 105,  99,  97, 116, 101,  32, 109,  97, 121,  32, 111, 110, 108, 121,
    32,  98, 101,  32, 114, 101, 108, 105, 101, 100,  32, 117, 112, 111, 110,
    32,  98, 121,  32,  82, 101, 108, 121, 105, 110, 103,  32,  80,  97, 114,
  116, 105, 101, 115,  32,  97, 110, 100,  32, 111, 110, 108, 121,  32, 105,
  110,  32,  97,  99,  99, 111, 114, 100,  97, 110,  99, 101,  32, 119, 105,
  116, 104,  32, 116, 104, 101,  32,  67, 101, 114, 116, 105, 102, 105,  99,
    97, 116, 101,  32,  80, 111, 108, 105,  99, 121,  32, 102, 111, 117, 110,
  100,  32,  97, 116,  32, 104, 116, 116, 112, 115,  58,  47,  47, 108, 101,
  116, 115, 101, 110,  99, 114, 121, 112, 116,  46, 111, 114, 103,  47, 114,
  101, 112, 111, 115, 105, 116, 111, 114, 121,  47,  48,  13,   6,   9,  42,
  134,  72, 134, 247,  13,   1,   1,  11,   5,   0,   3, 130,   1,   1,   0,
    90,  22, 228, 203,  66,  35,  79, 189, 107, 130, 228, 106, 254, 242, 247,
  248,  38, 238, 139,  84, 186,  39, 141, 220, 252, 248,   0, 215,  21, 231,
    46, 213,  28, 122,   1,  97, 247,  82, 213, 120, 141,  41, 182,   3, 251,
  212,  77, 141,   7, 245, 109, 242, 253,  44, 118, 186, 107,  63, 249, 102,
  164, 187, 148, 146, 230, 234, 212, 170,  99,  32,  58, 207,  96, 105,  92,
  124, 137, 176, 240, 219,  33,  17, 246, 112, 192, 152, 202, 249,  37, 232,
    16, 199,  94, 178, 216,  71,  28,  66, 188, 221,  36,  48,  41,  82, 169,
  206,  75, 116,  92, 139, 232, 171, 187, 102, 175,  10, 202,  13, 184, 192,
    82, 103,  38,  69, 163, 163, 194, 102, 109,  43,  97,  47,  21, 131,   3,
  235, 156, 208, 254,  15, 116,  64,  18,  42, 225, 174, 248, 172, 123,  93,
    18,  94, 140, 138,  17, 229,  71,  96,  12, 204,  49, 193,  98, 179,  70,
  206, 255,  59, 112, 124, 169, 209,   3, 214, 199, 203,  48,  10,  39, 172,
  222, 255, 197, 124,  81, 184,  88, 121,  55,  62, 151, 127, 166,   8, 188,
  220,  52,  38, 133, 252, 108,  12, 152, 158, 115,  78, 129, 122,  88, 170,
  126, 179, 162, 142,  94, 225, 204,  41, 198,  95,  52, 161, 125,  91, 195,
  100, 244, 135, 208, 156, 218, 179,  46, 126, 233,   7,  19, 162,  30, 184,
  227, 214,   8,   1, 223, 226,  18,  38,  15,  94,  98, 161, 112,  88,  44, 190};

// DST Root CA X3
unsigned char cert_data2[] = {
   48, 130,   3,  74,  48, 130,   2,  50, 160,   3,   2,   1,   2,   2,  16,
   68, 175, 176, 128, 214, 163,  39, 186, 137,  48,  57, 134,  46, 248,  64,
  107,  48,  13,   6,   9,  42, 134,  72, 134, 247,  13,   1,   1,   5,   5,
    0,  48,  63,  49,  36,  48,  34,   6,   3,  85,   4,  10,  19,  27,  68,
  105, 103, 105, 116,  97, 108,  32,  83, 105, 103, 110,  97, 116, 117, 114,
  101,  32,  84, 114, 117, 115, 116,  32,  67, 111,  46,  49,  23,  48,  21,
    6,   3,  85,   4,   3,  19,  14,  68,  83,  84,  32,  82, 111, 111, 116,
   32,  67,  65,  32,  88,  51,  48,  30,  23,  13,  48,  48,  48,  57,  51,
   48,  50,  49,  49,  50,  49,  57,  90,  23,  13,  50,  49,  48,  57,  51,
   48,  49,  52,  48,  49,  49,  53,  90,  48,  63,  49,  36,  48,  34,   6,
    3,  85,   4,  10,  19,  27,  68, 105, 103, 105, 116,  97, 108,  32,  83,
  105, 103, 110,  97, 116, 117, 114, 101,  32,  84, 114, 117, 115, 116,  32,
   67, 111,  46,  49,  23,  48,  21,   6,   3,  85,   4,   3,  19,  14,  68,
   83,  84,  32,  82, 111, 111, 116,  32,  67,  65,  32,  88,  51,  48, 130,
    1,  34,  48,  13,   6,   9,  42, 134,  72, 134, 247,  13,   1,   1,   1,
    5,   0,   3, 130,   1,  15,   0,  48, 130,   1,  10,   2, 130,   1,   1,
    0, 223, 175, 233, 151,  80,   8, 131,  87, 180, 204,  98, 101, 246, 144,
  130, 236, 199, 211,  44, 107,  48, 202,  91, 236, 217, 195, 125, 199,  64,
  193,  24,  20, 139, 224, 232,  51, 118,  73,  42, 227,  63,  33,  73, 147,
  172,  78,  14, 175,  62,  72, 203, 101, 238, 252, 211,  33,  15, 101, 210,
   42, 217,  50, 143, 140, 229, 247, 119, 176,  18, 123, 181, 149, 192, 137,
  163, 169, 186, 237, 115,  46, 122,  12,   6,  50, 131, 162, 126, 138,  20,
   48, 205,  17, 160, 225,  42,  56, 185, 121,  10,  49, 253,  80, 189, 128,
  101, 223, 183,  81,  99, 131, 200, 226, 136,  97, 234,  75,  97, 129, 236,
   82, 107, 185, 162, 226,  75,  26,  40, 159,  72, 163, 158,  12, 218,   9,
  142,  62,  23,  46,  30, 221,  32, 223,  91, 198,  42, 138, 171,  46, 189,
  112, 173, 197,  11,  26,  37, 144, 116, 114, 197, 123, 106, 171,  52, 214,
   48, 137, 255, 229, 104,  19, 123,  84,  11, 200, 214, 174, 236,  90, 156,
  146,  30,  61, 100, 179, 140, 198, 223, 191, 201,  65, 112, 236,  22, 114,
  213,  38, 236,  56,  85,  57,  67, 208, 252, 253,  24,  92,  64, 241, 151,
  235, 213, 154, 155, 141,  29, 186, 218,  37, 185, 198, 216, 223, 193,  21,
    2,  58, 171, 218, 110, 241,  62,  46, 245,  92,   8, 156,  60, 214, 131,
  105, 228,  16, 155,  25,  42, 182,  41,  87, 227, 229,  61, 155, 159, 240,
    2,  93,   2,   3,   1,   0,   1, 163,  66,  48,  64,  48,  15,   6,   3,
   85,  29,  19,   1,   1, 255,   4,   5,  48,   3,   1,   1, 255,  48,  14,
    6,   3,  85,  29,  15,   1,   1, 255,   4,   4,   3,   2,   1,   6,  48,
   29,   6,   3,  85,  29,  14,   4,  22,   4,  20, 196, 167, 177, 164, 123,
   44, 113, 250, 219, 225,  75, 144, 117, 255, 196,  21,  96, 133, 137,  16,
   48,  13,   6,   9,  42, 134,  72, 134, 247,  13,   1,   1,   5,   5,   0,
    3, 130,   1,   1,   0, 163,  26,  44, 155,  23,   0,  92, 169,  30, 238,
   40, 102,  55,  58, 191, 131, 199,  63,  75, 195,   9, 160, 149,  32,  93,
  227, 217,  89,  68, 210,  62,  13,  62, 189, 138,  75, 160, 116,  31, 206,
   16, 130, 156, 116,  26,  29, 126, 152,  26, 221, 203,  19,  75, 179,  32,
   68, 228, 145, 233, 204, 252, 125, 165, 219, 106, 229, 254, 230, 253, 224,
   78, 221, 183,   0,  58, 181, 112,  73, 175, 242, 229, 235,   2, 241, 209,
    2, 139,  25, 203, 148,  58,  94,  72, 196,  24,  30,  88,  25,  95,  30,
    2,  90, 240,  12, 241, 177, 173, 169, 220,  89, 134, 139, 110, 233, 145,
  245, 134, 202, 250, 185, 102,  51, 170,  89,  91, 206, 226, 167,  22, 115,
   71, 203,  43, 204, 153, 176,  55,  72, 207, 227,  86,  75, 245, 207,  15,
   12, 114,  50, 135, 198, 240,  68, 187,  83, 114, 109,  67, 245,  38,  72,
  154,  82, 103, 183,  88, 171, 254, 103, 118, 113, 120, 219,  13, 162,  86,
   20,  19,  57,  36,  49, 133, 162, 168,   2,  90,  48,  71, 225, 221,  80,
    7, 188,   2,   9, 144,   0, 235, 100,  99,  96, 155,  22, 188, 136, 201,
   18, 230, 210, 125, 145, 139, 249,  61,  50, 141, 101, 180, 233, 124, 177,
   87, 118, 234, 197, 182,  40,  57, 191,  21, 101,  28, 200, 246, 119, 150,
  106,  10, 141, 119,  11, 216, 145,  11,   4, 142,   7, 219,  41, 182,  10,
  238, 157, 130,  53,  53,  16};

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
  // wolfSSL_Debugging_ON();
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