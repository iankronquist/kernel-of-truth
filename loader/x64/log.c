#include <loader/log.h>

static uint16_t *const Boot_VGA_Window = (uint16_t *)0xb8000;
static size_t Boot_VGA_Window_Index = 0;

void boot_vga_log64(const char *string) {
    for (const char *c = string; *c != '\0'; ++c) {
        Boot_VGA_Window[Boot_VGA_Window_Index] = 0x0f00 | *c;
        Boot_VGA_Window_Index++;
    }
}

void boot_log_number(uint64_t n) {
    const size_t nibbles = sizeof(uint64_t) * 2;
    char number[nibbles + 1];
    for (size_t i = 0; i < nibbles; ++i) {
        int nibble = n % 16;
        n /= 16;
        if (nibble < 10) {
            number[nibbles - i - 1] = nibble + '0';
        } else {
            number[nibbles - i - 1] = nibble + 'a' - 10;
        }
    }

    number[nibbles] = '\0';
    boot_vga_log64(" ");
    boot_vga_log64((const char *)&number);
    boot_vga_log64(" ");
}
