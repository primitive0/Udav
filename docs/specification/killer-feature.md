# Киллер фичи языка Udav

## Диагностика

Диагностика ошибок будет реализована как в языке Clang:

```cpp
int main() {
```

```bash
/tmp/foo.cpp: In function ‘int main()’:
/tmp/foo.cpp:1:13: error: expected ‘}’ at end of input
    1 | int main() {
      |            ~^
```
