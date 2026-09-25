/* =============================================================================
 *  drivers/pci.h - PCI / PCI Express shinasi
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "lib/list.h"

struct pci_bar {
    uint64_t base;                      /* fizik manzil yoki port raqami */
    uint64_t size;
    bool is_io;                         /* true - I/O port, false - xotira (MMIO) */
    bool is_64;
    bool prefetchable;
};

struct pci_driver;

struct pci_device {
    uint8_t bus, dev, func;
    uint16_t vendor, device;
    uint8_t class_code, subclass, prog_if, revision;
    uint8_t header_type;
    uint8_t irq_line, irq_pin;
    struct pci_bar bars[6];
    struct pci_driver *driver;          /* qaysi drayver egallagan (NULL - hech kim) */
    void *driver_data;
    struct list_head node;
};

struct pci_driver {
    const char *name;
    /* Mos kelishini tekshirish: 0xFFFF / 0xFF = "istalgan". */
    uint16_t vendor, device;
    uint8_t class_code, subclass, prog_if;
    /* Qurilmani ishga tushirish. 0 - muvaffaqiyat. */
    int (*probe)(struct pci_device *dev);
    struct list_head node;
};

#define PCI_ANY_ID    0xFFFF
#define PCI_ANY_CLASS 0xFF

void pci_init(void);
void pci_register_driver(struct pci_driver *drv);

uint32_t pci_read32(const struct pci_device *d, uint8_t off);
uint16_t pci_read16(const struct pci_device *d, uint8_t off);
uint8_t pci_read8(const struct pci_device *d, uint8_t off);
void pci_write32(const struct pci_device *d, uint8_t off, uint32_t val);
void pci_write16(const struct pci_device *d, uint8_t off, uint16_t val);

/* Bus mastering (DMA) va xotira/port murojaatini yoqish. */
void pci_enable(struct pci_device *d);
/* Capability ro'yxatidan id ni topish (MSI = 0x05, MSI-X = 0x11). 0 - yo'q. */
uint8_t pci_find_capability(const struct pci_device *d, uint8_t id);
const char *pci_class_name(uint8_t class_code, uint8_t subclass);

/* index-chi qurilma (lspci uchun). */
struct pci_device *pci_get(int index);
int pci_count(void);
