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

// 题目给定的窗口尺寸和椭圆参数
constexpr int kWindowWidth = 640;
constexpr int kWindowHeight = 480;
constexpr int kCenterX = 320;
constexpr int kCenterY = 240;
constexpr int kSemiMajorA = 180;
constexpr int kSemiMinorB = 100;

// 定义一个整数像素坐标
struct Pixel
{
    int x;
    int y;
};

// 利用椭圆关于x轴和y轴的对称性，把第一象限中的一个点扩展为四个点
void appendSymmetricPixels(
    std::vector<Pixel>& pixels,
    int centerX,
    int centerY,
    int x,
    int y)
{
    pixels.push_back({centerX + x, centerY + y}); // push_back把像素添加到结果序列
    pixels.push_back({centerX - x, centerY + y});
    pixels.push_back({centerX + x, centerY - y});
    pixels.push_back({centerX - x, centerY - y});
}

// 中点椭圆算法：计算椭圆轮廓上的离散像素坐标
// 椭圆隐函数为F(x,y)=b²x²+a²y²-a²b²，中心暂时看作原点
// 第一象限的椭圆弧分成两个区域，因为两个区域的主位移方向不同
// 区域I以x为主位移方向，每一步x增加1，再判断y是否减少1
// 区域II以y为主位移方向，每一步y减少1，再判断x是否增加1
std::vector<Pixel> rasterizeEllipseMidpoint(
    int centerX,
    int centerY,
    int semiMajorA,
    int semiMinorB)
{
    std::vector<Pixel> pixels;

    // 长半轴或短半轴不是正数时，无法生成有效椭圆
    if (semiMajorA <= 0 || semiMinorB <= 0)
    {
        return pixels;
    }

    pixels.reserve(4 * (semiMajorA + semiMinorB + 2)); // 预先分配空间

    const long long aSquared = 1LL * semiMajorA * semiMajorA; // 1LL将a^2转化为long long类型
    const long long bSquared = 1LL * semiMinorB * semiMinorB;

    int x = 0;
    int y = semiMinorB;

    // 区域I的第一个判断中点是(1, b-0.5)，将它代入隐函数得到d1初值
    double d1 = bSquared + aSquared * (-semiMinorB + 0.25);

    appendSymmetricPixels(pixels, centerX, centerY, x, y);

    // 当椭圆法向量的x分量小于y分量时，仍处于以x为主位移方向的区域I
    while (bSquared * (x + 1) < aSquared * (y - 0.5))
    {
        if (d1 < 0.0)
        {
            // 中点位于椭圆内部，只需要沿x方向前进一步
            d1 += bSquared * (2 * x + 3);
        }
        else
        {
            // 中点位于椭圆外部，沿x方向前进的同时让y减少1
            d1 += bSquared * (2 * x + 3)
                + aSquared * (-2 * y + 2);
            --y;
        }

        ++x;
        appendSymmetricPixels(pixels, centerX, centerY, x, y);
    }

    // 进入区域II后，候选像素发生变化，因此重新计算中点误差项d2
    double d2 = bSquared * (x + 0.5) * (x + 0.5)
        + aSquared * (y - 1.0) * (y - 1.0)
        - aSquared * bSquared;

    while (y > 0)
    {
        if (d2 < 0.0)
        {
            // 中点位于椭圆内部，y减少1的同时还需要让x增加1
            d2 += bSquared * (2 * x + 2)
                + aSquared * (-2 * y + 3);
            ++x;
        }
        else
        {
            // 中点位于椭圆外部，只需要沿y方向向下前进一步
            d2 += aSquared * (-2 * y + 3);
        }

        --y;
        appendSymmetricPixels(pixels, centerX, centerY, x, y);
    }

    return pixels;
}

// 用OpenGL显示中点算法生成的像素
void drawPixels(const std::vector<Pixel>& pixels)
{
    glBegin(GL_POINTS); // 指定接下来的顶点按照独立像素点进行绘制

    for (const Pixel& pixel : pixels)
    {
        glVertex2i(pixel.x, pixel.y); // 提交一个二维整数坐标顶点
    }

    glEnd(); // 结束本次顶点提交
}

// 显示回调：清除旧画面并绘制题目指定的红色椭圆
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // 使用RGB浮点值把当前绘图颜色设置为红色

    const std::vector<Pixel> ellipsePixels = rasterizeEllipseMidpoint(
        kCenterX,
        kCenterY,
        kSemiMajorA,
        kSemiMinorB);

    drawPixels(ellipsePixels);
    glFlush();
}

// 窗口缩放回调：建立左下角为(0,0)、右上角为(width,height)的像素坐标系
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// 键盘回调
void keyboard(unsigned char key, int, int)
{
    constexpr unsigned char kEscapeKey = 27;

    if (key == kEscapeKey)
    {
        std::exit(EXIT_SUCCESS);
    }
}

// OpenGL初始化：这里将背景设置为白色
void initializeOpenGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

}

// 程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // 初始化GLUT并读取可能存在的命令行参数
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // 设置RGB颜色和单缓冲显示模式
    glutInitWindowSize(kWindowWidth, kWindowHeight); // 设置题目要求的640×480窗口
    glutInitWindowPosition(100, 100); // 设置窗口首次出现的位置
    glutCreateWindow("Midpoint Ellipse Rasterization"); // 创建窗口并设置标题

    initializeOpenGL();

    glutDisplayFunc(display); // 窗口需要重绘时调用display函数
    glutReshapeFunc(reshape); // 窗口尺寸变化时调用reshape函数
    glutKeyboardFunc(keyboard); // 用户按下键盘时调用keyboard函数

    glutMainLoop(); // 进入事件循环，持续等待绘制、缩放和键盘事件

    return EXIT_SUCCESS;
}
