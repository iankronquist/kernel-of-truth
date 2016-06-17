#include <drivers/ahci/ahci.h>
#define AHCI_DEV_SATA   0x00000101
#define AHCI_DEV_SATAPI 0xeb140101
#define AHCI_DEV_SEMB   0xc33c0101
#define AHCI_DEV_PM     0x96690101

/*
    The hba memory structure has a field which indicates which ports are
    implemented in hardware. Check each bit to see if it's present.
*/
void probe_port(struct hba_memory *mem) {
    uint32_t port_implemented = mem->port_implemented;
    for (uint32_t i = 0; i < 32; ++i) {
        // If the port is present.
        if (port_implemented & 1) {
            // Determine the type of the port.
            switch (check_port_type(mem->ports[i])) {
                case AHCI_DEV_SATA:
                    break;
                case AHCI_DEV_SATAPI:
                    break;
                case AHCI_DEV_SEMB:
                    break;
                case AHCI_DEV_PM:
                    break;
                default:
                    break;
            }
        }
        // Move on to the next port.
        port_implemented >>= 1;
    }
}

static check_port_type(struct hba_port *port) {
    uint32_t status = port->sata_status;
    uint8_t ipm = (status >> 8) & 0xf;
    uint8_t det = status & 0xf;
    if (det != H
}
