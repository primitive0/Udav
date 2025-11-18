# Удав 🐍
**Удав (Udav)** --- это интерпретируемый императивный язык с питоноподобным синтаксисом и строгой динамической типизацией.

```
fun main():
    io.println("Hello, world!")
```

Точкой входа в программу считается функция `main()`.

### Комментарии.
```
#Comment

# This is the main function.
fun main():
    pass
```

### Типы данных.
Удав поддерживает целые 64-битные знаковые числа, строки и булев тип.

```
fun main():
    let this_is_integer = 007  # decimal 7
    let this_is_string = "Hello, world!"
```

### Переменные.
```
fun main():
    let x = 42
    let y = 8
    let result = x + y

    io.println("{}", result)  # 50
```

### Функции.
```
fun add(x, y):
    return x + y

fun main():
    let i = add(18, 24)
    let s = add("X ", "Y")
    
    io.println("i={}", i)  # i=42
    io.println("s={}", s)  # s=X Y
```

### Управляющие конструкции.
```
fun fizzbuzz(n):
    let i = 1
    while i <= n:
        if i % 15 == 0:
            io.println("FizzBuzz")
        elif i % 3 == 0:
            io.println("Fizz")
        elif i % 5 == 0:
            io.println("Buzz")
        else:
            io.println("{}", i)
        i += 1
            
fun main():
    fizzbuzz(100)
```

### Операции.
Удав поддерживает следующие операции:
- Арифметика, приоритеты обычные (+, -, *, /)
- Возведение в степень (**), является правоассоциативной операцией
- Остаток от деления (%)
- Логические операции (&&, ||)
- Побитовые операции (&, |, ^, <<, >>)

Операторы |, &, ^ применимы как целому типу, так и к логическому типу.
