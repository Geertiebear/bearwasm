int main() {
    static int cnt = 0;
    cnt++;
    if(cnt < 10) {
         main();
    }
    return cnt;
}
