# Грамматика и правила языка Udav

## Правила основной программы

Основная программа в языке начинается с функции `main`, которая представляет собой точку входа. Строки в функции `main`
представляют собой инструкции (или `statements`), которые выполняются последовательно.
В программе не должно быть 2х функций `main` одновременно

**Пример:**

```udav
func main():
    pass
```

## Область действия переменных

### Правила

- Переменные, объявленные на уровне функции, доступны во всей функции и только внутри этой функции.
- Переменные, объявленные внутри вложенного блока, доступны только внутри этого блока.
- Попытка обратиться к недоступной переменной заканчивается ошибкой.

**Пример:**

```Udav
fun foo():
    print(a)  # Ошибка: обращение к `a` хоть и находится на одном уровне с её объявлением, недоступна, так как находится в другой функции

fun main():
    let a = 3  # уровень IDENT=1
    if true:
        print(a)  # уровень IDENT=2, символ `a` доступен
        let b = "2"
    print(b)  # Ошибка: Область действия `b` не распространяется на эту строку
    foo()
```

## Правила для инструкций statement

### Инструкции на уровне INDENT=0 могут включать следующие элементы

- Объявление функции.

**Пример:**

```Udav
let c = 3 # ошибка: на этом уровне можно только объявлять переменные

fun foo(): # может быть объявлена
    pass 

fun main():
    let a = 3 
    fun bar(): # ошибка: язык не поддерживает вложенные функции
        print(a) 
```

#### На очередной строке программы, внутри функции, могут находиться

- Объявление переменной.
- Оператор присваивания.
- Оператор вызова функции.
- Оператор `if`.
- Оператор `while`.
- Оператор `println`|`print`.
- `pass`

**Пример:**

```Udav
fun main():
    let a = 3 
    a = 5
    println(a)
    if a > 0:
        println("Positive")
    while a < 10:
        a = a + 1
    let b = readln()
```

## Правила объявления переменных

### Объявление символов

#### Объявление с инициализацией

В Udav каждый символ должен быть объявлен с начальным значением\
*Если символ не проинициализирован при объявлении, то выполнение завершается ошибкой*

```Udav
let i = 1
let i = "string"
```

#### Множественное объявление

В Udav поддерживается множественное объявление переменных с типами,
определёнными на основе значений. В одной строке можно объявить несколько переменных.

```Udav
let a = 1, b = "string", c = 3
```

### Оператор присваивания

Оператор присваивания используется для назначения значения переменной.
Присваивание может быть как прямым, так и комбинированным (например, присваивание с операцией)

```Udav
i = 2
i = "another string"
```

## Инструкции ввода-вывода

### Вывод в консоль

Для вывода в консоль используются встроенные функции:

```Udav
print(...)
println(...)
```

`...` означает, что на вход подаётся произвольное количество параметров. Обе функции выводят значения, разделяя их
пробелом.

**Примеры:**

```Udav
print()  # Печать пустой строки
print(a, 1, "string", foo())  # Печать нескольких значений
```

### Ввод из консоли

Для ввода данных из консоли используется функция:

```Udav
readln()
```

Функция `readln()` читает данные и возвращает значение типа string.
**Пример:**

```Udav
let a = readln()
```

## Приведения типов

Учитывая, что функция `readln()` всегда возвращает строку, в языке будут встроенные функции для приведения типов

```Udav
let b = "123"
let a = int(b) # тип а - число
let c = string(a)
```

## Правила в формате EBNF

```Udav
program =
    [ newline ] , { function } , EOF ;

function =
    "fun" , SYMBOL , "(" , [ argument_list ] , ")" , ":" , block ;

argument_list =
    SYMBOL , { "," , SYMBOL } ;

block =
    newline , INDENT , { stmt } , DEDENT ;

stmt =
    let_stmt |
    assign_stmt |
    pass_stmt |
    continue_stmt |
    break_stmt |
    return_stmt |
    call_stmt |
    if_stmt |
    while_stmt ;

let_stmt =
    "let" , variable_decl , { "," , variable_decl } , newline ;

variable_decl =
    SYMBOL , "=" , expr ;

assign_stmt =
    SYMBOL , assign_op , expr , newline ;

assign_op =
    "=" |
    "+=" |
    "-=" |
    "*=" |
    "/=" |
    "%=" |
    "**=" |
    "|=" |
    "&=" |
    "^=" |
    ">>=" |
    "<<=" ;

pass_stmt =
    "pass" , newline ;

continue_stmt =
    "continue" , newline ;

break_stmt =
    "break" , newline ;

return_stmt =
    "return" , [ expr ] , newline ;

call_stmt =
    SYMBOL , "(" , [ expr_list ] , ")" , newline ;

if_stmt =
    "if" , expr , ":" , block ,
    { "elif" , expr , ":" , block }
    [ "else" , ":" , block ] ;

while_stmt =
    "while" expr ":" block ;

expr_list =
    expr , { "," , expr } ;
```

## Пример кода

```udav
fun nothing():
    pass
    return

fun all_features(a, b):
    let x = 1, y = 2, z = 3
    let s = "hello", t = "world"
    let flag = true, flag2 = false

    x = 10
    x += 1

    let neg = -x
    let eq = a == b
    let bits = (a | b) ^ (a & b)
    let pow = a ** b ** 2
    let call_expr = max(a, b, x + y)

    if a > b:
        println("a > b")
    elif a == b:
        println("a == b")
    else:
        println("a < b")

    while true && flag:
        bits += 1

        if bits & 1 == 0:
            println("even")
            continue

        if bits > 100:
            println("stop")
            break

        println(bits)
    return bits

fun main():
    let a = 3, b = 5
    let result = all_features(a, b)
    println(result)
```
