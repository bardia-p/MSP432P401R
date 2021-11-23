/*
November 29, 2021
SYSC 3310
Lab 6 - Timer
Group 3
Bardia Parmoun 101143006
Kyra Lothrop 101145872
Trong Nguyen 100848232
*/

// Include peripheral device's register definitions
#include "msp.h"

// Define constants
#define DELAY_VALUE 5000
#define RGB_LED_MASK 0x07

#define LED_RED 0
#define LED_RGB 1

// Function prototypes
void config_switches(void);
void config_leds(void);
void config_interrupts(void);

// Variable definitions
static volatile uint8_t current_led;

int main(void) {
    // Disable the watchdog timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Configure GPIO switchs and LEDs.
	config_switches();
	config_leds();
	
	// Check initial LED selection is set to RED
	current_led = LED_RED;

    // Globally enable interrupts in CPU
    __ASM("CPSIE I");

    // Configure interrupts
    config_interrupts();

	// NVIC configuration
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);

    // Setup Timer
    TA0->CCTL0 = TIMER_A_CCTLN_CCIE;        // Enable CC interrupt
    TA0->CTL = TIMER_A_CTL_TASSEL_1 |       // ACLK as source for timer -> 32.768 kHz
                TIMER_A_CTL_ID_0 |          // Divide clock by 1 -> 32.768 kHz / 1 = 32.768 kHz
                TIMER_A_CTL_MC_1 |          // Up mode
                TIMER_A_CTL_IE;             // Enable overflow interrupt

    // Holds the upper limit value
    TA0->CCR0 = 32768;                      // Timer 16 bits range gives 2 seconds, we want every 1 second

	// Create infinite loop.
	while(1) {
		__ASM("WFI");
	}
	
	return 0;
}

/* 
Configure the switches (P1.1 and P1.4) as inputs, using pull-up internal resistors. 
*/
void config_switches(void) {
	// Set function to GPIO (P1.1 and P1.4)
	P1->SEL0 &= (uint8_t)(~((1<<1)|(1<<4)));
	P1->SEL1 &= (uint8_t)(~((1<<1)|(1<<4)));

	// Set direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	// Set pull resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	// Set resistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

/*
Configure LEDs (P1.0 and P2.0, P2.1, P2.2) as outputs.
*/
void config_leds(void) {
    /* LEDs P1.0 */
	// Set function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));

	// Set direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	// Set drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	// Set default state of all pins to off
	P1->OUT &= (uint8_t)(~(1<<0));

    // Disable interrupts
	P1->IE &= (uint8_t)(~(1<<0)); 

    /* LEDs P2.0, P2.1, P2.2 */
	// Set function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Set direction of LED pin to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	// Set drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Initialize LEDs states (all turned off)
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}

/*
Configure switch interrupts at port level (device), NVIC, and CPU.
*/
void config_interrupts(void) {
    // Configure interrupts on falling edge
	P1->IES |= (uint8_t)((1<<1)|(1<<4));
	
	// Clear interrupt flags
	P1->IFG &= (uint8_t)~((1<<1)|(1<<4));
	
	// Interrupt pin enabled
	P1->IE |= (uint8_t)((1<<1)|(1<<4));
}

/*
Port 1 interrupt service request handler.
*/
void PORT1_IRQHandler(void) {
	//Test for pin 3 interrupt flag (Button 1 pressed)
	if ((P1->IFG & (uint8_t)(1<<1)) != 0) {
        // Debouncing the LEDs
	    static uint16_t i = DELAY_VALUE;
        while (i--) {
            __ASM volatile ("") // Delay loop contains some asm code placeholder
        }

		// Clearing the interrupt flag
		P1->IFG &= ~(uint8_t)(1<<1);
		
		// Button 1 toggles between LEDs
        // When the button is pressed, the currently selected LED retains its state
		current_LED = (current_LED + 1) % 2;

	} else if ((P1->IFG & (uint8_t)(1<<4)) != 0) {
        // Debouncing the LEDs
	    static uint16_t i = DELAY_VALUE;
        while (i--) {
            __ASM volatile ("") // Delay loop contains some asm code placeholder
        }

		// Clearing the interrupt flag
		P1->IFG &= ~(uint8_t)(1<<4);
		
        // Button 2 pauses/resumes the system
        // When paused, if LEDs were off, they should remain off
        // Timer A Control Register Mode Control: Up Mode (p.797)
        TA0->CTL ^= TIMER_A_CTL_MC_1;
	}
}

/*
Timer A interrupt request handler.
*/
void TA0_N_IRQHandler(void) {
    // TIMER_A_CTL_IFG = (uint16_t)(1<<0);

    if (TA0->CTL & TIMER_A_CTL_IFG) {
		if (currentLED == LED_RED) {
            // RED_LED has 2 states, bitwise toggling
			P1->OUT ^= (uint8_t)((1<<0));
		} else {
            // RGB_LED has 8 states, bitwise clearing and setting
			uint8_t count = (P2->OUT & RGB_LED_MASK);
			count++;
			P2->OUT &= ~RGB_LED_MASK;
			P2->OUT |= count & RGB_LED_MASK;
		}
        // Clear Timer A Control Register interrupt flag (p.797)
        TA0->CTL &= ~TIMER_A_CTL_IFG;
    }
}