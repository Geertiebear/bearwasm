int add(char **argv) {
    int a = argv[1][0] - '0';
    int b = argv[2][0] - '0';
    return a + b;
}

int main(int argc, char **argv) {
    return add(argv);
}
