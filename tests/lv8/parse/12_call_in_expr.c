// Call result used inside a larger expression.
int f() {
  return 1;
}

int main() {
  int x = f() + 2;
  return x;
}
