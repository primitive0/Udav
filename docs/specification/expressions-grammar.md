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

```antlr4
expression
    : or_expression
    ;

or_expression
    : and_expression ('||' and_expression)*
    ;

and_expression
    : bitwise_expression ('&&' bitwise_expression)*
    ;

bitwise_expression
    : equality_expression
    | bitwise_expression ('&' | '|' | '^') equality_expression
    ;

equality_expression
    : relational_expression
    | equality_expression ('==' | '!=') relational_expression
    ;

relational_expression
    : additive_expression
    | relational_expression ('<' | '>' | '<=' | '>=') additive_expression
    ;

additive_expression
    : multiplicative_expression
    | additive_expression ('+' | '-') multiplicative_expression
    ;

multiplicative_expression
    : power_expression
    | multiplicative_expression ('*' | '/' | '%') power_expression
    ;

power_expression
    : primary_expression
    | power_expression '**' primary_expression
    ;

primary_expression
    : IDENTIFIER
    | NUMBER
    | STRING
    | 'true' | 'false'
    | '(' expression ')'
    ;
```
