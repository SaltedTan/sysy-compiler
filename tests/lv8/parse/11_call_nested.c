// Nested calls: a call result used as an argument.
int g(int x) {
  return 1;
}

int f(int a, int b) {
  return 2;
}

int main() {
  return f(g(1), 2);
}
