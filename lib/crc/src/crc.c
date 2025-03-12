#include "crc.h"
#include <limits.h>

extern uint8_t crc8(uint8_t crc, const void *data, size_t len) {
    if (data == NULL)
        return (uint8_t)-1;

    const uint8_t *l_data = data;
    uint8_t acc;
    for (size_t i = 0; i < len; i++) {
        acc = l_data[i] ^ crc;
        crc = 0;
        if ((acc & 0x01) != 0)
            crc ^= 0x5E;
        if ((acc & 0x02) != 0)
            crc ^= 0xBC;
        if ((acc & 0x04) != 0)
            crc ^= 0x61;
        if ((acc & 0x08) != 0)
            crc ^= 0xC2;
        if ((acc & 0x10) != 0)
            crc ^= 0x9D;
        if ((acc & 0x20) != 0)
            crc ^= 0x23;
        if ((acc & 0x40) != 0)
            crc ^= 0x46;
        if ((acc & 0x80) != 0)
            crc ^= 0x8C;
    }
    return crc;
}

extern uint16_t crc16(uint16_t crc, const void *data, size_t len) {
    if (data == NULL)
        return (uint16_t)-1;

    const uint8_t *l_data = data;
    uint8_t acc;
    for (size_t i = 0; i < len; i++) {
        acc = l_data[i] ^ crc;
        crc >>= CHAR_BIT;
        if ((acc & 0x01) != 0)
            crc ^= 0xC0C1;
        if ((acc & 0x02) != 0)
            crc ^= 0xC181;
        if ((acc & 0x04) != 0)
            crc ^= 0xC301;
        if ((acc & 0x08) != 0)
            crc ^= 0xC601;
        if ((acc & 0x10) != 0)
            crc ^= 0xCC01;
        if ((acc & 0x20) != 0)
            crc ^= 0xD801;
        if ((acc & 0x40) != 0)
            crc ^= 0xF001;
        if ((acc & 0x80) != 0)
            crc ^= 0xA001;
    }
    return crc;
}

extern uint32_t crc32(uint32_t crc, const void *data, size_t len) {
    if (data == NULL)
        return (uint32_t)-1;

    const uint8_t *l_data = data;
    uint8_t acc;
    for (size_t i = 0; i < len; i++) {
        acc = l_data[i] ^ crc;
        crc >>= CHAR_BIT;
        if ((acc & 0x01) != 0)
            crc ^= 0x04C11DB7;
        if ((acc & 0x02) != 0)
            crc ^= 0x09823B6E;
        if ((acc & 0x04) != 0)
            crc ^= 0x130476DC;
        if ((acc & 0x08) != 0)
            crc ^= 0x2608EDB8;
        if ((acc & 0x10) != 0)
            crc ^= 0x4C11DB70;
        if ((acc & 0x20) != 0)
            crc ^= 0x9823B6E0;
        if ((acc & 0x40) != 0)
            crc ^= 0x34867077;
        if ((acc & 0x80) != 0)
            crc ^= 0x690CE0EE;
    }
    return crc;
}

extern uint64_t crc64(uint64_t crc, const void *data, size_t len) {
    if (data == NULL)
        return (uint64_t)-1;

    const uint8_t *l_data = data;
    uint8_t acc;
    for (size_t i = 0; i < len; i++) {
        acc = l_data[i] ^ crc;
        crc >>= CHAR_BIT;
        if ((acc & 0x01) != 0)
            crc ^= 0x7AD870C830358979;
        if ((acc & 0x02) != 0)
            crc ^= 0xF5B0E190606B12F2;
        if ((acc & 0x04) != 0)
            crc ^= 0xC038E5739841B68F;
        if ((acc & 0x08) != 0)
            crc ^= 0xAB28ECB46814FE75;
        if ((acc & 0x10) != 0)
            crc ^= 0x7D08FF3B88BE6F81;
        if ((acc & 0x20) != 0)
            crc ^= 0xFA11FE77117CDF02;
        if ((acc & 0x40) != 0)
            crc ^= 0xDF7ADABD7A6E2D6F;
        if ((acc & 0x80) != 0)
            crc ^= 0x95AC9329AC4BC9B5;
    }
    return crc;
}
