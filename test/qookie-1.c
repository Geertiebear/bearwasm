int atoi(char *c) {
    int i = 0;
    while(*c) {
        i *= 10;
        i += *c - '0';
        c++;
    }
    return i;
}

// factorial
int main(int argc, char **argv) {
    int f = 1;
    for (int n = atoi(argv[1]); n > 0; n--) f *= n;
    return f;
}
