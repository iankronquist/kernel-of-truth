#include <stdint.h>

#include <arch/x86/io.h>
#include <drivers/pci.h>
#include <truth/kassert.h>

static void pci_scan_slot(pci_func_t f, int type, int bus, int slot,
        void *extra);
static void pci_scan_bus(pci_func_t f, int type, int bus, void *extra);
void pci_write_field(uint32_t dev, int field, int size, uint32_t value) {
    write_port(pci_get_addr(dev, field), PCI_ADDRESS_PORT);
    write_port(value, PCI_VALUE_PORT);
    switch (size) {
        case 4:
            write_port32(value, PCI_VALUE_PORT);
            break;
        case 2:
            write_port16(value, PCI_VALUE_PORT + (field & 2));
            break;
        case 1:
            write_port(value, PCI_VALUE_PORT + (field & 3));
            break;
        default:
            kassert(0);
    }

}

uint32_t pci_read_field(uint32_t dev, int field, int size) {
    write_port(pci_get_addr(dev, field), PCI_ADDRESS_PORT);
    switch (size) {
        case 4:
            return read_port32(PCI_VALUE_PORT);
        case 2:
            return read_port16(PCI_VALUE_PORT + (field & 2));
        case 1:
            return read_port(PCI_VALUE_PORT + (field & 3));
        default:
            kassert(0);
            return 0;
    }
}

/*
static void pci_dev_vend_lookup(unsigned short vendor_id,
        unsigned short device_id, const char **vend_str,
        const char **dev_str) {
    // Gotta love O(n) search times!
    // Note that if we find a valid vendor but can't find a valid ID we will
    // mark the vendor as unknown. This is an acceptable trade off for not
    // doing duplicate work.
    for (unsigned int i = 0; i < PCI_DEVTABLE_LEN; ++i) {
        if (PciVenTable[i].VenId == vendor_id &&
                PciDevTable[i].DevId == device_id) {
            *vend_str = PciDevTable[i].VenFull;
            *dev_str = PciDevTable[i].ChipDesc;
            return;
        }
    }
    *vend_str = "Unknown";
    *dev_str = "Unknown";
}

static const char *pci_device_lookup(unsigned short vendor_id,
        unsigned short device_id) {
    // Gotta love O(n) search times!
    for (unsigned int i = 0; i < PCI_DEVTABLE_LEN; ++i) {
        if (PciDevTable[i].VenId == vendor_id &&
                PciDevTable[i].DevId == device_id) {
            return PciDevTable[i].ChipDesc;
        }
    }
    return "Unknown";
}

static const char *pci_vendor_lookup(unsigned short vendor_id,
        unsigned short device_id) {
    // Gotta love O(n) search times!
    for (unsigned int i = 0; i < PCI_DEVTABLE_LEN; ++i) {
        if (PciVenTable[i].VenId == vendor_id) {
            return PciDevTable[i].VenFull;
        }
    }
    return "Unknown";
}
*/


uint16_t pci_find_type(uint32_t dev) {
    return (pci_read_field(dev, PCI_CLASS, 1) << 8) |
           pci_read_field(dev, PCI_CLASS, 1);
}

void pci_scan_hit(pci_func_t f, uint32_t dev, void *extra) {
    int dev_vend = (int)pci_read_field(dev, PCI_VENDOR_ID, 2);
    int dev_dvid = (int)pci_read_field(dev, PCI_DEVICE_ID, 2);
    f(dev, dev_vend, dev_dvid, extra);
}

static void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func,
        void *extra) {
    uint32_t dev = pci_box_device(bus, slot, func);
    if (type == -1 || type == pci_find_type(dev)) {
        pci_scan_hit(f, dev, extra);
    }
    if (pci_find_type(dev) == PCI_TYPE_BRIDGE) {
        pci_scan_bus(f, type, pci_read_field(dev, PCI_SECONDARY_BUS, 1),
                     extra);
    }
}

static void pci_scan_slot(pci_func_t f, int type, int bus, int slot,
        void *extra) {
    uint32_t dev = pci_box_device(bus, slot, 0);
    if (pci_read_field(dev, PCI_VENDOR_ID, 2) == PCI_NONE) {
        return;
    }
    pci_scan_func(f, type, bus, slot, 0, extra);
    if (!pci_read_field(dev, PCI_HEADER_TYPE, 1)) {
        return;
    }
    for (int func = 1; func < 8; ++func) {
        uint32_t dev = pci_box_device(bus, slot, func);
        if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
            pci_scan_func(f, type, bus, slot, func, extra);
        }
    }
}

static void pci_scan_bus(pci_func_t f, int type, int bus, void *extra) {
    for (int i = 0; i < 32; ++i) {
        pci_scan_slot(f, type, bus, i, extra);
    }
}

void pci_scan(pci_func_t f, int type, void *extra) {
    pci_scan_bus(f, type, 0, extra);
    if (!pci_read_field(0, PCI_HEADER_TYPE, 1)) {
        return;
    }

    for (int func = 1; func < 8; ++func) {
        uint32_t dev = pci_box_device(0, 0, func);
        if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
            pci_scan_bus(f, type, func, extra);
        } else {
            break;
        }
    }
}
