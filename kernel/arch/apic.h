/* =============================================================================
 *  arch/apic.h - Local APIC va IO APIC
 * ============================================================================= */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Vektorlar */
#define VECTOR_LAPIC_TIMER 0x40
#define VECTOR_TLB_SHOOT   0xF0
#define VECTOR_RESCHEDULE  0xF1
#define VECTOR_STOP        0xF2
#define VECTOR_LAPIC_ERROR 0xFE
#define VECTOR_SPURIOUS    0xFF

extern bool apic_active;                /* false - eski PIC rejimi */

void lapic_init_bsp(void);              /* ACPI dan keyin, BSP da */
void lapic_init_ap(void);               /* har bir AP da */
uint32_t lapic_id(void);
void lapic_eoi(void);
/* dest - APIC ID. */
void lapic_send_ipi(uint32_t dest, uint8_t vector);
void lapic_send_ipi_all_but_self(uint8_t vector);
void lapic_send_init(uint32_t dest);
void lapic_send_sipi(uint32_t dest, uint8_t page);
/* Taymerni kalibrlash (BSP) va davriy rejimda ishga tushirish (har bir CPU). */
void lapic_timer_calibrate(void);
void lapic_timer_start(void);

void ioapic_init(void);
/* ISA IRQ ni vector ga, dest APIC ID li CPU'ga yo'naltirish. */
void ioapic_route_isa(uint8_t irq, uint8_t vector, uint32_t dest);
