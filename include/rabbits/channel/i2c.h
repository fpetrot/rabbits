#ifndef _I2C_H
#define _I2C_H

typedef struct s_i2c_packet {
    uint16_t addr; // 7-bit or 10-bit addressing mode
    uint8_t data;
} i2c_packet;

#endif

