

int 
setenv (const char *name, const char *value, int overwrite)
{
    return 0;
}

int
atoi(const char *s)
{
    int n;

    n = 0;
    while('0' <= *s && *s <= '9')
        n = n*10 + *s++ - '0';

    return n;
}
