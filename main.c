#include "include/pwmc.h"
#include "include/ibus.h"
#include "include/debug_uart.h"

#define PWM_CLK_FREQ        40000000UL 
#define ESC_PERIOD_TICKS    800000UL     // 20ms at 40MHz
#define THROTTLE_CH         2            

// This offset (850us) is the "missing" time you observed.
// (1000us requested - 150us measured = 850us offset)
#define CORRECTION_US       48

void main(void) {
    unsigned short rc_values[IBUS_NUM_CHANNELS];
    ibus_init();
    
    printf("\r\n--- Trisul-SoC Final Timing Calibration ---\n");

    while (1) {
        if (ibus_read(rc_values)) {
            unsigned int thr = rc_values[THROTTLE_CH];

            // 1. Clamp RC input to standard range
            if (thr < 1000) thr = 1000;
            if (thr > 2000) thr = 2000;

            // 2. Add the 850us correction to align with your hardware's behavior
            // Resulting ticks: 1000us -> 74000 ticks | 2000us -> 114000 ticks
            unsigned int adjusted_us = thr + CORRECTION_US;
            unsigned int final_ticks = (unsigned int)(((unsigned long long)adjusted_us * PWM_CLK_FREQ) / 1000000ULL);

            // 3. Update PWM Channels (Forcing RepeatCount to 0 to fix the "4 pulse" issue)
            for(int ch = 0; ch < 4; ch++) {
                PWMC_Set_OnOffTime(ch, final_ticks);
                // Clear the whole control word to ensure Continuous Mode and no RepeatCount
                PWMCreg(ch).Control.word = (PWM_CONTINUOUS_MODE << 0); 
                PWMC_Set_Period(ch, ESC_PERIOD_TICKS);
            }
            
            PWMC_Enable();

            // 4. UART0 Monitoring
            printf("Stick: %u us | Adjusted: %u us | Ticks: %u \r", thr, adjusted_us, final_ticks);
        }
    }
}