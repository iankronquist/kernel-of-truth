#pragma once

#include <truth/types.h>

/* Write to an I/O port.
 * A wrapper around outb. Takes a value to write and a port to write it to.
 */
static inline void write_port(uint8_t data, uint16_t port) {
    __asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (data));
}


/* Write two bytes to an I/O port. */
static inline void write_port16(uint16_t data, uint16_t port) {
    __asm__ volatile ("outw %1, %0" : : "dn" (port), "a" (data));
}

/* Write a dword to an I/O port. */
static inline void write_port32(uint32_t data, uint16_t port) {
    __asm__ volatile ("outl %1, %0" : : "dn" (port), "a" (data));
}

/* Read from an I/O port.
 * A wrapper around inb. Takes a port and returns its value.
 */
static inline uint8_t read_port(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

/* Read two bytes from an I/O port. */
static inline uint16_t read_port16(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

/* Read a dword from an I/O port. */
static inline uint32_t read_port32(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}


/* Read a buffer from an I/O port in 32 bit increments. */
static inline void read_port_buffer32(void *addr, size_t size, const uint16_t port) {
  __asm__ volatile("cld; rep insl" : "=D" (addr), "=c" (size) : "d" (port), "0" (addr), "1" (size) : "memory", "cc");
}


/* Write a buffer to an I/O port in 32 bit increments. */
static inline void write_port_buffer32(const void *addr, size_t size, const uint16_t port) {
  __asm__ volatile("cld; rep outsl" : "=S" (addr), "=c" (size) : "d" (port), "0" (addr), "1" (size) : "cc");
}
