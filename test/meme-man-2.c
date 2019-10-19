void erast_odd();
const int number = 100;
int i, j;
int primes[number + 1] = {0};

void erast_even() {
  if ((i * i) <= number) {
    if (primes[i] != 0) {
      for (int j = 2; j < number; j++) {
        if (primes[i] * j > number)
          break;
        else
          primes[primes[i] * j] = 0;
      }
    }
    i++;
    erast_odd();
  }
}

void erast_odd() {
  if ((i * i) <= number) {
    if (primes[i] != 0) {
      for (int j = 2; j < number; j++) {
        if (primes[i] * j > number)
          break;
        else
          primes[primes[i] * j] = 0;
      }
    }
    i++;
    erast_even();
  }
}

int main() {
  for (i = 2; i <= number; i++)
    primes[i] = i;

  i = 2;
  erast_even();
  int max = 0;
  for (i = 0; i <= number; i++) {
    if (primes[i] != 0)
      if (primes[i] > max)
        max = primes[i];
  }
  return max;
}
