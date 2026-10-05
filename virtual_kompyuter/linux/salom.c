/* salom.c — init dan "run /bin/salom" bilan ishga tushiriladigan alohida dastur (fork + execve namunasi) */
int main(int argc, char **argv)
{
    printf("Men alohida jarayonman: pid %d, ota %d, argumentlar: %d ta\n", getpid(), getppid(), argc);
    for (int i = 0; i < argc; i++)
        printf("  argv[%d] = %s\n", i, argv[i]);
    return 7;
}
