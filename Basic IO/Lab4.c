/*
October 22, 2021
Lab 4 - Basic I/O
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
void config_switches(void);
void config_leds(void);

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
	
	// Infinite loop
	while(1) {
		static uint8_t flag_led_button_pressed;
    static uint8_t flag_mode_button_pressed;
		
		// Select LED Button Action
		if (!(P1->IN & (uint8_t)(1<<1)) && !flag_led_button_pressed) {
			uint16_t i = DELAY_VALUE;
			while (i--) {
				__asm volatile (""); // Delay loop contains some asm code placeholder
			}
			flag_led_button_pressed = 1;
			selected_led = (selected_led == LED_RED) ? LED_RGB : LED_RED;
		} else if (P1->IN & (1<<1)) {
			flag_led_button_pressed = 0;
		}
		
		// Select Mode Button Action
		if (!(P1->IN & (uint8_t)(1<<4)) && !flag_mode_button_pressed) { // Button 2 is pressed
			uint16_t i = DELAY_VALUE;
			while (i--) {
				__asm volatile (""); // Delay loop contains some asm code placeholder
			}
			flag_mode_button_pressed = 1;
			if (selected_led == LED_RED) {
				P1->OUT ^= (uint8_t)(1<<0);
			} else if (selected_led == LED_RGB) {
				uint8_t led_state = (P2->OUT & RGB_LED_MASK)>>0;
				led_state++;
				P2->OUT &= ~(RGB_LED_MASK);
				P2->OUT |= (led_state & RGB_LED_MASK)<<0;
			}
		} else if (P1->IN & (1<<4)) {
				flag_mode_button_pressed = 0;
			}
	}
	return 0;
}

void config_switches(void){
	//disables interrupts
	P1->IE &= (uint8_t)(~((1<<1)|(1<<4)));
	
	//sets function to GPIO
	P1->SEL0 &= (uint8_t)(~((1<<1)|(1<<4)));
	P1->SEL1 &= (uint8_t)(~((1<<1)|(1<<4)));
	
	//sets direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	//sets resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	//sets restistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

void config_leds(void){
	//disables interrupts
	P1->IE &= (uint8_t)(~(1<<0));
	
	//sets function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));
	
	//sets direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	//sets drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	//sets default state to off
	P1->OUT &= (uint8_t)(~(1<<0));
	
	//disables interrupts
	P2->IE &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	//sets function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	//sets direction of LED pins to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	//sets drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	//sets default state of all pins to off
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}
