/*
November 22, 2021
SYSC 3310
Lab 5 - Basic I/O using Interrupts
Group 3
Bardia Parmoun 101143006
Kyra Lothrop 101145872
Trong Nguyen 100848232
*/

// Gives the peripheral device's register definitions.
#include "msp.h"

// Define constants
#define DELAY_VALUE 5000
#define RGB_LED_MASK 0x07

#define LED_RED 0
#define LED_RGB 1

// Function prototypes.
void config_switches(void);
void config_leds(void);

// Keeps track of the current LED.
static uint8_t volatile currentLED;
	
int main(void) {
	// RED LED is selected by default.
	currentLED = LED_RED;
	
	// Stop watchdog timer.
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	//intializes switches and LEDs.
	config_switches();
	config_leds();
	
  // Configure interrupts on falling edge
	P1->IES |= (uint8_t)((1<<1)|(1<<4));
	
	// Clear interrupt flags
	P1->IFG &= (uint8_t)~((1<<1)|(1<<4));
	
	// Interrupt pin enabled
	P1->IE |= (uint8_t)((1<<1)|(1<<4));
	
	//NVIC configuration.
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);

	//Globally enable interrupts in CPU.
	__ASM("CPSIE I");
	
	// Create infinite loop.
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
	
	// Sets direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	// Sets resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	// Sets restistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

/*
Configure LED settings.
*/
void config_leds(){	
	// Sets function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));
	
	/* P1.0 */
	// Sets direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	// Sets drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	// Sets default state to off
	P1->OUT &= (uint8_t)(~(1<<0));
	
	/* P2.0 P2.1 P2.2 */
	// Sets function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Sets direction of LED pins to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	// Sets drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Sets default state of all pins to off
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}

/*
Interrupt Service Routine with debouncing
*/
void PORT1_IRQHandler(void){
	// Debouncing the LEDs.
	static uint16_t i = DELAY_VALUE;
  while (i--) {
		// Delay loop contains some asm code placeholder
		__ASM volatile (""); 
  }
		
	//Test for pin 3 interrupt flag (Button 1 pressed)
	if((P1->IFG & (uint8_t)(1<<1)) != 0){
		// Clearing the interrupt flag.
		P1->IFG &= (uint8_t)~(1<<1);
		
		// Changing the LEDs.
		currentLED = (currentLED + 1) % 2;
	} else if((P1->IFG & (uint8_t)(1<<4)) != 0){
		// Clearing the interrupt flag.
		P1->IFG &= (uint8_t)~(1<<4);
		
		 // If current LED is LED 1
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
}
