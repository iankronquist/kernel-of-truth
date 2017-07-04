#include <arch/x64/port.h>
#include <arch/x64/cpu.h>
#include <truth/interrupts.h>
#include <truth/drive.h>
#include <truth/lock.h>
#include <truth/scheduler.h>
#include <truth/log.h>
#include <truth/panic.h>

#define TEST 0

#define ATA_IRQ_Number 0x2e
#define ATA_Max_Drive_Probe 1000

#define ATA_Busy        0x80
#define ATA_Drive_Ready 0x40
#define ATA_Disk_Fault  0x20
#define ATA_Error       0x01

#define ATA_Command_Read           0x20
#define ATA_Command_Write          0x30
#define ATA_Command_Read_Multiple  0xc4
#define ATA_Command_Write_Multiple 0xc5
#define ATA_Interrupt_Enable 0

#define ATA_Drive_Max 2

#define ATA_Status_Busy                0x80
#define ATA_Status_Drive_Ready         0x40
#define ATA_Status_Drive_Fault         0x20
#define ATA_Status_Drive_Seek_Complete 0x10
#define ATA_Status_Drive_Request_Ready 0x08
#define ATA_Status_Corrected_Data      0x04
#define ATA_Status_Index               0x02
#define ATA_Status_Error               0x01

#define ATA_Register_Data       0x00
#define ATA_Register_Error      0x01
#define ATA_Register_Features   0x01
#define ATA_Register_Sec_Count0 0x02
#define ATA_Register_LBA0       0x03
#define ATA_Register_LBA1       0x04
#define ATA_Register_LBA2       0x05
#define ATA_Register_Drive_Sel  0x06
#define ATA_Register_Command    0x07
#define ATA_Register_Status     0x07
#define ATA_Register_Sec_Count1 0x08
#define ATA_Register_LBA3       0x09
#define ATA_Register_LBA4       0x0a
#define ATA_Register_LBA5       0x0b
#define ATA_Register_Control    0x0c
#define ATA_Register_Alt_Status 0x0c
#define ATA_Register_Address    0x0d

#define ATA_Control0 0x376
#define ATA_Control1 0x3f6
#define ATA_Data0 0x170
#define ATA_Data1 0x1f0

#define ATA_Drive_Number(x) (((x) & 1) << 4)


static struct lock ata_lock = Lock_Clear;
static struct drive_buffer *ata_queue = NULL;

static bool Drives[ATA_Drive_Max];


static void ata_use_drive(int drive) {
    write_port(0xe0 | ATA_Drive_Number(drive), ATA_Data1 + ATA_Register_Drive_Sel);
}


static enum status ata_wait(int drive, bool check_error) {
    uint8_t drive_status;

    while (((drive_status = read_port(ATA_Data1 + ATA_Register_Status)) & (ATA_Busy | ATA_Drive_Ready)) != ATA_Drive_Ready) {
        cpu_pause();
    }

    if (check_error && (drive_status & (ATA_Disk_Fault | ATA_Error)) != 0) {
        return Error_IO;
    }

    return Ok;
}


static void ata_issue_request(struct drive_buffer *b) {

    assert(b != NULL);

    int sectors_per_block = Drive_Buffer_Size/Drive_Sector_Size;
    int sector = b->block_number * sectors_per_block;

    int read_cmd = (sectors_per_block == 1) ? ATA_Command_Read :  ATA_Command_Read_Multiple;
    int write_cmd = (sectors_per_block == 1) ? ATA_Command_Write : ATA_Command_Write_Multiple;

    assert(sectors_per_block < 8);

    ata_use_drive(b->dev);
    ata_wait(b->dev, false);

    write_port(ATA_Interrupt_Enable, ATA_Control1);
    write_port(sectors_per_block, ATA_Data1 + ATA_Register_Sec_Count0);

    write_port(sector & 0xff, ATA_Data1 + ATA_Register_LBA0);
    write_port((sector >> 8) & 0xff, ATA_Data1 + ATA_Register_LBA1);

    write_port((sector >> 16) & 0xff, ATA_Data1 + ATA_Register_LBA2);
    write_port(0xe0 | ATA_Drive_Number(b->dev) | ((sector >> 24) & 0x0f), ATA_Data1 + ATA_Register_Drive_Sel);

    if (b->flags & Drive_Buffer_Dirty){
        write_port(write_cmd, ATA_Data1 + ATA_Register_Command);
        write_port_buffer32(b->data, Drive_Buffer_Size/sizeof(uint32_t), ATA_Data1 + ATA_Register_Data);
    } else {
        write_port(read_cmd, ATA_Data1 + ATA_Register_Command);
    }
}

static void ata_queue_append(struct drive_buffer *b) {
    struct drive_buffer **pp;
    b->next = 0;
    for (pp=&ata_queue; *pp; pp=&(*pp)->next) { }
    *pp = b;
}

void ata_request(struct drive_buffer *b) {

    //assert(lock_is_writer_acquired(&b->lock));

    assert((b->flags & (Drive_Buffer_Read | Drive_Buffer_Dirty)) != Drive_Buffer_Read);
    assert(b->dev < ATA_Drive_Max);
    assert(Drives[b->dev] == true);

    lock_acquire_writer(&ata_lock);

    ata_queue_append(b);

    if (ata_queue == b) {
        lock_release_writer(&ata_lock);
        ata_issue_request(b);
    } else {
        lock_release_writer(&ata_lock);
    }

    while ((b->flags & (Drive_Buffer_Read | Drive_Buffer_Dirty)) != Drive_Buffer_Read){
        scheduler_yield();
    }
}


static bool ata_handler(struct interrupt_cpu_state *unused(_)) {
    bool handled = true;
    uint8_t drive_status;
    struct drive_buffer *b;

    lock_acquire_writer(&ata_lock);

    b = ata_queue;
    if (b == NULL){
        log(Log_Warning, "Spurious ATA interrupt");
        handled = false;
        goto out;
    }

    ata_use_drive(b->dev);
    drive_status = read_port(ATA_Data1 + ATA_Register_Status);
    if ((drive_status & ATA_Status_Drive_Ready) == 0) {
        log(Log_Info, "ATA interrupt handler passing");
        handled = false;
        goto out;
    }

    ata_queue = b->next;

    if (drive_status & ATA_Status_Error) {
        b->flags |= Drive_Buffer_Error;

        handled = true;
        goto out;
    }

    if (!(b->flags & Drive_Buffer_Dirty) && ata_wait(b->dev, true) == Ok) {
        read_port_buffer32(b->data, Drive_Buffer_Size/sizeof(uint32_t), ATA_Data1 + ATA_Register_Data);
    }

    b->flags |= Drive_Buffer_Read;
    b->flags &= ~Drive_Buffer_Dirty;

    if (ata_queue != NULL) {
        ata_issue_request(ata_queue);
    }

out:
    lock_release_writer(&ata_lock);
    return handled;
}

#ifdef TEST
static void ata_test_read(void) {
    struct drive_buffer b = {
        .flags = 0,
        .dev = 1,
        .block_number = 2,
        .next = NULL,
        .data = {'Z'},
    };

    ata_request(&b);
    while ((b.flags & Drive_Buffer_Dirty) != 0) {
        cpu_pause();
    }

    b.data[sizeof(b.data)-1] = '\0';
    logf(Log_Info, "data %s\n", b.data);
}
#endif // TEST

constructor enum status ata_init(void) {

    for (size_t drive = 0; drive < ATA_Drive_Max; ++drive) {
        ata_use_drive(drive);
        for (size_t i = 0; i < ATA_Max_Drive_Probe; ++i) {
            if (read_port(ATA_Data1 + ATA_Register_Status) != 0){
                Drives[drive] = true;
                break;
            }
        }
        logf(Log_Info, "Drive %zu is %spresent\n", drive, Drives[drive] ? "" : "not ");
    }

    interrupt_register_handler(Interrupt_Number_ATA0, ata_handler);
#ifdef TEST
    ata_test_read();
#endif
    return Ok;
}
