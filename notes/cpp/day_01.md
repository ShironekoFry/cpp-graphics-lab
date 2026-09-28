# Day 01

## 标准输入输出

```cpp
#include <iostream>

int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}
```

- `std` 是 **standard** 的缩写，是 C++ 标准库使用的namespace（命名空间）。命名空间用于组织名称、避免命名冲突。
- `::` 是作用域解析运算符。`std::cout` 表示“`std` 命名空间中的 `cout`”。
- `cout` 是标准输出流对象，通常用于向终端输出内容。
- `<<` 是流插入运算符，把右侧内容送入左侧的输出流。
- `endl` 是 **end line** 的缩写，用于换行，并刷新（flush）输出缓冲区。普通换行通常可以写成 `\n`。

```cpp
std::cout << "Age: " << 20 << '\n';
```

标准输入与标准输出相对应：

```cpp
int age;
std::cin >> age;
std::cout << age << '\n';
```

- `cin` 是标准输入流对象，通常用于读取终端输入。
- `>>` 是流提取运算符，把输入流中的数据读入右侧变量。

速记：

```cpp
std::cout << 数据;  // 输出
std::cin  >> 变量;  // 输入
```
