#include "matn.h"
#include "test.h"

int main(void)
{
    BOSHLA("T5 id_nazorat");
    TEKSHIR(id_nazorat(1042), 3);
    TEKSHIR(id_nazorat(2087), 5);
    TEKSHIR(id_nazorat(9999), 6);
    TEKSHIR(id_nazorat(1000), 3);
    TEKSHIR(id_nazorat(999), -1);
    TEKSHIR(id_nazorat(10000), -1);
    TEKSHIR(id_nazorat(-5), -1);
    TUGAT();

    BOSHLA("T6 nom_uzunligi");
    TEKSHIR(nom_uzunligi(""), 0);
    TEKSHIR(nom_uzunligi("a"), 1);
    TEKSHIR(nom_uzunligi("Aziza"), 5);
    TEKSHIR(nom_uzunligi("Operatsion tizim"), 16);
    TUGAT();

    BOSHLA("T7 nom_teng");
    TEKSHIR(nom_teng("Bobur", "Bobur"), 1);
    TEKSHIR(nom_teng("Bob", "Bobur"), 0);
    TEKSHIR(nom_teng("Bobur", "Bob"), 0);
    TEKSHIR(nom_teng("Ali", "ali"), 0);
    TEKSHIR(nom_teng("", ""), 1);
    TEKSHIR(nom_teng("", "a"), 0);
    TUGAT();
    YAKUN();
}
