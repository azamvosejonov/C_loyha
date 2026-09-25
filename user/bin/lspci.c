/* user/bin/lspci.c - PCI qurilmalari ro'yxati (Linux'dagi lspci kabi). */
#include "ulib.h"

int main(void)
{
    struct myos_pci_info d;
    for (int i = 0; pciinfo(i, &d) == 0; i++)
        printf("%02x:%02x.%u %04x:%04x [%02x%02x] %-24s %s\n", d.bus, d.dev, d.func, d.vendor,
               d.device, d.class_code, d.subclass, d.class_name, d.driver[0] ? d.driver : "-");
    return 0;
}
