# Грамматика выражений языка Udav

## О чём здесь написано

1) Синтаксис выражений
   - арифметические выражения
   - выражения со строками
   - булевые выражения
2) Операторы
3) Приоритет операторов
4) Грамматика в формате antlr4

## Синтаксис выражений

### Арифметические выражения

Арифметические выражения включают базовые математические операции: сложение, вычитание, умножение, деление, степень.

```
d = a + b - c * 4 - (2 + 5) / 3
let a = b ** 2
```

### Выражения со строками

Строки в Udav могут быть использованы в выражениях, включая конкатенацию с помощью оператора `+`.

```
let greeting = "Hello" + " " + "World"
```

### Булевые выражения

Булевые выражения поддерживают операторы сравнения и логические операторы (AND, OR, NOT).

```
let flag = true
flag = true || false && true || !false
```

## Операторы

### Арифметические операторы

| Оператор | Описание           |
|:---------|:-------------------|
| `+`      | Сложение           |
| `-`      | Вычитание          |
| `*`      | Умножение          |
| `/`      | Деление            |
| `%`      | Остаток от деления |
| `**`     | Степень            |

### Логические операторы

| Оператор | Описание            |
|:---------|:--------------------|
| `&&`     | Логическое И (AND)  |
| `\|\| `  | Логическое ИЛИ (OR) |
| `!`      | Логическое НЕ (NOT) |

### Побитовые операторы

| Оператор | Описание           |
|:---------|:-------------------|
| `&`      | Побитовое И (AND)  |
| `\|`     | Побитовое ИЛИ (OR) |
| `^`      | Побитовый XOR      |
| `<<`     | Сдвиг влево        |
| `>>`     | Сдвиг вправо       |

## Приоритет операторов

| Оператор                         | Приоритет |
|:---------------------------------|:----------|
| `**`                             | 1         |
| `*`, `/`, `%`                    | 2         |
| `+`, `-`                         | 3         |
| `<<`, `>>`                       | 4         |
| `&`                              | 5         |
| `^`                              | 6         |
| `\| `                            | 7         |
| `==`, `!=`, `<`, `<=`, `>`, `>=` | 8         |
| `&&`                             | 9         |
| `\|\|`                           | 10        |
| `!`                              | 11        |
| `=`                              | 12        |
| `+=`, `-=`, `*=`, `/=`, `%=`     | 13        |

## Грамматика в формате ANTLR4

```ebnf
expr =
    or_expr ;

or_expr =
    and_expr |
    or_expr , "||" , and_expr ;

and_expr =
    relational_expr |
    and_expr , "&&" , relational_expr ;

relational_expr =
    bitwise_or_expr |
    relational_expr , relational_op , bitwise_or_expr ;

relational_op = "==" | "!=" | ">=" | "<=" | ">" | "<" ;

bitwise_or_expr =
    bitwise_xor_expr |
    bitwise_or_expr , "|" , bitwise_xor_expr ;

bitwise_xor_expr =
    bitwise_and_expr |
    bitwise_xor_expr , "^" , bitwise_and_expr ;

bitwise_and_expr =
    shift_expr |
    bitwise_and_expr , "&" , shift_expr ;

shift_expr =
    add_expr |
    shift_expr , shift_op , add_expr ;

shift_op = "<<" | ">>" ;

add_expr =
    mul_expr |
    add_expr , add_op , mul_expr ;

add_op = "+" | "-" ;

mul_expr =
    unary_expr |
    mul_expr , mul_op , unary_expr ;

mul_op = "*" | "/" | "%" ;

unary_expr =
    power_expr |
    unary_op , power_expr ;

unary_op = "!" | "-" ;

power_expr =
    postfix_expr,
    | postfix_expr "**" power_expr ; (* Правая рекурсия для правой ассоциативности *)

postfix_expr =
    primary_expr |
    primary_expr , "(" , [ expr_list ] , ")" ;

primary_expr =
    literal |
    SYMBOL |
    "(" , expr , ")" ;

literal =
    INTEGER_LITERAL |
    STRING_LITERAL |
    "true" |
    "false" ;

newline =
    NEWLINE , { NEWLINE } ;
```
