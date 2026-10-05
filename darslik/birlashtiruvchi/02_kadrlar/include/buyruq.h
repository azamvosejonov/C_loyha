/* buyruq.h - buyruq qatori: ./kadrlar royxat | varaqa ID | top | jami | izla ISM */
#ifndef BUYRUQ_H
#define BUYRUQ_H

#include "ombor.h"

/* argv[1] dagi buyruqni bajaradi. Qaytaradi: 0 - OK, 1 - foydalanuvchi xatosi (noma'lum buyruq, xodim topilmadi ...) */
int buyruq_bajar(struct ombor *o, int argc, char **argv);

#endif
