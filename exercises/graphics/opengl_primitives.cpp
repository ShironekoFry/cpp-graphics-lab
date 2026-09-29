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

// 设置窗口初始尺寸，与PPT中保持一致
constexpr int kWindowWidth = 400;
constexpr int kWindowHeight = 400;

// 显示回调：先还原PPT中的场景，再增加三条白线组成倒三角形
void display()
{
    glClear(GL_COLOR_BUFFER_BIT); 

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 绘制白色矩形
    glColor3f(1.0f, 1.0f, 1.0f);
    glRectf(-0.5f, -0.5f, 0.5f, 0.5f); // glRectf：使用两个对角顶点绘制填充矩形

    // 绘制渐变三角形，每个顶点使用不同颜色
    glBegin(GL_TRIANGLES); // glBegin指定接下来的每三个顶点组成一个填充三角形
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 1.0f); // 提交一个顶点
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(0.8f, -0.5f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(-0.8f, -0.5f);
    glEnd(); // 结束顶点的提交

    // 绘制示范程序中的三个彩色点
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.4f, -0.4f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(0.4f, 0.4f);
    glEnd();

    // 连接大三角形三条边的中点，使用三条独立线段组成倒三角形
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f); // 设置后续线段的显示宽度
    glBegin(GL_LINES); // GL_LINES：每两个顶点组成一条独立线段
    glVertex2f(-0.4f, 0.25f);
    glVertex2f(0.4f, 0.25f);
    glVertex2f(0.4f, 0.25f);
    glVertex2f(0.0f, -0.5f);
    glVertex2f(0.0f, -0.5f);
    glVertex2f(-0.4f, 0.25f);
    glEnd();

    glFlush(); // 要求OpenGL尽快执行此前提交的绘图命令
}

// 窗口缩放回调：根据宽高比调整正交投影，防止三角形和矩形被拉伸
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    const double aspect = static_cast<double>(width) / height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (aspect >= 1.0)
    {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0); // 定义二维坐标系的范围
    }
    else
    {
        glOrtho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
    }

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

// OpenGL初始化：使用黑色背景和平滑着色
void initializeOpenGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
    glShadeModel(GL_SMOOTH); // 启用顶点颜色之间的平滑插值
}

}

// 程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // 初始化GLUT并读取可能存在的命令行参数
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // 设置RGB颜色和单缓冲显示模式
    glutInitWindowSize(kWindowWidth, kWindowHeight); // 设置窗口初始尺寸
    glutInitWindowPosition(100, 100); // 设置窗口首次出现的位置
    glutCreateWindow("OpenGL Primitive Composition"); // 创建窗口并设置标题

    initializeOpenGL();

    glutDisplayFunc(display); // 窗口需要重绘时调用display函数
    glutReshapeFunc(reshape); // 窗口尺寸变化时调用reshape函数
    glutKeyboardFunc(keyboard); // 用户按下键盘时调用keyboard函数

    glutMainLoop(); // 进入事件循环，持续等待绘制、缩放和键盘事件

    return EXIT_SUCCESS;
}
