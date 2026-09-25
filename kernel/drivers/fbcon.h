/* =============================================================================
 *  drivers/fbcon.h - framebuffer (grafik) konsoli
 * ============================================================================= */
#pragma once

#include "boot/bootinfo.h"
#include "drivers/console.h"

/* Framebuffer'ni xaritalab, matn konsolini tayyorlaydi. Xato bo'lsa NULL. */
const struct screen_ops *fbcon_init(const struct boot_framebuffer *fb);
