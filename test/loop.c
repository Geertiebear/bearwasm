int main(int argc, char **argv) {
	int num  = argv[1][0] - '0';
	int multiplier = argv[2][0] - '0';
	int sum = 0;
	for (int i = 0; i < multiplier; i++)
		sum += num;
	return sum;
}
