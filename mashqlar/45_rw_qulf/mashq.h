#pragma once

struct rwqulf;                              /* ichki tuzilishi - yechim.c da */

struct rwqulf *rw_yarat(void);
void rw_oqish_ol(struct rwqulf *q);
void rw_oqish_qoy(struct rwqulf *q);
void rw_yozish_ol(struct rwqulf *q);
void rw_yozish_qoy(struct rwqulf *q);
void rw_yoq(struct rwqulf *q);
