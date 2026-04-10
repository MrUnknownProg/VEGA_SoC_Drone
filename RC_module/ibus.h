#ifndef IBUS_H
#define IBUS_H

#include "stdlib.h"

#define IBUS_FRAME_LEN      32
#define IBUS_START_BYTE1    0x20
#define IBUS_START_BYTE2    0x40
#define IBUS_NUM_CHANNELS   14

/* FS-i6 channel mapping */
#define IBUS_CH_AILERON     0
#define IBUS_CH_ELEVATOR    1
#define IBUS_CH_THROTTLE    2
#define IBUS_CH_RUDDER      3
#define IBUS_CH_SW1         4
#define IBUS_CH_SW2         5

/* UART1 registers — THEJAS32 address map */
#define UART1_BASE   0x10000200UL
#define UART1_DR     (*((volatile unsigned char*)(UART1_BASE + 0x00)))
#define UART1_IER    (*((volatile unsigned char*)(UART1_BASE + 0x04)))
#define UART1_LCR    (*((volatile unsigned char*)(UART1_BASE + 0x0C)))
#define UART1_LSR    (*((volatile unsigned char*)(UART1_BASE + 0x14)))
#define UART1_DLL    (*((volatile unsigned char*)(UART1_BASE + 0x00)))
#define UART1_DLM    (*((volatile unsigned char*)(UART1_BASE + 0x04)))
#define UART1_LSR_DR (1 << 0)

/*
 * ibus_init() — init UART1 at 115200 baud
 * ibus_read(channels) — call every loop
 *   fills channels[] with 14 values 1000-2000
 *   returns 1 when new valid frame received
 *   returns 0 otherwise
 */
void ibus_init(void);
int  ibus_read(unsigned short *channels);

#endif
