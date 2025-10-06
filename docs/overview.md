# Удав 🐍
**Удав (Udav)** --- это компилируемый императивный язык с питоноподобным синтаксисом и статической типизацией.

```
fun main():
    fmt.println("Hello, world!")
```

### Комментарии.
```
# Comment

#[
  This program
  does nothing.
]#

fun main():
    pass
```

### Переменные.
```
fun main():
    let x = 42
    let y = 8
    let result = x + y

    fmt.println("result={}", result)
```

```
fun main():
    let foo i32 = 42
    let bar string = "One string to rule them all."
```

### Функции.
```
fun add_numbers(x i32, y i32) -> i32:
    return x + y

fun main():
    let x = 18
    let result = add_numbers(x, 24)
    fmt.println("result={}", result)
```

### Управляющие конструкции.
```
fun fizzbuzz(n i32):
    i = 0
    while i < n:
        i += 1
        if i % 5 == 0 && i % 3 == 0:
            if i % 3 == 0:
                fmt.print("Fizz")
            if i % 5 == 0:
                fmt.print("Buzz")
            fmt.println()
        else:
            fmt.println("{}", i)

fun main():
    fizzbuzz(100)
```

### Операции.
Удав поддерживает следующие операции:
- Арифметика, приоритеты обычные (+, -, *, /)
- Возведение в степень (**)
- Остаток от деления (%)
- Логические операции (&&, ||)
- Побитовые операции (&, |, ^, <<, >>)

Оператор ^ применим как целочисленным типам, так и к логическому типу.
