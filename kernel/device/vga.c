#include <external/multiboot.h>
#include <truth/slab.h>
#include <truth/string.h>
#include <truth/file.h>
#include <truth/lock.h>
#include <truth/log.h>
#include <truth/logo.h>
#include <truth/panic.h>
#include <truth/physical_allocator.h>
#include <truth/types.h>


#define VGA_Tab_Size 4


static enum status vga_write(const uint8_t *in, size_t size);


struct file VGA = {
    .type = File_Device,
    .name = "vga",
    .write = vga_write,
    .permissions = 0644,
    .obj = Object_Clear,
};


enum VGA_Color {
    VGA_Black = 0,
    VGA_Blue = 1,
    VGA_Green = 2,
    VGA_Cyan = 3,
    VGA_Red = 4,
    VGA_Magenta = 5,
    VGA_Brown = 6,
    VGA_Light_Grey = 7,
    VGA_Dark_Grey = 8,
    VGA_Light_Blue = 9,
    VGA_Light_Green = 10,
    VGA_Light_Cyan = 11,
    VGA_Light_Red = 12,
    VGA_Light_Magenta = 13,
    VGA_Light_Brown = 14,
    VGA_White = 15,
};

static const phys_addr VGA_Framebuffer_Phys = 0xb8000;
static uint16_t *VGA_Framebuffer;
static const size_t VGA_Framebuffer_Size = Page_Small;
static const size_t VGA_Height = 24;
static const size_t VGA_Width = 80;

static size_t VGA_Cursor_Row = 0;
static size_t VGA_Cursor_Column = 0;
static enum VGA_Color VGA_Current_Background = VGA_Black;
static enum VGA_Color VGA_Current_Foreground = VGA_White;


static void vga_free(struct object *unused(obj)) {
    lock_acquire_writer(&VGA.obj.lock);
    slab_free(VGA_Framebuffer_Size, VGA_Framebuffer);
    // Drop lock so writers deadlock instead of page faulting.
}


static inline uint8_t vga_entry_color(enum VGA_Color fg, enum VGA_Color bg) {
        return fg | bg << 4;
}
 
static inline uint16_t vga_entry(unsigned char uc, enum VGA_Color fg,
                                 enum VGA_Color bg) {
        return (uint16_t)uc | ((uint16_t)vga_entry_color(fg, bg) << 8);
}


static void vga_framebuffer_clear(void) {
    uint16_t clear_entry = vga_entry(' ', VGA_Current_Foreground,
                                      VGA_Current_Background);
    lock_acquire_writer(&VGA.obj.lock);
    for (size_t i = 0; i < VGA_Width * VGA_Height; ++i) {
        VGA_Framebuffer[i] = clear_entry;
    }
    lock_release_writer(&VGA.obj.lock);
}


static void vga_scroll_up_line(void) {
    VGA_Cursor_Row = 0;
    VGA_Cursor_Column = VGA_Height - 1;
    uint16_t vga_clear = vga_entry(' ', VGA_Current_Foreground,
            VGA_Current_Background);
    for (size_t y = 0; y < VGA_Height - 1; ++y) {
        for (size_t x = 0; x < VGA_Width; ++x) {
            VGA_Framebuffer[x + y * VGA_Width] =
                VGA_Framebuffer[x + (y + 1) * VGA_Width];
        }
    }
    for (size_t x = 0; x < VGA_Width; ++x) {
        VGA_Framebuffer[x + VGA_Width * (VGA_Height - 1)] = vga_clear;
    }
}


static void vga_putc_unlocked(char c) {
    size_t index = VGA_Cursor_Column * VGA_Width + VGA_Cursor_Row;
    assert(index < VGA_Width * VGA_Height);
    if (c == '\n') {
        VGA_Cursor_Row = 0;
        VGA_Cursor_Column++;
    } else if (c == '\t') {
        if (VGA_Cursor_Column + VGA_Tab_Size < VGA_Width) {
            for (size_t i = index; i < index + VGA_Tab_Size; ++i) {
                VGA_Framebuffer[i] = vga_entry(' ', VGA_Current_Foreground,
                                               VGA_Current_Background);
            }

            VGA_Cursor_Column++;
        } else {
            VGA_Cursor_Row = 0;
            VGA_Cursor_Column++;
        }
    } else {
        VGA_Framebuffer[index] = vga_entry(c, VGA_Current_Foreground,
                                                VGA_Current_Background);
        VGA_Cursor_Row = (VGA_Cursor_Row + 1) % VGA_Width;
        if (VGA_Cursor_Row == 0) {
            VGA_Cursor_Column++;
        }
    }
    if (VGA_Cursor_Column == VGA_Height) {
        vga_scroll_up_line();
    }
}


void vga_putc(const char c) {
    lock_acquire_writer(&VGA.obj.lock);
    vga_putc_unlocked(c);
    lock_release_writer(&VGA.obj.lock);
}


static void vga_puts(const char *str) {
    lock_acquire_writer(&VGA.obj.lock);
    for (size_t i = 0; str[i] != '\0'; ++i) {
        vga_putc_unlocked(str[i]);
    }
    vga_putc_unlocked('\n');
    lock_release_writer(&VGA.obj.lock);
}


static enum status vga_write(const uint8_t *in, size_t size) {
    lock_acquire_writer(&VGA.obj.lock);
    for (size_t i = 0; i < size; ++i) {
        vga_putc_unlocked(in[i]);
    }
    lock_release_writer(&VGA.obj.lock);
    return Ok;
}


enum status vga_init(void) {
    // FIXME: Uncomment when finished rewriting physical memory allocator.
    /*
    enum status status = physical_page_remove(VGA_Framebuffer_Phys);
    if (status != Ok) {
        return status;
    }*/
    file_attach(&Dev, &VGA);
    VGA_Framebuffer = slab_alloc_request_physical(VGA_Framebuffer_Phys,
                                                  Memory_Writable);
    if (VGA_Framebuffer == NULL) {
        physical_free(VGA_Framebuffer_Phys);
        file_detach(&Dev, &VGA);
        return Error_No_Memory;
    }
    object_set_free(&VGA.obj, vga_free);
    vga_framebuffer_clear();
    vga_puts(Logo);
    return Ok;
}
