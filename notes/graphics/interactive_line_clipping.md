# Interactive Line Clipping Program
# 交互式直线段裁剪程序

## 1. 项目简介 / Project Overview

本程序是一个用于演示直线段裁剪算法的交互式小程序。程序首先绘制一个由 360 条线段组成的放射状图形，每条线段从画面中心出发，相邻线段之间相差 1 度。

用户可以选择不同的裁剪区域，并在裁剪前和裁剪后的画面之间切换。程序支持三种矩形裁剪算法，也加入了裁剪区域的平移、缩放、旋转以及三角形和圆形裁剪等扩展功能。

This program is an interactive demonstration of line clipping. It first draws 360 radial line segments. Every segment starts from the center of the window, and the angle between two neighboring segments is one degree.

The user can select different clipping regions and switch between the original view and the clipped view. The program supports three rectangle clipping algorithms. It also includes several extended functions, such as moving, scaling and rotating the clipping region, as well as triangle and circle clipping.

## 2. 功能列表 / Features

### 2.1 基本功能 / Basic Features

- 绘制由 360 条线段组成的初始图形。
- 使用固定矩形作为默认裁剪区域。
- 支持 Cohen–Sutherland 裁剪算法。
- 支持中点分割裁剪算法。
- 支持 Liang–Barsky 裁剪算法。
- 使用鼠标重新框选裁剪区域。
- 在裁剪前和裁剪后的画面之间切换。
- 使用界面按钮切换裁剪算法。

- Draw an initial image containing 360 radial line segments.
- Use a rectangle as the default clipping region.
- Support the Cohen–Sutherland algorithm.
- Support the Midpoint Subdivision algorithm.
- Support the Liang–Barsky algorithm.
- Select a new clipping region with the mouse.
- Switch between the before-clipping and after-clipping views.
- Select the clipping algorithm through interface buttons.

### 2.2 扩展功能 / Extended Features

- 使用鼠标拖动裁剪区域。
- 使用鼠标滚轮或键盘缩放裁剪区域。
- 使用鼠标右键或键盘旋转裁剪区域。
- 支持矩形、三角形和圆形三种裁剪区域。
- 使用无文字的图形按钮切换裁剪区域。
- 在裁剪后的画面中移动、缩放或旋转裁剪区域时，实时重新计算裁剪结果。

- Move the clipping region by dragging it with the mouse.
- Scale the clipping region with the mouse wheel or keyboard.
- Rotate the clipping region with the right mouse button or keyboard.
- Support rectangle, triangle and circle clipping regions.
- Select the clipping shape with icon buttons.
- Recalculate the clipping result in real time when the region is moved, scaled or rotated.

## 3. 编译和运行 / Build and Run

### 3.1 开发环境 / Development Environment

本程序使用 C++ 编写，并使用 OpenGL 和 GLUT 完成图形绘制及鼠标、键盘事件处理。

The program is written in C++. OpenGL and GLUT are used for rendering and input event handling.

### 3.2 macOS 编译方法 / Build on macOS

```bash
cd "/Users/xieyuwei/05_Repo/cpp_graphics_lab"
mkdir -p build

clang++ exercises/graphics/interactive_line_clipping.cpp \
  -std=c++20 -Wall -Wextra -pedantic \
  -framework OpenGL -framework GLUT \
  -o build/interactive_line_clipping
```

运行程序：

```bash
./build/interactive_line_clipping
```

## 4. 使用指南 / User Guide

### 4.1 界面按钮 / Interface Buttons

界面上方左侧是三个矩形裁剪算法按钮：

- Cohen–Sutherland
- Midpoint Subdivision
- Liang–Barsky

界面上方右侧是三个形状按钮：

- 正方形图标：矩形裁剪区域
- 三角形图标：三角形裁剪区域
- 圆形图标：圆形裁剪区域

绿色按钮表示当前选中的项目。

The three buttons on the upper-left side are used to select the rectangle clipping algorithm. The three icon buttons on the upper-right side are used to select the clipping shape. The selected button is displayed in green.

三角形和圆形有各自的裁剪方法，因此选择它们时，三个矩形专用算法按钮会暂时变灰。

Triangle and circle regions use their own clipping methods. Therefore, the rectangle algorithm buttons are disabled when one of these shapes is selected.

### 4.2 鼠标操作 / Mouse Controls

| 操作 / Input | 功能 / Function |
| --- | --- |
| 在裁剪区域外按住左键拖动 | 重新框选裁剪区域，仅在裁剪前状态可用 / Select a new region in the before-clipping view |
| 在裁剪区域内按住左键拖动 | 平移裁剪区域 / Move the clipping region |
| 鼠标滚轮 | 缩放裁剪区域 / Scale the clipping region |
| 在裁剪区域内按住右键拖动 | 旋转裁剪区域 / Rotate the clipping region |
| 单击算法按钮 | 切换矩形裁剪算法 / Select a rectangle clipping algorithm |
| 单击形状按钮 | 切换裁剪区域形状 / Select a clipping shape |

### 4.3 键盘操作 / Keyboard Controls

| 按键 / Key | 功能 / Function |
| --- | --- |
| `C` | 选择 Cohen–Sutherland 算法 |
| `M` | 选择中点分割算法 |
| `L` | 选择 Liang–Barsky 算法 |
| `Space` | 切换裁剪前和裁剪后画面 |
| `+` / `-` | 放大或缩小裁剪区域 |
| `Q` / `E` | 逆时针或顺时针旋转裁剪区域 |
| `0` | 将旋转角度恢复为 0 |
| `R` | 重置整个程序 |
| `Esc` | 退出程序 |

The `C`, `M` and `L` keys select the three rectangle clipping algorithms. The space key switches the view. The other keys are used for scaling, rotation, reset and exit.

## 5. 程序的整体实现方法 / General Implementation

程序主要保存两类数据：原始线段和裁剪区域。

每条线段由起点和终点组成。裁剪区域保存中心位置、半宽、半高以及旋转角度。程序不会直接修改原始的 360 条线段，而是在每次绘制画面时，根据当前裁剪区域重新计算需要显示的部分。

The program mainly stores two types of data: the original line segments and the clipping region.

Each segment contains a start point and an end point. The clipping region stores its center, half-width, half-height and rotation angle. The original 360 segments are not modified. The visible parts are recalculated whenever the window is redrawn.

程序每一帧的基本流程如下：

1. 清空上一帧画面。
2. 判断当前是裁剪前还是裁剪后状态。
3. 如果是裁剪前，绘制全部原始线段。
4. 如果是裁剪后，逐条裁剪原始线段，再绘制可见部分。
5. 绘制裁剪区域边框。
6. 绘制算法按钮、形状按钮和状态文字。

The basic rendering process is:

1. Clear the previous frame.
2. Check the current view mode.
3. Draw all original segments in the before-clipping view.
4. Clip every segment and draw its visible part in the after-clipping view.
5. Draw the clipping boundary.
6. Draw the buttons and status information.

## 6. 三种矩形裁剪算法 / Rectangle Clipping Algorithms

### 6.1 Cohen–Sutherland 算法

Cohen–Sutherland 算法把矩形周围的平面划分成九个区域，并为线段的两个端点计算区域码。

如果两个端点的区域码都是 0，说明整条线段都在矩形内部，可以直接保留。如果两个区域码按位与的结果不为 0，说明两个端点位于同一条边界的外侧，可以直接舍弃。其他情况需要计算线段与矩形边界的交点，然后继续判断。

The Cohen–Sutherland algorithm assigns an outcode to each endpoint. It can quickly accept a segment inside the rectangle or reject a segment outside the same boundary. Other segments are shortened by calculating boundary intersections.

### 6.2 中点分割算法 / Midpoint Subdivision

中点分割算法通过不断计算线段中点来寻找裁剪边界。

当一个端点位于窗口内、另一个端点位于窗口外时，程序重复把线段分成两部分，并保留靠近边界的部分。当线段长度小于设定的误差范围时，就把得到的点看作边界交点。

The Midpoint Subdivision algorithm repeatedly divides a segment into two parts. It searches for the boundary point by keeping the part between an inside point and an outside point until the remaining interval is small enough.

### 6.3 Liang–Barsky 算法

Liang–Barsky 算法把线段表示为参数方程：

\[
P(t)=P_0+t(P_1-P_0),\quad 0\leq t\leq 1
\]

矩形的四条边分别给出参数 \(t\) 的限制条件。程序逐次更新可见参数区间的下界和上界。如果最后的下界小于等于上界，就说明线段存在可见部分。

The Liang–Barsky algorithm represents a segment with a parameter \(t\). Each rectangle boundary gives a restriction on the valid parameter interval. If the final lower bound is not greater than the upper bound, the segment has a visible part.

## 7. 平移、缩放和旋转 / Translation, Scaling and Rotation

### 7.1 平移 / Translation

鼠标按下时，程序记录鼠标位置和裁剪区域原来的中心位置。拖动过程中，根据鼠标的位移更新裁剪区域中心，因此不会改变区域的大小和角度。

When dragging starts, the program records the original mouse position and region center. The region center is updated using the mouse displacement, while its size and angle remain unchanged.

### 7.2 缩放 / Scaling

缩放时，程序同时按照相同的比例修改半宽和半高，因此裁剪区域原来的宽高比例不会改变。同时设置最小尺寸，避免裁剪区域缩小到无法继续操作。

Scaling changes the half-width and half-height by the same factor. A minimum size is used to prevent the region from becoming too small.

### 7.3 旋转和局部坐标 / Rotation and Local Coordinates

老师给出的三种算法处理的是与坐标轴平行的矩形。为了让它们支持旋转矩形，程序没有直接修改三种算法，而是加入了局部坐标变换。

裁剪前，先把世界坐标中的线段平移到裁剪区域中心，再旋转相反的角度。这样，旋转矩形在局部坐标系中就重新变成了轴对齐矩形。裁剪完成后，再把结果旋转和平移回世界坐标系。

The three rectangle algorithms work with an axis-aligned rectangle. To support rotation, the segment is first transformed into the local coordinate system of the clipping region. The rectangle then becomes axis-aligned again. After clipping, the result is transformed back to world coordinates.

这种方法保留了原来的矩形裁剪算法，也把旋转功能与具体算法分开。

This method keeps the original clipping algorithms unchanged and separates rotation from the clipping process.

## 8. 扩展形状的实现 / Extended Clipping Shapes

### 8.1 三角形裁剪 / Triangle Clipping

三角形是凸多边形。对于逆时针排列的三角形，每条有向边的左侧都可以看作三角形内部。

程序把线段写成参数形式，然后依次检查三角形的三条边。每条边都会给参数 \(t\) 增加一个限制。三条边处理完成后，剩下的参数区间就是线段位于三角形内部的部分。

A triangle is a convex polygon. For a counterclockwise triangle, the inside region is located on the left side of every directed edge.

The program checks the three edge half-planes and gradually reduces the valid parameter interval of the segment. The final interval represents the part inside the triangle.

这种方法不需要把三角形分成多个矩形，也能够继续支持平移、缩放和旋转。

This method does not divide the triangle into smaller rectangles. It also works with translation, scaling and rotation.

### 8.2 圆形裁剪 / Circle Clipping

圆形是凸区域，但严格来说不是多边形，因此没有使用三角形的逐边判断方法。

程序把线段参数方程代入圆方程：

\[
x^2+y^2=r^2
\]

代入后可以得到关于参数 \(t\) 的二次方程。根据判别式可以判断线段与圆的关系：

- 判别式小于 0：直线与圆没有交点。
- 判别式等于 0：直线与圆相切。
- 判别式大于 0：直线与圆有两个交点。

程序再把求出的两个参数与线段自身的参数范围 \([0,1]\) 相交，得到最终可见部分。

A circle is a convex region, but it is not a polygon. Therefore, circle clipping uses the exact intersection between the parametric segment and the circle equation.

The resulting quadratic equation determines whether the line misses, touches or crosses the circle. The intersection parameters are then limited to the segment range \([0,1]\).

界面中的圆周使用较多的小线段进行绘制，使外观更加平滑，但实际裁剪使用的是圆方程，并不是使用这些小线段近似计算。

The displayed circle is drawn with many short line segments for a smooth appearance. However, the clipping calculation uses the exact circle equation.

### 8.3 形状切换 / Shape Selection

三个形状按钮保存各自的屏幕范围。用户单击按钮时，程序判断鼠标是否位于按钮范围内，并更新当前裁剪形状。

矩形、三角形和圆形共用同一个中心、尺寸和旋转角数据，因此切换形状时不需要重新创建裁剪区域。

The shape buttons use simple rectangular hit areas. When a button is clicked, the current shape is changed. All three shapes share the same center, size and rotation data.

## 9. 交互状态管理 / Interaction State Management

为了区分不同的鼠标操作，程序设置了四种状态：

- `Idle`：当前没有鼠标操作。
- `DrawingWindow`：正在重新框选裁剪区域。
- `DraggingWindow`：正在平移裁剪区域。
- `RotatingWindow`：正在旋转裁剪区域。

鼠标按下时确定操作状态，鼠标移动时根据当前状态更新裁剪区域，鼠标松开后返回 `Idle` 状态。

The program uses four mouse states: idle, drawing, dragging and rotating. The state is selected when a mouse button is pressed, used during mouse movement, and reset when the button is released.

在裁剪后状态中，用户不能在空白位置重新框选区域，但是仍然可以移动、缩放和旋转已有区域。这样可以避免误操作，同时也方便观察裁剪结果的变化。

In the after-clipping view, the user cannot create a new region in an empty area, but the existing region can still be moved, scaled and rotated.

## 10. 总结 / Conclusion

本程序完成了三种矩形直线段裁剪算法的显示和切换，并加入了鼠标框选、裁剪前后切换、平移、缩放和旋转功能。在此基础上，程序进一步实现了三角形和圆形裁剪区域。

This program implements and compares three rectangle line clipping algorithms. It also adds interactive selection, view switching, translation, scaling, rotation, triangle clipping and circle clipping.

## 11. AI 使用说明 / AI Usage Statement

本程序的源代码是在生成式 AI 的辅助下完成的。AI 主要用于设计程序结构、检查代码问题以及撰写技术文档。矩形裁剪算法的基本思路以课程课件和老师提供的示例代码为依据。

对于 AI 提供的代码，我进行了阅读、修改、编译和运行验证。我本人理解程序中各项功能的实现原理，包括三种矩形裁剪算法、局部坐标变换、平移、缩放、旋转、三角形裁剪、圆形裁剪以及鼠标交互状态管理，并能够对这些内容进行说明。

The source code of this program was completed with the assistance of generative AI. AI was mainly used to help design the program structure, check code problems and write the technical article. The basic ideas of the rectangle clipping algorithms were based on the course slides and the example code provided by the instructor.

I read, modified, compiled and tested the code provided by AI. I understand the principles behind all major functions of the program, including the three rectangle clipping algorithms, local coordinate transformations, translation, scaling, rotation, triangle clipping, circle clipping and mouse interaction state management. I am also able to explain how these functions work.

## 12. 参考资料 / References

- 计算机图形学课程课件。
- 老师提供的直线段裁剪算法示例代码。

- Computer Graphics course slides.
- Line clipping example code provided by the instructor.
