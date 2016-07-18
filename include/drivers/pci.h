#ifndef _PCI_H
#define _PCI_H

#include <stdint.h>

#define PCI_VENDOR_ID            0x00
#define PCI_DEVICE_ID            0x02
#define PCI_COMMAND              0x04
#define PCI_STATUS               0x06
#define PCI_REVISION_ID          0x08

#define PCI_PROG_IF              0x09
#define PCI_SUBCLASS             0x0a
#define PCI_CLASS                0x0b
#define PCI_CACHE_LINE_SIZE      0x0c
#define PCI_LATENCY_TIMER        0x0d
#define PCI_HEADER_TYPE          0x0e
#define PCI_BIST                 0x0f
#define PCI_BAR0                 0x10
#define PCI_BAR1                 0x14
#define PCI_BAR2                 0x18
#define PCI_BAR3                 0x1C
#define PCI_BAR4                 0x20
#define PCI_BAR5                 0x24

#define PCI_INTERRUPT_LINE       0x3C

#define PCI_SECONDARY_BUS        0x09

#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT   0xCFC

#define PCI_NONE 0xFFFF

// A function registered by the driver. Takes a device number, a vendor ID, a
// device ID, and a some device specific extra data.
typedef void (*pci_func_t)(uint32_t device, uint16_t vendor_id,
        uint16_t device_id, void *extra);

// Get the bus ID of a device.
static inline uint8_t pci_get_bus(uint32_t dev) {
    return (dev >> 16) & 0xff;
}

// Get the slot ID of a device.
static inline uint8_t pci_get_slot(uint32_t dev) {
    return (dev >> 8) & 0xff;
}

// Get the function of a device.
static inline uint8_t pci_get_func(uint32_t dev) {
    return dev;
}

// Get the PCI address of field of a device.
static inline uint32_t pci_get_addr(uint32_t device, int field) {
    return 0x80000000 | (pci_get_bus(device) << 16) |
           (pci_get_slot(device) << 11) | (pci_get_func(device) << 8) |
           (field & 0xfc);
}

// Take a bus ID, a slot ID, and a function and box them up.
static inline uint32_t pci_box_device(int bus, int slot, int func) {
    return (bus << 16) | (slot << 8) | func;
}

// Read a field of a device on the PCI bus.
uint32_t pci_read_field(uint32_t dev, int field, int size);
// Write a field of a device on the PCI bus.
void pci_write_field(uint32_t dev, int field, int size, uint32_t value);
// Scan the PCI bus for devices.
void pci_scan(pci_func_t f, int type, void *extra);

#endif
