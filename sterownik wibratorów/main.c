// === main.c (OSTATECZNA WERSJA z poprawionym debounce) ===
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "modbus.h"

#define TRIAC_PORT PORTB
#define TRIAC_PIN  PORTB0
#define DEBUG_LED_PIN PORTB1
#define DIR_PORT   PORTD
#define DIR_PIN    PORTD2
#define ZERO_CROSS_PIN  PORTC2
#define RESET_PIN  PORTD6

volatile uint16_t power_setting = 0;     // 0–1000 promili
volatile uint8_t zero_cross_detected = 0;
volatile uint8_t zero_cross_side = 0;    // 0 = ujemna, 1 = dodatnia
volatile uint16_t current_raw = 0;
volatile uint8_t triac_enable = 1;       // domyślnie włączony
volatile uint8_t pulse_scheduled = 0;    // zabezpieczenie przed podwójnym impulsem
volatile uint8_t zero_cross_block = 0;   // blokada ISR (debounce)

uint8_t EEMEM modbus_address_eeprom = 1;      // domyślny adres modbus
uint8_t EEMEM modbus_baudrate_code_eeprom = 1; // domyślny baudrate (4800)

uint8_t current_modbus_address;
uint8_t current_baudrate_code;

void triac_pulse() {
	PORTB |= (1 << DEBUG_LED_PIN);
	TRIAC_PORT |= (1 << TRIAC_PIN);
	_delay_us(100);
	TRIAC_PORT &= ~(1 << TRIAC_PIN);
	PORTB &= ~(1 << DEBUG_LED_PIN);
}

// Detekcja przejścia przez zero + określenie półfali + debounce
ISR(PCINT1_vect) {
	if (zero_cross_block == 0) {
		zero_cross_detected = 1;

		if (PINC & (1 << ZERO_CROSS_PIN)) {
			zero_cross_side = 1; // dodatnia półfala
			} else {
			zero_cross_side = 0; // ujemna półfala
		}

		zero_cross_block = 5; // blokada ISR na ok. 5 cykli (ok. 5-10ms)
	}
}

ISR(TIMER1_COMPA_vect) {
	triac_pulse();
	timer1_stop();
	pulse_scheduled = 0; // Impuls wykonany ? zezwól na kolejny w następnej półfali
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
	DDRB |= (1 << DEBUG_LED_PIN);
	DDRD |= (1 << DIR_PIN);
	DDRC &= ~(1 << ZERO_CROSS_PIN);
	PORTC |= (1 << ZERO_CROSS_PIN);

	DDRD &= ~(1 << RESET_PIN);
	PORTD |= (1 << RESET_PIN);

	_delay_ms(10);

	if (!(PIND & (1 << RESET_PIN))) {
		eeprom_update_byte(&modbus_address_eeprom, 1);
		eeprom_update_byte(&modbus_baudrate_code_eeprom, 1);
	}

	current_modbus_address = eeprom_read_byte(&modbus_address_eeprom);
	current_baudrate_code = eeprom_read_byte(&modbus_baudrate_code_eeprom);

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

	if (zero_cross_block > 0) zero_cross_block--; // Odliczanie blokady debounce

	if (zero_cross_detected) {
		zero_cross_detected = 0;

		if (zero_cross_side == 1) { // tylko dodatnia półfala
			if (triac_enable && power_setting > 0 && pulse_scheduled == 0) {
				uint16_t delay = (uint32_t)5000 * (1000 - power_setting) / 1000;
				timer1_start(delay);
				pulse_scheduled = 1;
				} else {
				timer1_stop();
				pulse_scheduled = 0;
			}
			} else {
			timer1_stop();
			pulse_scheduled = 0;
		}
	}
}

void modbus_write_handler(uint16_t addr, uint16_t value) {
	if (addr == 0x0001) {
		if (value > 1000) value = 1000;
		power_setting = value;
		} else if (addr == 0x0004) {
		if (value >= 1 && value <= 247) {
			current_modbus_address = value;
			eeprom_update_byte(&modbus_address_eeprom, value);
		}
		} else if (addr == 0x0005) {
		if (value >= 1 && value <= 3) {
			current_baudrate_code = value;
			eeprom_update_byte(&modbus_baudrate_code_eeprom, value);
		}
	}
}

int main(void) {
	setup();
	while (1) {
		loop();
	}
}
