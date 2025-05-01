// === main.c ===
#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "modbus.h"

#define TRIAC_PORT PORTB
#define TRIAC_PIN  PB0
#define DIR_PORT   PORTD
#define DIR_PIN    PD2
#define ZERO_CROSS_PIN  PC2

volatile uint16_t power_setting = 0;     // 0–1000 promili
volatile uint8_t zero_cross_detected = 0;
volatile uint16_t current_raw = 0;
volatile uint8_t triac_enable = 0;       // 1 = w³¹czony, 0 = wy³¹czony (domyœlnie wy³¹czony)

void triac_pulse() {
	TRIAC_PORT |= (1 << TRIAC_PIN);
	_delay_us(100);
	TRIAC_PORT &= ~(1 << TRIAC_PIN);
}

ISR(PCINT1_vect) {
	if (!(PINC & (1 << ZERO_CROSS_PIN))) {
		zero_cross_detected = 1;
	}
}

ISR(TIMER1_COMPA_vect) {
	triac_pulse();
}

void timer1_start(uint16_t delay_us) {
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = (F_CPU / 1000000) * delay_us / 8;
	TCNT1 = 0;
	TCCR1B |= (1 << WGM12) | (1 << CS11);
	TIMSK1 |= (1 << OCIE1A);
}

void timer1_stop(void) {
	TCCR1B = 0;
	TIMSK1 &= ~(1 << OCIE1A);
}

void setup() {
	DDRB |= (1 << TRIAC_PIN);
	DDRD |= (1 << DIR_PIN);
	DDRC &= ~(1 << ZERO_CROSS_PIN);
	PORTC |= (1 << ZERO_CROSS_PIN);

	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT10);

	sei();
	modbus_init();
}

uint16_t read_adc(void) {
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

void loop() {
	modbus_poll();
	current_raw = read_adc();

	if (zero_cross_detected) {
		zero_cross_detected = 0;

		if (triac_enable && power_setting > 0) {
			uint16_t delay = 5000 - ((power_setting * 5) / 10);
			timer1_start(delay);
			} else {
			timer1_stop();
		}
	}
}

void modbus_write_handler(uint16_t addr, uint16_t value) {
	if (addr == 0x0001) {
		if (value > 1000) value = 1000;
		power_setting = value;
		} else if (addr == 0x0003) {
		triac_enable = (value != 0);
	}
}

int main(void) {
	setup();
	while (1) {
		loop();
	}
}
