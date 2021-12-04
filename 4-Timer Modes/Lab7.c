/*
December 6, 2021
SYSC 3310
Lab 7 - Timer Modes
Group 3
Bardia Parmoun 101143006
Kyra Lothrop 101145872
Trong Nguyen 100848232
*/

// Include peripheral device register definitions
#include "msp.h"

// Define constants
#define RGB_LED_MASK 0x07

// Function prototypes
void config_leds(void);
void config_TA0(void);
void config_TA1(void);
void handleRGBState(void);

int main(void) {	
	// Stop watchdog timer.
	WDT_A -> CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	//intializes LEDs.
	config_leds();
	
	// Configure Timer A0
	config_TA0();
	
	// Configure NVIC for Timer A0
	NVIC_SetPriority(TA0_N_IRQn, 2);
	NVIC_ClearPendingIRQ(TA0_N_IRQn);
	NVIC_EnableIRQ(TA0_N_IRQn);
	
	// Configure Timer A1
	config_TA1();
	
	// Configure NVIC for timer A1 when timer reaches 0
	NVIC_SetPriority(TA1_N_IRQn, 2);
	NVIC_ClearPendingIRQ(TA1_N_IRQn);
	NVIC_EnableIRQ(TA1_N_IRQn);

	// Configure NVIC for timer A1 when timer reaches CCR0
	NVIC_SetPriority(TA1_0_IRQn, 2);
	NVIC_ClearPendingIRQ(TA1_0_IRQn);
	NVIC_EnableIRQ(TA1_0_IRQn);
	
	//Globally enable interrupts in CPU.
	__ASM("CPSIE I");
	
	// Turning the RED LED on initially. 
	P1->OUT |= (uint8_t)((1<<0));
	
	// Waiting for interrupts
	while(1) {
		__ASM("WFI");
	}
	
	return 0;
}

/*
Configure LEDs (P1.0 and P2.0, P2.1, P2.2) as outputs.
*/
void config_leds() {	
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
Configure Timer A0.
*/
void config_TA0(void) {
  TA0CTL &= (uint16_t)(~((1<<5)|(1<<4)));     // Stop the timer
	TA0CTL &= (uint16_t)(~(1<<0));              // Clear interrupt flag TAIFG
	TA0CCR0 = (uint16_t)(32768);             	// Holds the upper limit value 1s
	TA0CTL |= (uint16_t)((1<<1));               // Interrupt enable TAIE
	TA0CTL |= (uint16_t)((1<<4));               // Up count Mode Control
	TA0CTL |= (uint16_t)((1<<8));               // ACLK as source for timer -> 32.768 kHz
	TA0CTL &= (uint16_t)(~(1<<9));				// Clear clock source TASSEL
}

/*
Configure Timer A1.
*/
void config_TA1(void) {
  TA1CTL &= (uint16_t)(~((1<<5)|(1<<4)));     // Stop the timer
	TA1CTL &= (uint16_t)(~(1<<0));              // Clear interrupt TAIFG
	TA1CCR0 = (uint16_t)(3277);                // Holds the upper limit value 0.1s -> 32768/10 = 3277
	TA1CTL |= (uint16_t)((1<<1));               // Interrupt enable count reaches CCR0
	TA1CTL |= (uint16_t)((1<<4)|(1<<5));        // Up/down count Mode Control
	TA1CTL |= (uint16_t)((1<<8));               // ACLK as source for timer -> 32.768 kHz
	TA0CTL &= (uint16_t)(~(1<<9));				// Clear clock source TASSEL
}

/*
Timer A interrupt request handler.
*/
void TA0_N_IRQHandler(void){
	// Clear the interrupt flag
	TA0CTL &= (uint16_t)(~(1<<0));
	
	// RED_LED has 2 states, bitwise toggling
	P1->OUT ^= (uint8_t)((1<<0));
}

/*
Handle when Timer A1 reaches 0.
*/
void TA1_N_IRQHandler(void){
	// Clear the interrupt flag
	TA1CTL &= (uint16_t)(~(1<<0));
	handleRGBState();
	
	// Making the timer a 0.5s timer.
	TA1CCR0 = (uint16_t)(16834); // Holds the upper limit value 0.5s -> 32768/2 = 16384
}

/*
Handle when Timer A1 reaches CCR0 (upper time limit).
*/
void TA1_0_IRQHandler(void) {
	// Clear the interrupt flag
	TA1CCTL0 &= (uint16_t)(~(1<<0));
	handleRGBState();
}

/*
RGB_LED has 8 states, helper function to handle bitwise clearing and setting
*/
void handleRGBState(void) {
	volatile uint8_t count = (P2->OUT & RGB_LED_MASK);
	count++;
	// Clear previous state
	P2->OUT &= ~(RGB_LED_MASK);
	// Set new state
	P2->OUT |= count & RGB_LED_MASK;
}
