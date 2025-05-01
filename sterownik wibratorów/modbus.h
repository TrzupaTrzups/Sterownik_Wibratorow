#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

void modbus_init(void);
void modbus_poll(void);
void modbus_write_handler(uint16_t addr, uint16_t value);

#endif
