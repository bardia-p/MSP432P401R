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
static volatile uint8_t currentLED;
static volatile uint8_t paused;

int main() {
    // RED LED is selected by default
	currentLED = LED_RED;

    // The system originally is not paused
	paused = 0;
	
    // Disable the watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// Configure GPIO switchs and LEDs
	config_switches();
	config_leds();

    // Configure interrupts
    config_interrupts();
	
	TA0CTL &= (uint16_t)(~((1<<5)|(1<<4)));     // Stop the timer
	TA0CTL &= (uint16_t)(~(1<<0));              // Clear interrupt flag
	TA0CCR0 = (uint16_t)(32768);                // Holds the upper limit value
	TA0CTL |= (uint16_t)((1<<1));               // Interrupt enable
	TA0CTL |= (uint16_t)((1<<4));               // Up count mode
	TA0CTL |= (uint16_t)((1<<8));               // ACLK as source for timer -> 32.768 kHz
								
	// NVIC configuration
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	NVIC_SetPriority(TA0_N_IRQn,3);
	NVIC_ClearPendingIRQ(TA0_N_IRQn);
	NVIC_EnableIRQ(TA0_N_IRQn);
	
	//Globally enable interrupts in CPU
	__ASM("CPSIE I");
	
	// Waiting for interrupts
	while(1) {
		__ASM("WFI");
	}
	
	return 0;
}

/* 
Configure the switches (P1.1 and P1.4) as inputs, using pull-up internal resistors. 
*/
void config_switches(){	
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
void config_leds(){	
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
void PORT1_IRQHandler(void){
	static volatile uint16_t i = DELAY_VALUE;
	
	if (((P1->IFG & (uint8_t)(1<<1)) != 0) || ((P1->IFG & (uint8_t) (1<<4)) != 0)) {
    // Debouncing the LEDs
    while (i--) {
			__ASM volatile (""); // Delay loop contains some asm code placeholder
    }
		
		// Test for pin 3 interrupt flag
		if ((P1->IFG & (uint8_t)(1<<1)) != 0) {
            // Clearing the interrupt flag
			P1->IFG &= (uint8_t)~(1<<1);

            // Button 1 toggles between LEDs
            // When the button is pressed, the currently selected LED retains its state
			currentLED = (currentLED + 1) % 2;
		} else if((P1->IFG & (uint8_t) (1<<4)) != 0) {
			P1->IFG &= (uint8_t)~(1<<4);
			
            // When paused, if LEDs were off, they should remain off
			if (!paused) {
				paused = 1;
				TA0CTL &= (uint16_t)(~(1<<1)); // Disable interrupt
				TA0CTL &= (uint16_t)(~((1<<5)|(1<<4))); // Stop timer
			} else {
				paused = 0;
				TA0CTL |= (uint16_t)((1<<1)); // Interrupt enable
				TA0CTL |= (uint16_t)((1<<4)); // Up count mode
			}
		}
	}
}

/*
Timer A interrupt request handler.
*/
void TA0_N_IRQHandler(void){
	// Clear the interrupt flag
	TA0CTL &= (uint16_t) (~(1<<0));
	
	// Change the LEDs
	if (currentLED == LED_RED){ 
        // RED_LED has 2 states, bitwise toggling
		P1->OUT ^= (uint8_t)((1<<0));
	} else {
        // RGB_LED has 8 states, bitwise clearing and setting
		uint8_t count = (P2->OUT & RGB_LED_MASK);
		count++;
		P2->OUT &= ~(RGB_LED_MASK);
		P2->OUT |= count & RGB_LED_MASK;
	}
}
