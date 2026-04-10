#include "ibus.h"
#include <stdio.h>
// If your SDK doesn't provide a global tick, we use a loop counter for timing
static unsigned int fake_tick = 0;
void main(void) {
    // Array to store the 10 channels from the FS-iA10B
    unsigned short rc_values[IBUS_NUM_CHANNELS];
    unsigned int last_print_tick = 0;
    // Initialize UART1 for 115200 baud
    ibus_init();
    printf("\r\n=== Trisul-SoC iBUS RC Test ===\r\n");
    while (1) {
        // 1. Pass the array to the function (Fixes the "too few arguments" error)
        // This returns 1 only when a complete, valid frame is received
        int valid_frame = ibus_read(rc_values);
        // Increment a counter to simulate timing for printing
        fake_tick++;
        // 2. Non-blocking Print (every few thousand loops)
        if (fake_tick - last_print_tick > 5000) {
            if (valid_frame || rc_values[0] != 0) {
                // Channels: 0=Ail, 1=Ele, 2=Thr, 3=Rud (standard FlySky mapping)
                printf("THR:%4u | AIL:%4u | ELE:%4u | RUD:%4u\r", 
                        rc_values[2], rc_values[0], rc_values[1], rc_values[3]);
            } else {
                printf("Searching for iBUS signal...\r");
            }
            last_print_tick = fake_tick;
        }
        
        // Safety: If fake_tick gets too high, reset it to prevent overflow
        if (fake_tick > 1000000) {
            fake_tick = 0;
            last_print_tick = 0;
        }
    }
}

