# Day 03

## If Statement

You never need an if-statement of the form:

```cpp
if (condition)
  return true;
else
  return false;
```

This can be replaced by the single statement return condition:

```cpp
return condition;
```

## Input Char

`std::cin` will normally ignore the whitespace, but `std::cin.get()` can extract it.

## Escape Sequences

Here’s a table of all of the escape sequences:

| Name | Symbol | Meaning |
|---|---|---|
| Alert | `\a` | Makes an alert, such as a beep |
| Backspace | `\b` | Moves the cursor back one space |
| Formfeed | `\f` | Moves the cursor to next logical page |
| Newline | `\n` | Moves cursor to next line |
| Carriage return | `\r` | Moves cursor to beginning of line |
| Horizontal tab | `\t` | Prints a horizontal tab |
| Vertical tab | `\v` | Prints a vertical tab |
| Single quote | `\'` | Prints a single quote |
| Double quote | `\"` | Prints a double quote |
| Backslash | `\\` | Prints a backslash |
| Question mark | `\?` | Prints a question mark. No longer relevant; you can use question marks unescaped. |
| Octal number | `\(number)` | Translates into char represented by octal |
| Hex number | `\x(number)` | Translates into char represented by hex number |

## Type conversion of a value produces a new value

The type conversion process does not modify the value (or object) supplying the data to be converted. Instead, the conversion process uses that data as input, and produces the converted result.

In the above example, the conversion does not change variable y from type int to double or the value of y from 5 to 5.0. Instead, the conversion uses the value of y (5) as input, and returns a temporary object of type double with value 5.0. This temporary object is then passed to function print.

## Explicit Type Conversion

```cpp
static_cast<new_type>(expression)
```

`static_cast` can also convert char to int, and convert signed int to unsigned int.