/* =============================================================================
 *  user/bin/ps.c - jarayonlar ro'yxati
 * ============================================================================= */
#include <stdio.h>
#include "myos.h"

static const char *state_name(int s)
{
    switch (s) {
    case MYOS_PROC_EMBRYO:  return "yaratilmoqda";
    case MYOS_PROC_READY:   return "tayyor";
    case MYOS_PROC_RUNNING: return "ishlayapti";
    case MYOS_PROC_BLOCKED: return "kutyapti";
    case MYOS_PROC_ZOMBIE:  return "zombie";
    default:                return "?";
    }
}

int main(void)
{
    static struct myos_proc_info procs[64];
    int n = ps(procs, 64);
    printf("  PID  PPID  TUR     HOLAT         CPU(tik)  XOTIRA  NOMI\n");
    for (int i = 0; i < n; i++) {
        struct myos_proc_info *p = &procs[i];
        printf("%5d %5d  %-6s  %-12s  %8lu  %4lu KB  %s\n", p->pid, p->ppid,
               p->is_user ? "user" : "yadro", state_name(p->state), p->cpu_ticks,
               p->mem_pages * 4, p->name);
    }
    return 0;
}
