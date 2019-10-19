long atol(char *s) {
    long n = 0;
    while(*s) {
        n *= 10L;
        n += (long)(*s - '0');
        s++;
    }
    return n;
}
main(c, v) char**v; {
    long z = 1, n = atol(v[1]), i = 0;
    for(; i++, n > 9; n = z, z = 1)
        for(; z *= n % 10, n /= 10;);
    return i;
}
