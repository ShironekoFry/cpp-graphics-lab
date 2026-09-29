#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

// macOS使用系统自带的GLUT，Windows和Linux使用FreeGLUT
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

namespace
{

// 配置窗口初始尺寸和直线间的角度间隔，都是常量
// 初始尺寸会在程序启动时使用，代表窗口刚创建时，默认用多大尺寸
// k_ 是常见的常量命名方式
constexpr int kInitialWindowWidth = 640;
constexpr int kInitialWindowHeight = 480;
constexpr int kSpokeAngleStep = 5;  //角度的步长设置为5度
constexpr double kPi = 3.14159265358979323846;

// 窗口当前尺寸，会随用户拖动发生变化，用于计算图形中心
int gWindowWidth = kInitialWindowWidth;
int gWindowHeight = kInitialWindowHeight;

// 定义像素数据
struct Pixel
{
    int x;
    int y;
};

// 通用DDA算法：生成从(x0, y0)到(x1, y1)的全部像素，包括两个端点。
// 总步数由坐标变化量较大的方向决定：
// 当|dx| >= |dy|时，x每步增加1或减少1；
// 当|dy| > |dx|时，y每步增加1或减少1。
// 这个统一公式可以处理水平线、垂直线、对角线、正负斜率和反向端点。
std::vector<Pixel> rasterizeLineDDA(
    int x0,
    int y0,
    int x1,
    int y1)
{
    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int steps = std::max(std::abs(dx), std::abs(dy)); // std::abs取绝对值，std::max选出主位移方向的总步数。

    // 起点和终点重合时，线段只包含一个像素。
    if (steps == 0)
    {
        return {{x0, y0}};
    }

    const double xIncrement = static_cast<double>(dx) / steps; // 计算每一步的步长
    const double yIncrement = static_cast<double>(dy) / steps; // 转换成double

    double x = static_cast<double>(x0);
    double y = static_cast<double>(y0);

    std::vector<Pixel> pixels; // 这里在做的是声明一个类型为Pixel的变量pixels
    pixels.reserve(static_cast<std::size_t>(steps + 1)); // reserve是pixels对象的成员函数，预先分配空间，避免添加像素时反复扩容

    for (int i = 0; i <= steps; ++i)
    {
        pixels.push_back({
            static_cast<int>(std::lround(x)), // std::lround将浮点坐标四舍五入为最近整数
            static_cast<int>(std::lround(y))
        }); // push_back把当前像素添加到结果序列末尾

        x += xIncrement;
        y += yIncrement;
    }

    return pixels;
}

// 用OpenGL绘制DDA生成的像素
void drawPixels(const std::vector<Pixel>& pixels)
{
    glBegin(GL_POINTS); // glBegin指定接下来的顶点按照独立像素点进行绘制

    for (const Pixel& pixel : pixels)
    {
        glVertex2i(pixel.x, pixel.y); // glVertex2i提交一个二维整数坐标顶点
    }

    glEnd(); // glEnd结束本次顶点提交
}

void drawLineDDA(int x0, int y0, int x1, int y1)
{
    drawPixels(rasterizeLineDDA(x0, y0, x1, y1));
}

// 显示回调：绘制覆盖全部象限以及不同正负斜率的放射状测试图
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); // glClear使用当前清屏颜色（前面设置的glClearColor）清除上一帧画面

    glMatrixMode(GL_MODELVIEW); // glMatrixMode选择接下来要操作的模型视图矩阵
    glLoadIdentity(); // glLoadIdentity把当前矩阵重置为单位矩阵，表示不进行任何变换
    // 这两行代码表示：选择模型视图矩阵，并清除之前可能存在的平移、旋转和缩放，这样后面的像素坐标就不会受到上一次绘制状态的影响

    const int centerX = gWindowWidth / 2;
    const int centerY = gWindowHeight / 2;
    const double radius = 0.40 * std::min(gWindowWidth, gWindowHeight); // 计算半径，std::min选较短边，避免图形超出窗口

    glPointSize(1.0f); // glPointSize设置每个光栅像素点的显示大小
    glColor3f(0.0f, 0.0f, 0.0f); // glColor3f使用RGB浮点值设置当前绘图颜色

    for (int degrees = 0; degrees < 360; degrees += kSpokeAngleStep)
    {
        const double radians = degrees * kPi / 180.0;

        const int endX = centerX + static_cast<int>(
            std::lround(radius * std::cos(radians))); // std::cos计算终点的水平分量
        const int endY = centerY + static_cast<int>(
            std::lround(radius * std::sin(radians))); // std::sin计算终点的垂直分量

        drawLineDDA(centerX, centerY, endX, endY);
    }

    glFlush(); // glFlush要求OpenGL尽快执行此前提交的绘图命令
}

// 窗口缩放回调：用户改变窗口大小后，怎样重新建立窗口坐标系
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    gWindowWidth = width;
    gWindowHeight = height;

    glViewport(0, 0, width, height); // glViewport决定绘制的窗口区域，四个参数分别为（左下角x、左下角y、宽度、高度）

    glMatrixMode(GL_PROJECTION); // 为了设置投影暂时选择了投影矩阵
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0); // glOrtho建立与窗口大小对应的二维坐标系

    glMatrixMode(GL_MODELVIEW); // 设置完后要将矩阵切回模型视图矩阵
    glLoadIdentity();
}

// 键盘回调：在macOS和Windows上都可以按Esc退出程序
void keyboard(unsigned char key, int, int)
{
    constexpr unsigned char kEscapeKey = 27;

    if (key == kEscapeKey)
    {
        std::exit(EXIT_SUCCESS); // std::exit立即以成功状态结束程序
    }
}

// OpenGL初始化：使用白色作为清屏颜色，与题目中的黑白示意图一致
void initializeOpenGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // glClearColor设置之后清屏时使用的白色背景
}

} // 匿名命名空间

// 程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // 初始化GLUT并读取可能存在的命令行参数
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // 设置显示模式：使用RGB颜色和单缓冲显示模式
    glutInitWindowSize(kInitialWindowWidth, kInitialWindowHeight); // 设置窗口初始尺寸
    glutInitWindowPosition(100, 100); // 设置窗口首次出现的位置
    glutCreateWindow("General DDA Line Rasterization"); // 创建窗口并设置标题

    initializeOpenGL();

    glutDisplayFunc(display); // 告诉GLUT当窗口需要重绘时，请调用display函数
    glutReshapeFunc(reshape); // 同上，当窗口尺寸变化时...
    glutKeyboardFunc(keyboard); // 同上，当用户按下键盘时...

    glutMainLoop(); // 进入事件循环，持续等待绘制、缩放和键盘事件

    return EXIT_SUCCESS;
}
