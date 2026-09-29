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

// 设置窗口初始尺寸
constexpr int kWindowWidth = 640;
constexpr int kWindowHeight = 480;

// 菜单项使用这些整数值区分要绘制的图形和退出命令
enum class MenuOption
{
    Line = 1,
    Triangle = 2,
    Square = 3,
    Exit = 4
};

// 保存当前选择的图形，程序启动时默认显示直线
MenuOption gCurrentOption = MenuOption::Line;

// 保存主窗口编号，供菜单回调重新指定需要刷新的窗口
int gMainWindow = 0;

// 绘制一条水平线段
void drawLine()
{
    glBegin(GL_LINES); // glBegin指定接下来的每两个顶点组成一条独立线段
    glVertex2f(-0.65f, 0.0f); // glVertex2f提交一个二维浮点坐标顶点
    glVertex2f(0.65f, 0.0f);
    glEnd(); // glEnd结束本次顶点提交
}

// 绘制只有边框的三角形
void drawTriangle()
{
    glBegin(GL_LINE_LOOP); // GL_LINE_LOOP按顺序连接全部顶点，并自动连接最后一个点与第一个点
    glVertex2f(0.0f, 0.65f);
    glVertex2f(-0.65f, -0.5f);
    glVertex2f(0.65f, -0.5f);
    glEnd();
}

// 绘制只有边框的正方形
void drawSquare()
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.5f, 0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(-0.5f, -0.5f);
    glEnd();
}

// 显示回调：根据右键菜单保存的当前选项绘制对应图形
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);

    switch (gCurrentOption)
    {
        case MenuOption::Line:
            drawLine();
            break;
        case MenuOption::Triangle:
            drawTriangle();
            break;
        case MenuOption::Square:
            drawSquare();
            break;
        case MenuOption::Exit:
            break;
    }

    glFlush();
}

// 右键菜单回调：保存用户选择的图形，并通知GLUT重新绘制窗口
void menuHandler(int option)
{
    const MenuOption selectedOption = static_cast<MenuOption>(option);

    if (selectedOption == MenuOption::Exit)
    {
        std::exit(EXIT_SUCCESS);
    }

    gCurrentOption = selectedOption;
    glutSetWindow(gMainWindow); // 将主窗口设为当前窗口，确保重绘请求作用于正确窗口
    glutPostRedisplay(); // 请求GLUT在下一次事件循环中重新调用display函数
}

// 创建右键菜单，并把菜单项的整数编号交给menuHandler处理
void createRightClickMenu()
{
    glutCreateMenu(menuHandler); // 创建菜单并注册菜单选择回调函数
    glutAddMenuEntry("Line", static_cast<int>(MenuOption::Line)); // 向当前菜单添加一个选项
    glutAddMenuEntry("Triangle", static_cast<int>(MenuOption::Triangle));
    glutAddMenuEntry("Square", static_cast<int>(MenuOption::Square));
    glutAddMenuEntry("Exit", static_cast<int>(MenuOption::Exit));
    glutAttachMenu(GLUT_RIGHT_BUTTON); // 把当前菜单绑定到鼠标右键
}

// 窗口缩放回调：根据宽高比调整正交投影，防止图形变形
void reshape(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height); // glViewport决定绘制结果在窗口中使用的区域

    const double aspect = static_cast<double>(width) / height;

    glMatrixMode(GL_PROJECTION); // 选择投影矩阵，准备设置二维正交投影
    glLoadIdentity();

    if (aspect >= 1.0)
    {
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0); // glOrtho定义不会随窗口比例变形的正交坐标范围
    }
    else
    {
        glOrtho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// 键盘回调：除右键菜单中的Exit外，也可以按Esc退出程序
void keyboard(unsigned char key, int, int)
{
    constexpr unsigned char kEscapeKey = 27;

    if (key == kEscapeKey)
    {
        std::exit(EXIT_SUCCESS);
    }
}

// OpenGL初始化：使用黑色背景，使白色线框图形保持清晰
void initializeOpenGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
}

} 

// 程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv); // 初始化GLUT并读取可能存在的命令行参数
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // 设置RGB颜色和单缓冲显示模式
    glutInitWindowSize(kWindowWidth, kWindowHeight); // 设置窗口初始尺寸
    glutInitWindowPosition(100, 100); // 设置窗口首次出现的位置
    gMainWindow = glutCreateWindow("OpenGL Shape Menu - Right Click"); // 创建窗口并保存GLUT返回的窗口编号

    initializeOpenGL();
    createRightClickMenu();

    glutDisplayFunc(display); // 窗口需要重绘时调用display函数
    glutReshapeFunc(reshape); // 窗口尺寸变化时调用reshape函数
    glutKeyboardFunc(keyboard); // 用户按下键盘时调用keyboard函数

    glutMainLoop(); // 进入事件循环，持续等待绘制、菜单和键盘事件

    return EXIT_SUCCESS;
}
