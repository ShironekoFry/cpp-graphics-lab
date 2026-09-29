# OpenGL 与 GLUT 基础

## 核心概念总结

```text
OpenGL：负责如何绘制
GLUT：负责窗口和事件

具名命名空间：组织可以跨文件共享的名称
匿名命名空间：隐藏翻译单元内部实现

display：绘制画面
reshape：响应窗口尺寸变化并重建映射关系
keyboard/menuHandler：响应用户输入
initializeOpenGL：设置初始 OpenGL 状态
main：创建环境、注册回调并启动事件循环

glClearColor：设置清屏颜色
glClear：执行清屏
glBegin/glEnd：定义立即模式图元
glVertex：提交顶点
glColor：设置颜色状态
glViewport：指定窗口中的绘制区域
glOrtho：建立正交投影坐标系
glFlush：提交单缓冲绘图命令
glutSwapBuffers：交换双缓冲区
```

## OpenGL 和 GLUT 的职责

OpenGL 和 GLUT 承担不同的任务：

- **OpenGL**：设置绘图状态、坐标变换，并绘制点、线和三角形等图元。
- **GLUT**：创建窗口、接收输入、注册回调函数，并运行事件循环。

它们之间的基本关系是：

```text
GLUT 创建窗口并接收事件
          ↓
GLUT 调用注册的回调函数
          ↓
回调函数使用 OpenGL 完成绘制
```

OpenGL 本身不负责创建操作系统窗口，也不直接管理键盘、鼠标和菜单。GLUT 为这些平台相关操作提供了一层简单接口。

## 跨平台头文件

macOS 可以使用系统提供的 GLUT，Windows 和 Linux 通常使用 FreeGLUT：

```cpp
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
```

- `#ifdef __APPLE__`：判断代码是否正在 macOS 上编译。
- `GL_SILENCE_DEPRECATION`：隐藏 macOS 对旧版 OpenGL API 的弃用警告，不会改变 API 的行为。
- FreeGLUT 是 GLUT 的开源兼容实现。

在 macOS 上手动编译单文件程序时，需要链接 OpenGL 和 GLUT framework：

```bash
clang++ program.cpp -std=c++20 -framework OpenGL -framework GLUT -o program
```

## 程序的常见模块

一个基于 GLUT 的小型 OpenGL 程序通常包含以下模块：

```text
1. 引入标准库和 OpenGL/GLUT 头文件
2. 定义常量、数据类型和程序状态
3. 编写算法或具体图形的绘制函数
4. 编写显示回调 display
5. 编写窗口缩放回调 reshape
6. 编写键盘、鼠标或菜单回调
7. 初始化 OpenGL 状态
8. 在 main 中创建窗口并注册回调
9. 进入 GLUT 事件循环
```

一种常见的文件布局是：

```cpp
#include <cstdlib>

// OpenGL/GLUT 头文件

namespace
{

// 常量和程序状态
// 数据类型
// 算法和绘图函数
// GLUT 回调函数
// OpenGL 初始化函数

}

int main(int argc, char* argv[])
{
    // 初始化 GLUT
    // 创建窗口
    // 初始化 OpenGL
    // 注册回调
    // 进入事件循环
}
```

## C++ 命名空间与文件组织

命名空间在编译阶段为函数、变量和类型分组，用于说明名称的归属并避免同名冲突。

### 具名命名空间

具名命名空间适合组织需要跨文件共享的功能：

```cpp
namespace graphics
{
void drawLine()
{
}
}

int main()
{
    graphics::drawLine();
}
```

`::` 是作用域解析运算符，`graphics::drawLine` 表示 `graphics` 命名空间中的 `drawLine`。

一个文件可以包含多个命名空间，同一个命名空间也可以在一个或多个文件中被重新打开。命名空间还可以嵌套，C++17 可以写成：

```cpp
namespace graphics::rasterization
{
void drawLine()
{
}
}
```

调用时使用：

```cpp
graphics::rasterization::drawLine();
```

### 匿名命名空间

匿名命名空间适合隐藏只在当前翻译单元中使用的实现：

```cpp
namespace
{
constexpr int kWindowWidth = 640;

void display()
{
}
}

int main()
{
    display();
}
```

匿名命名空间通常保存：

- 文件内部的常量和程序状态；
- 辅助函数和数据类型；
- OpenGL 初始化函数；
- GLUT 回调函数。

不同 `.cpp` 文件各自拥有独立的匿名命名空间，因此可以定义同名的内部函数而不发生链接冲突。同一翻译单元中的多个匿名命名空间块则属于同一个匿名命名空间。

### 翻译单元和 `main`

一个翻译单元大致是：

```text
一个 .cpp 文件
+
经过 #include 展开的头文件内容
```

不要在头文件中随意定义匿名命名空间内容，否则每个包含该头文件的翻译单元都会得到一份独立副本。

C++ 要求 `main` 位于全局命名空间，但它可以直接调用同一翻译单元中匿名命名空间里的函数。因此常见结构是：

```text
匿名命名空间：保存文件内部实现
全局命名空间：保存 main
```

### `std` 命名空间

C++ 标准库的名称位于 `std` 命名空间中，例如：

```cpp
std::cout
std::vector
std::max
std::exit
```

一般不建议使用 `using namespace std;`，尤其不要在头文件中使用。明确写出 `std::` 可以表明名称来源，并降低命名冲突的可能。

## 常量、数据类型与程序状态

不会变化的配置通常使用 `constexpr`：

```cpp
constexpr int kWindowWidth = 640;
constexpr int kWindowHeight = 480;
```

`constexpr` 表示该值可以在编译阶段确定，并且运行时不能修改。

需要被多个回调函数共同访问的状态可以保存在文件内部变量中：

```cpp
int gWindowWidth = kWindowWidth;
int gWindowHeight = kWindowHeight;
```

常见命名习惯：

- `kWindowWidth`：`k` 表示常量；
- `gWindowWidth`：`g` 表示具有全局生命周期的状态。

交互式程序可以使用枚举表示有限状态：

```cpp
enum class MenuOption
{
    Line = 1,
    Triangle = 2,
    Square = 3,
    Exit = 4
};

MenuOption gCurrentOption = MenuOption::Line;
```

`enum class` 不会把枚举项名称直接放入外层作用域，使用时需要写：

```cpp
MenuOption::Line
```

## GLUT 初始化和窗口创建

程序入口通常包含以下流程：

```cpp
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Program");

    initializeOpenGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();

    return EXIT_SUCCESS;
}
```

### `glutInit`

```cpp
glutInit(&argc, argv);
```

初始化 GLUT，并读取可能存在的命令行参数。它通常是第一个被调用的 GLUT 函数。

### `glutInitDisplayMode`

```cpp
glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
```

- `GLUT_RGB`：使用 RGB 颜色模式；
- `GLUT_SINGLE`：使用单缓冲。

双缓冲模式可以写成：

```cpp
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
```

### 创建窗口

```cpp
glutInitWindowSize(640, 480);
glutInitWindowPosition(100, 100);
int windowId = glutCreateWindow("OpenGL Program");
```

- `glutInitWindowSize`：设置窗口初始尺寸；
- `glutInitWindowPosition`：设置窗口首次出现的位置；
- `glutCreateWindow`：创建窗口和 OpenGL 上下文，并返回窗口编号。

OpenGL 状态初始化一般放在 `glutCreateWindow` 之后，因为创建窗口时才会建立 OpenGL 上下文。

## 回调函数与事件循环

GLUT 使用事件驱动结构：

```cpp
glutDisplayFunc(display);
glutReshapeFunc(reshape);
glutKeyboardFunc(keyboard);
glutMainLoop();
```

前三个函数不会立即执行传入的回调，而是把函数地址注册到 GLUT。进入 `glutMainLoop` 后，GLUT 持续等待并分发事件：

```text
窗口需要绘制 → display
窗口尺寸变化 → reshape
用户按下键盘 → keyboard
用户选择菜单 → menuHandler
```

`glutMainLoop()` 通常会一直运行，直到程序退出。

## 显示回调 `display`

常见显示回调结构如下：

```cpp
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 设置状态并绘制图形

    glFlush();
}
```

显示回调通常完成四件事：

1. 清除上一帧；
2. 重置模型视图矩阵；
3. 绘制画面；
4. 提交绘图命令或交换缓冲区。

## OpenGL 状态与清屏

OpenGL 是状态机。颜色、点大小、线宽和当前矩阵等设置会持续生效，直到被新的调用修改。

### 设置清屏颜色

```cpp
glClearColor(red, green, blue, alpha);
```

例如：

```cpp
glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
```

`glClearColor` 只设置之后清屏时使用的颜色，不会立即清除窗口。

### 清除颜色缓冲区

```cpp
glClear(GL_COLOR_BUFFER_BIT);
```

`glClear` 使用之前设置的清屏颜色清除颜色缓冲区。典型关系是：

```cpp
// 初始化阶段设置状态
glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

// 每次绘制时执行清屏
glClear(GL_COLOR_BUFFER_BIT);
```

## 立即模式绘制

旧版 OpenGL 可以在 `glBegin` 和 `glEnd` 之间逐个提交顶点：

```cpp
glBegin(GL_POINTS);
glVertex2f(0.0f, 0.0f);
glEnd();
```

### 常用绘制模式

| 模式 | 顶点的组织方式 |
|---|---|
| `GL_POINTS` | 每个顶点绘制成一个独立点 |
| `GL_LINES` | 每两个顶点组成一条独立线段 |
| `GL_LINE_STRIP` | 按顺序连接顶点，不自动闭合 |
| `GL_LINE_LOOP` | 按顺序连接顶点，并自动闭合 |
| `GL_TRIANGLES` | 每三个顶点组成一个独立三角形 |

### 提交顶点

```cpp
glVertex2i(x, y);
glVertex2f(x, y);
```

- `glVertex2i`：提交二维整数坐标；
- `glVertex2f`：提交二维浮点坐标。

整数坐标适合表达离散像素位置，浮点坐标适合表达与具体窗口分辨率无关的逻辑位置。

### 设置颜色

```cpp
glColor3f(red, green, blue);
```

三个参数通常位于 `[0, 1]` 范围：

```cpp
glColor3f(1.0f, 0.0f, 0.0f);  // 红色
glColor3f(0.0f, 1.0f, 0.0f);  // 绿色
glColor3f(0.0f, 0.0f, 1.0f);  // 蓝色
glColor3f(1.0f, 1.0f, 1.0f);  // 白色
```

如果图元的顶点使用不同颜色，并启用平滑着色：

```cpp
glShadeModel(GL_SMOOTH);
```

OpenGL 会在顶点颜色之间进行插值。

### 设置点和线的大小

```cpp
glPointSize(2.0f);
glLineWidth(2.0f);
```

- `glPointSize`：设置后续点的显示大小；
- `glLineWidth`：设置后续线段的宽度。

不同 OpenGL 实现支持的点大小和线宽范围可能不同，尤其是大于 `1.0f` 的线宽不一定具有一致的跨平台表现。

### 绘制矩形

```cpp
glRectf(left, bottom, right, top);
```

`glRectf` 使用两个对角顶点绘制轴对齐的填充矩形。

## 单缓冲与双缓冲

单缓冲模式直接在当前显示缓冲区中绘制：

```cpp
glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
```

绘制末尾通常调用：

```cpp
glFlush();
```

`glFlush` 要求 OpenGL 在有限时间内开始处理此前提交的命令，但不会等待所有命令完全执行结束。

双缓冲模式使用前缓冲区显示画面，在后缓冲区完成下一帧：

```cpp
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
```

绘制完成后交换前后缓冲区：

```cpp
glutSwapBuffers();
```

连续动画通常使用双缓冲，以减少闪烁和未完成画面被显示的问题。

## 窗口缩放与坐标系

窗口尺寸变化时，GLUT 调用缩放回调：

```cpp
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 设置投影范围

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
```

将零高度修正为 `1`，可以避免后续计算宽高比时发生除零。

### `glViewport`

```cpp
glViewport(0, 0, width, height);
```

指定绘制结果映射到窗口中的区域。四个参数依次表示：

```text
左下角 x、左下角 y、区域宽度、区域高度
```

### 投影矩阵和模型视图矩阵

```cpp
glMatrixMode(GL_PROJECTION);
```

选择投影矩阵，用于确定可见空间和坐标范围。

```cpp
glMatrixMode(GL_MODELVIEW);
```

选择模型视图矩阵，用于控制模型以及观察相关的平移、旋转和缩放。

```cpp
glLoadIdentity();
```

把当前选中的矩阵重置为单位矩阵，清除之前累积的变换。

### `glOrtho`

```cpp
glOrtho(left, right, bottom, top, near, far);
```

`glOrtho` 建立正交投影坐标系，不会产生近大远小的透视效果。

与窗口像素对应的二维坐标可以写成：

```cpp
glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
```

此时左下角是 `(0, 0)`，右上角是 `(width, height)`。

如果希望使用以原点为中心的逻辑坐标并保持图形比例，可以根据宽高比调整投影范围：

```cpp
const double aspect = static_cast<double>(width) / height;

if (aspect >= 1.0)
{
    glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
}
else
{
    glOrtho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
}
```

这样可以避免窗口比例改变后，正方形被拉伸成长方形。

## 键盘回调

普通键盘回调的函数签名是：

```cpp
void keyboard(unsigned char key, int x, int y)
```

如果不使用鼠标坐标，可以省略参数名称：

```cpp
void keyboard(unsigned char key, int, int)
```

按 Esc 退出程序：

```cpp
constexpr unsigned char kEscapeKey = 27;

if (key == kEscapeKey)
{
    std::exit(EXIT_SUCCESS);
}
```

注册键盘回调：

```cpp
glutKeyboardFunc(keyboard);
```

## GLUT 右键菜单

创建菜单并注册菜单选择回调：

```cpp
glutCreateMenu(menuHandler);
```

添加菜单项：

```cpp
glutAddMenuEntry("Line", 1);
glutAddMenuEntry("Triangle", 2);
```

把菜单绑定到鼠标右键：

```cpp
glutAttachMenu(GLUT_RIGHT_BUTTON);
```

用户选择菜单项后，GLUT 把对应的整数值传给回调：

```cpp
void menuHandler(int option)
{
    // 根据 option 更新状态
}
```

修改状态后可以请求重新绘制：

```cpp
glutPostRedisplay();
```

`glutPostRedisplay` 不会直接调用 `display`，而是把窗口标记为需要重绘，让 GLUT 在后续事件循环中安排调用。

如果程序包含多个窗口，可以先指定当前窗口：

```cpp
glutSetWindow(windowId);
```

## 算法、场景与显示分离

计算逻辑与 OpenGL 显示代码应尽可能分离：

```text
算法或场景构建
      ↓
生成顶点或像素数据
      ↓
OpenGL 绘制函数
      ↓
显示到窗口
```

例如：

```cpp
std::vector<Pixel> rasterizeLine(...);
void drawPixels(const std::vector<Pixel>& pixels);
```

这种分离有几个优点：

- 算法不依赖窗口系统；
- 算法结果可以单独测试；
- 同一份数据可以输出到 OpenGL、图像文件或其他界面；
- 绘图代码不需要了解算法内部的判断过程。

## 完整执行顺序

```text
main
│
├── 初始化 GLUT
├── 设置显示模式
├── 创建窗口和 OpenGL 上下文
├── 初始化 OpenGL 状态
├── 创建菜单等交互组件
├── 注册 display、reshape、keyboard 等回调
└── 进入 glutMainLoop
        │
        ├── 窗口需要绘制 → display
        ├── 窗口尺寸变化 → reshape
        ├── 用户按键 → keyboard
        └── 用户选择菜单 → menuHandler
```

## 立即模式与现代 OpenGL

`glBegin`、`glVertex` 和 `glEnd` 属于旧版 OpenGL 的立即模式。这种模式能够直观展示顶点如何组成图元，但在现代 OpenGL 核心模式中已经被移除。

现代 OpenGL 通常使用：

- VAO：保存顶点输入配置；
- VBO：把顶点数据存储在 GPU 缓冲区；
- Shader：通过可编程着色器处理顶点和片元；
- `glDrawArrays` 或 `glDrawElements`：发起批量绘制。

立即模式适合用于理解基本图元、状态机、坐标系和事件驱动结构；需要更高性能或可编程渲染流程时，应使用现代 OpenGL 的缓冲区与着色器方式。
