#include <iostream>

int main() {
    int n;
    std::cin >> n;
    long long f = 1;
    for (int i = 2; i <= n; i++)
        f *= i;
    std::cout << f << '\n';
}