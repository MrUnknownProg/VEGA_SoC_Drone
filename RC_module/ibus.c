/*
 * ibus.c — FlySky iBUS decoder for THEJAS32
 *
 * Packet format (32 bytes, 115200 baud, every 7ms):
 *   Byte  0:    0x20  (header)
 *   Byte  1:    0x40  (header)
 *   Byte  2-3:  CH0   little endian
 *   Byte  4-5:  CH1   little endian
 *   ...
 *   Byte 28-29: CH13  little endian
 *   Byte 30-31: checksum little endian
 *
 * Checksum = 0xFFFF - sum of all bytes except checksum bytes
 */

#include "ibus.h"
#include "stdlib.h"

/* State machine */
typedef struct {
    unsigned int   state;
    unsigned short checksum;
    unsigned char  datal;
} IBusState_t;

static IBusState_t    g_sm;
static unsigned short g_data[IBUS_NUM_CHANNELS];
static unsigned short g_channels[IBUS_NUM_CHANNELS];
static int            g_new_frame = 0;

/* ============================================================
 *  UART1 INIT
 *  115200 baud at 38707200 Hz
 *  Divisor = 38707200 / (16 x 115200) = 21
 * ============================================================ */
void ibus_init(void)
{
    int i;

    UART1_LCR = 0x83;   /* enable divisor latch */
    UART1_DLL = 21;     /* baud divisor = 21    */
    UART1_DLM = 0;
    UART1_LCR = 0x03;   /* 8N1                  */
    UART1_IER = 0x00;   /* polling mode         */

    g_sm.state    = 0;
    g_sm.checksum = 0xFFFF;
    g_sm.datal    = 0;
    g_new_frame   = 0;

    for (i = 0; i < IBUS_NUM_CHANNELS; i++)
    {
        g_data[i]     = 1000;
        g_channels[i] = 1000;
    }
}

/* ============================================================
 *  PROCESS ONE BYTE — state machine
 *  Returns 1 when complete valid frame received
 * ============================================================ */
static int process_byte(unsigned char ch)
{
    int i;

    switch (g_sm.state)
    {
        /* Wait for first header byte 0x20 */
        case 0:
            if (ch == IBUS_START_BYTE1)
            {
                g_sm.checksum = 0xFFFF - 0x20;
                g_sm.state    = 1;
            }
            break;

        /* Wait for second header byte 0x40 */
        case 1:
            if (ch == IBUS_START_BYTE2)
            {
                g_sm.checksum -= ch;
                g_sm.state     = 2;
            }
            else
            {
                g_sm.state = 0;   /* bad header — reset */
            }
            break;

        /* Checksum low byte */
        case 30:
            g_sm.datal = ch;
            g_sm.state = 31;
            break;

        /* Checksum high byte — validate frame */
        case 31:
        {
            unsigned short rx_checksum = ((unsigned short)ch << 8)
                                       | g_sm.datal;
            g_sm.state = 0;

            if (rx_checksum == g_sm.checksum)
            {
                /* Valid — copy parsed data to output channels */
                for (i = 0; i < IBUS_NUM_CHANNELS; i++)
                    g_channels[i] = g_data[i];

                return 1;   /* new valid frame */
            }
            /* Bad checksum — discard */
            break;
        }

        /* States 2-29 — channel data bytes */
        default:
        {
            unsigned int ch_idx = (g_sm.state / 2) - 1;

            if ((g_sm.state & 1) == 0)
            {
                /* Even = low byte */
                g_sm.datal = ch;
            }
            else
            {
                /* Odd = high byte — complete one channel */
                if (ch_idx < IBUS_NUM_CHANNELS)
                {
                    unsigned short val =
                        ((unsigned short)ch << 8) | g_sm.datal;

                    if (val < 1000) val = 1000;
                    if (val > 2000) val = 2000;

                    g_data[ch_idx] = val;
                }
            }

            g_sm.checksum -= ch;
            g_sm.state++;
            break;
        }
    }

    return 0;
}

/* ============================================================
 *  IBUS READ
 *
 *  Call every loop iteration — completely non blocking
 *  Reads all available bytes from UART1
 *  Returns 1 when new complete valid frame received
 *  Returns 0 otherwise
 *
 *  channels[] is filled with 14 values (1000-2000)
 *  matching your main.c: rc_values[0..13]
 * ============================================================ */
int ibus_read(unsigned short *channels)
{
    unsigned char byte;
    int           got_frame = 0;
    int           i;

    while (UART1_LSR & UART1_LSR_DR)
    {
        byte = UART1_DR;

        if (process_byte(byte))
        {
            /* Copy channels to caller's array */
            for (i = 0; i < IBUS_NUM_CHANNELS; i++)
                channels[i] = g_channels[i];

            got_frame = 1;
        }
    }

    return got_frame;
}
