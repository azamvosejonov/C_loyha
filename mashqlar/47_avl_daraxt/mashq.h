#pragma once

struct avl {
    int kalit;
    int balandlik;          /* barg = 1, bo'sh daraxt (NULL) = 0 */
    struct avl *chap, *ong;
};

struct avl *avl_qosh(struct avl *ildiz, int kalit);
int avl_bormi(const struct avl *ildiz, int kalit);
struct avl *avl_ochir(struct avl *ildiz, int kalit);
void avl_ozod(struct avl *ildiz);
