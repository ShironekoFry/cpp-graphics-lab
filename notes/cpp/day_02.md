# Day 02

## 输出格式控制

`std::left` 和 `std::setw` 都用于控制输出格式，定义在 `<iomanip>` 头文件中，属于 C++ 标准库。

```cpp
#include <iostream>
#include <iomanip>
```

- `std::left` 把后续内容设置为左对齐。
- `std::right` 把后续内容设置为右对齐。
- `std::setw(宽度)` 为紧接着输出的下一项指定字段宽度。

```cpp
std::cout << std::left << std::setw(10) << "Apple" << 100 << '\n';
std::cout << std::left << std::setw(10) << "Banana" << 200 << '\n';
```

输出：

```text
Apple     100
Banana    200
```

`std::left` 会持续生效，直到使用 `std::right` 等方式改变对齐规则；`std::setw(10)` 通常只对紧接着的下一项输出生效。

```cpp
std::cout << std::right << std::setw(10) << "Apple" << '\n';
```

常见坑：

- 正确名称是 `std::setw`，不是 `std::stew`。
- 只写 `std::left` 而没有设置字段宽度，通常看不出对齐效果。
- `std::setw` 需要为每个需要固定宽度的输出项重新设置。
- 字符串长度超过指定宽度时不会被截断，而是按原长度输出。

速记：

```cpp
std::left          // 后续输出左对齐，持续生效
std::right         // 后续输出右对齐，持续生效
std::setw(10)      // 下一项宽度为 10，使用一次后失效
```
