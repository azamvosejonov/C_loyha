/* =============================================================================
 *  drivers/vga.h - VGA matn rejimi (80x25) ekrani
 * ============================================================================= */
#pragma once

#include "drivers/console.h"

/* BIOS rejimida, framebuffer o'rniga matn rejimi berilganda ishlatiladi. */
const struct screen_ops *vga_text_init(void);
