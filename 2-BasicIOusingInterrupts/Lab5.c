/*
November 22, 2021
SYSC 3310
Lab 5 - Basic I/O using Interrupts
Group 3
Bardia Parmoun 101143006
Kyra Lothrop 101145872
Trong Nguyen 100848232
*/

#include "msp.h"

// Define constants
#define DELAY_VALUE 5000
#define RGB_LED_MASK 0x07

#define LED_RED 0
#define LED_RGB 1

// Function prototypes
static void main_loop(void);
void config_switches(void);
void config_leds(void);

// ISR Prototype
void PORT1_IRQHandler(void);

// Variable definitions
static uint8_t selected_led;
	
int main(void) {
	// Stop watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Configure GPIO switches and LEDs
	config_switches();
	config_leds();

	// Check initial LED selection is set to RED
	selected_led = LED_RED;
	
    // Configure interrupts on falling edge
    P1->IES |= (uint8_t)(((1<<1)|(<1<<4));

    // Clear interrupt flags
    P1->IFG &= (uint8_t)(~((1<<1)|(1<<4)));

    // Interrupt pin enabled
    P1->IE |= (uint8_t)(((1<<1)|(<1<<4));

    // Configure NVIC
    NVIC_SetPriority(PORT1_IRQn, 2);
    NVIC_ClearPendingIRQ(PORT1_IRQn);
    NVIC_EnableIRQ(PORT1_IRQn);

    // Enable global interrups
    __asm("CPSIE I");

	// Creat infinite loop
	while(1) {
        __asm("WFI");
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

	// Disable interrupts
	P1->IE &= (uint8_t)(~((1<<1)|(1<<4)));
	
	// Set direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	// Set resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	// Set resistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

/*
Configure switch interrupts.
*/
void config_leds(void) {
	// Set function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));
	
    /* P1.0 */
	// Disables interrupts
	P1->IE &= (uint8_t)(~(1<<0));

	// Set direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	// Set drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	// Initialize LEDs states (all turned off).
	P1->OUT &= (uint8_t)(~(1<<0));
	
    /* P2.0, P2.1, P2.2 */
	// Disable interrupts
	P2->IE &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Set function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Set direction of LED pins to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	// Set drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Set default state of all pins to off
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}

/*
Interrupt Service Routine with debouncing
*/
void PORT1_IRQHandler(void) {
		
    // Select LED Button Action
    if ((P1->IFG & (uint8_t)(1<<1)) != 0) {
        
        uint16_t i = DELAY_VALUE;
        while (i--) {
            __asm volatile ("") // Delay loop contains some asm code placeholder
        }

        P1->IFG &= (uint8_t)(~(1<<1));
    } else {
        if (!(P1->IN & (uint8_t)(1<<1))) { // Button 1 is pressed
            selected_led = (selected_led == LED_RED) ? LED_RGB : LED_RED;
        }
    }

    // Select Mode Button Action
    if ((P1->IFG & (uint8_t)(1<<4)) != 0) { // Button 2 is pressed
        
        uint16_t i = DELAY_VALUE;
        while (i--) {
            __asm volatile ("") // Delay loop contains some asm code placeholder
        }

        P1->IFG &= (uint8_t)(~(1<<4));
    } else {
        if ((P1->IN & (uint8_t)(1<<4)) {
            if (selected_led == LED_RED) {
                 // RED_LED has 2 states, bitwise toggling
                P1->OUT ^= (uint8_t)(1<<0);
            } else if (selected_led == LED_RGB) {
                // RGB_LED has 8 states, bitwise clearing and setting
                (uint8_t) count = (P2->OUT & RGB_LED_MASK) >> 0;
                count++;
                P2->OUT &= ~(RGB_LED_MASK);
                P2->OUT |= (count & RGB_LED_MASK) << 0;
            }
        }
    }     
}