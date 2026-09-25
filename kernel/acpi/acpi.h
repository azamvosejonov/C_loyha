/* =============================================================================
 *  acpi/acpi.h - ACPI jadvallari (MADT, FADT, ...)
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define ACPI_MAX_CPUS    64
#define ACPI_MAX_IOAPICS 8

struct acpi_cpu {
    uint32_t apic_id;
    uint32_t acpi_id;
};

struct acpi_ioapic {
    uint8_t id;
    uint64_t phys;                      /* MMIO manzili */
    uint32_t gsi_base;                  /* birinchi "global system interrupt" raqami */
};

/* ISA IRQ -> GSI qayta yo'naltirish (masalan, taymer IRQ0 odatda GSI2 ga). */
struct acpi_irq_override {
    uint32_t gsi;
    bool active_low;
    bool level_triggered;
};

struct acpi_info {
    bool present;
    uint8_t revision;
    char oem[7];
    uint64_t lapic_phys;
    struct acpi_cpu cpus[ACPI_MAX_CPUS];
    int cpu_count;
    struct acpi_ioapic ioapics[ACPI_MAX_IOAPICS];
    int ioapic_count;
    struct acpi_irq_override isa[16];   /* har bir ISA IRQ uchun (override bo'lmasa: gsi = irq) */
    bool legacy_pic;                    /* MADT: 8259 PIC ham bor (uni o'chirish kerak) */
    /* FADT dan: */
    uint16_t pm1a_cnt, pm1b_cnt;
    uint16_t slp_typa, slp_typb;
    bool s5_found;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    bool reset_supported;
    uint64_t reset_addr;
    uint8_t reset_space, reset_value;
    uint64_t mcfg_base;                 /* PCIe ECAM (0 = yo'q) */
};

extern struct acpi_info acpi;

void acpi_init(void);
/* Kompyuterni o'chirish (S5). Muvaffaqiyatsiz bo'lsa qaytadi. */
void acpi_poweroff(void);
/* Qayta yuklash. */
void acpi_reboot(void);
