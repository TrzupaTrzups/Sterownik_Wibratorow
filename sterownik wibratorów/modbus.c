// === modbus.c ===
#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "modbus.h"

#define MODBUS_ADDRESS 0x01
#define DIR_PORT PORTD
#define DIR_PIN  PD2
#define USART_BAUDRATE 4800
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

volatile uint8_t rx_buffer[8];
volatile uint8_t rx_index = 0;
volatile uint8_t frame_ready = 0;

extern volatile uint8_t power_setting;
extern volatile uint16_t current_raw;
extern volatile uint8_t triac_enable;

uint16_t modbus_crc(uint8_t *buf, uint8_t len) {
	uint16_t crc = 0xFFFF;
	for (uint8_t pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos];
		for (uint8_t i = 8; i != 0; i--) {
			if ((crc & 0x0001) != 0) {
				crc >>= 1;
				crc ^= 0xA001;
				} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

void modbus_init(void) {
	UBRR0H = (BAUD_PRESCALE >> 8);
	UBRR0L = BAUD_PRESCALE;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

ISR(USART_RX_vect) {
	if (rx_index < sizeof(rx_buffer)) {
		rx_buffer[rx_index++] = UDR0;
		if (rx_index >= 8) {
			frame_ready = 1;
		}
	}
}

void modbus_poll(void) {
	if (!frame_ready) return;
	frame_ready = 0;

	if (rx_buffer[0] != MODBUS_ADDRESS) {
		rx_index = 0;
		return;
	}

	uint8_t func = rx_buffer[1];
	uint16_t crc = modbus_crc(rx_buffer, 6);
	uint16_t crc_received = (rx_buffer[7] << 8) | rx_buffer[6];
	if (crc != crc_received) {
		rx_index = 0;
		return;
	}

	uint16_t reg_addr = (rx_buffer[2] << 8) | rx_buffer[3];
	uint16_t reg_value = (rx_buffer[4] << 8) | rx_buffer[5];

	if (func == 0x06) {
		modbus_write_handler(reg_addr, reg_value);
		DIR_PORT |= (1 << DIR_PIN);
		_delay_us(100);
		for (uint8_t i = 0; i < 6; i++) {
			while (!(UCSR0A & (1 << UDRE0))) ;
			UDR0 = rx_buffer[i];
		}
		crc = modbus_crc(rx_buffer, 6);
		while (!(UCSR0A & (1 << UDRE0))) ; UDR0 = crc & 0xFF;
		while (!(UCSR0A & (1 << UDRE0))) ; UDR0 = crc >> 8;
		while (!(UCSR0A & (1 << TXC0))) ; UCSR0A |= (1 << TXC0);
		DIR_PORT &= ~(1 << DIR_PIN);
	}
	else if (func == 0x03 || func == 0x04) {
		uint8_t response[7];
		response[0] = rx_buffer[0];
		response[1] = func;
		response[2] = 2;

		uint16_t value = 0;
		if (reg_addr == 0x0001) {
			value = power_setting;
			} else if (reg_addr == 0x0002) {
			value = current_raw;
			} else if (reg_addr == 0x0003) {
			value = triac_enable;
		}

		response[3] = value >> 8;
		response[4] = value & 0xFF;
		uint16_t r_crc = modbus_crc(response, 5);
		response[5] = r_crc & 0xFF;
		response[6] = r_crc >> 8;

		DIR_PORT |= (1 << DIR_PIN);
		_delay_us(100);
		for (uint8_t i = 0; i < 7; i++) {
			while (!(UCSR0A & (1 << UDRE0))) ;
			UDR0 = response[i];
		}
		while (!(UCSR0A & (1 << TXC0))) ; UCSR0A |= (1 << TXC0);
		DIR_PORT &= ~(1 << DIR_PIN);
	}
	else if (func == 0x05) { // Write Single Coil
		if (reg_addr == 0x0001) {
			if (reg_value == 0xFF00) {
				triac_enable = 1;
				} else if (reg_value == 0x0000) {
				triac_enable = 0;
			}

			DIR_PORT |= (1 << DIR_PIN);
			_delay_us(100);
			for (uint8_t i = 0; i < 6; i++) {
				while (!(UCSR0A & (1 << UDRE0))) ;
				UDR0 = rx_buffer[i];
			}
			crc = modbus_crc(rx_buffer, 6);
			while (!(UCSR0A & (1 << UDRE0))) ; UDR0 = crc & 0xFF;
			while (!(UCSR0A & (1 << UDRE0))) ; UDR0 = crc >> 8;
			while (!(UCSR0A & (1 << TXC0))) ; UCSR0A |= (1 << TXC0);
			DIR_PORT &= ~(1 << DIR_PIN);
		}
	}
	else if (func == 0x01) { // Read Coils
		if (reg_addr == 0x0001) {
			uint8_t response[6];
			response[0] = rx_buffer[0]; // slave ID
			response[1] = 0x01;         // function code
			response[2] = 0x01;         // byte count
			response[3] = triac_enable ? 0x01 : 0x00;
			uint16_t r_crc = modbus_crc(response, 4);
			response[4] = r_crc & 0xFF;
			response[5] = r_crc >> 8;

			DIR_PORT |= (1 << DIR_PIN);
			_delay_us(100);
			for (uint8_t i = 0; i < 6; i++) {
				while (!(UCSR0A & (1 << UDRE0))) ;
				UDR0 = response[i];
			}
			while (!(UCSR0A & (1 << TXC0))) ; UCSR0A |= (1 << TXC0);
			DIR_PORT &= ~(1 << DIR_PIN);
		}
	}

	rx_index = 0;
}
