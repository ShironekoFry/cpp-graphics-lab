#include <array>
#include <cstdlib>

// macOS使用系统自带的GLUT，Windows和Linux使用FreeGLUT
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

namespace
{

// 窗口和矩形的常量定义
constexpr int kWindowWidth = 900;
constexpr int kWindowHeight = 420;
constexpr int kRectangleCount = 6;
constexpr int kRectangleWidth = 110;
constexpr int kRectangleHeight = 180;
constexpr int kBandLeft = 120; // 左侧起始位置
constexpr int kBandBottom = 120; // 底部起始位置

// 六个矩形的灰度值数组
constexpr std::array<float, kRectangleCount> kGrayLevels{
    0.2f,
    0.3f,
    0.4f,
    0.5f,
    0.6f,
    0.7f,
};

// 使用扫描转换逐像素填充矩形，同一矩形内的所有像素使用相同灰度。
// 循环采用左闭右开、下闭上开规则，避免相邻矩形的共享边被重复绘制。
void fillRectangleRasterized(
    int xMin,
    int yMin,
    int xMax,
    int yMax,
    float gray)
{
    glColor3f(gray, gray, gray); // 设置当前灰度值
    glBegin(GL_POINTS);

    for (int y = yMin; y < yMax; ++y)
    {
        for (int x = xMin; x < xMax; ++x)
        {
            glVertex2i(x, y); // 绘制当前像素点
        }
    }

    glEnd();
}

// 显示回调：从左到右绘制六个灰度逐渐增大的相邻矩形
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0f);

    // 绘制六个灰度逐渐增大的相邻矩形
    for (int index = 0; index < kRectangleCount; ++index)
    {
        const int xMin = kBandLeft + index * kRectangleWidth;
        const int xMax = xMin + kRectangleWidth;
        const int yMin = kBandBottom;
        const int yMax = yMin + kRectangleHeight;

        fillRectangleRasterized(
            xMin,
            yMin,
            xMax,
            yMax,
            kGrayLevels[static_cast<std::size_t>(index)]);
    }

    glFlush();
}

// 窗口缩放回调：保持逻辑坐标范围不变，窗口改变时重新显示马赫带
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, kWindowWidth, 0.0, kWindowHeight, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// 按Esc退出程序
void keyboard(unsigned char key, int, int)
{
    constexpr unsigned char kEscapeKey = 27;

    if (key == kEscapeKey)
    {
        std::exit(EXIT_SUCCESS);
    }
}

void initializeOpenGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

} // 匿名命名空间

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowSize(kWindowWidth, kWindowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Mach Band with Flat Shading");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return EXIT_SUCCESS;
}
