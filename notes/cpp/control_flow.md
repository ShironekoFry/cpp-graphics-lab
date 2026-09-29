# 流程控制

## 简化布尔值返回

当条件表达式本身就是布尔值时，不需要写成：

```cpp
if (condition)
    return true;
else
    return false;
```

可以直接返回条件表达式：

```cpp
return condition;
```

例如，判断一个数字是否为个位数质数：

```cpp
bool isPrime(int number)
{
    return number == 2 || number == 3 || number == 5 || number == 7;
}
```
