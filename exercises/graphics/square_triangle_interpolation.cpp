#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>
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

// 窗口和两个正方形位置的定义
constexpr int kWindowWidth = 1000;
constexpr int kWindowHeight = 520;
constexpr int kSquareSize = 300;
constexpr int kSquareBottom = 100;
constexpr int kLeftSquareX = 100;
constexpr int kRightSquareX = 600;

// 颜色结构体定义
struct Color
{
    float red;
    float green;
    float blue;
};

// 顶点结构体定义
struct Vertex
{
    float x;
    float y;
    Color color;
};

// 一条扫描线上的左右边标志点，同时保存标志点的插值颜色
struct EdgeFlags
{
    bool hasLeft = false; // 是否有左标志点
    bool hasRight = false; // 是否有右标志点
    float leftX = 0.0f; // 左标志点的x坐标
    float rightX = 0.0f; // 右标志点的x坐标
    Color leftColor{}; // 左标志点的颜色
    Color rightColor{}; // 右标志点的颜色
};

// 颜色插值函数定义
Color interpolateColor(const Color& start, const Color& end, float t)
{
    t = std::clamp(t, 0.0f, 1.0f); // 将插值参数限制在[0, 1]范围内

    return {
        start.red + (end.red - start.red) * t,
        start.green + (end.green - start.green) * t,
        start.blue + (end.blue - start.blue) * t,
    };
}

// 绘制文本函数
void drawText(float x, float y, const std::string& text)
{
    glRasterPos2f(x, y); // 设置文本起始位置
    for (const unsigned char character : text) // 遍历文本中的每个字符
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, character); // 绘制当前字符
    }
}

// 把一条三角形边离散到各条扫描线上，并更新每行的左右边标志点
void markEdge(
    const Vertex& first,
    const Vertex& second,
    std::vector<EdgeFlags>& scanlines)
{
    // 如果边的两个端点y坐标相同，则不处理该边
    if (first.y == second.y)
    {
        return;
    }

    // 确定边的下端点和上端点
    const Vertex* lower = &first;
    const Vertex* upper = &second;
    if (lower->y > upper->y)
    {
        std::swap(lower, upper);
    }

    const int yStart = static_cast<int>(std::ceil(lower->y));
    const int yEnd = static_cast<int>(std::ceil(upper->y));

    // 下闭上开：处理下端扫描线，不处理上端扫描线
    for (int y = yStart; y < yEnd; ++y)
    {
        if (y < 0 || y >= static_cast<int>(scanlines.size()))
        {
            continue;
        }

        // 计算当前扫描线与边的交点及插值颜色
        const float t = (static_cast<float>(y) - lower->y)
            / (upper->y - lower->y);
        const float x = lower->x + (upper->x - lower->x) * t;
        const Color color = interpolateColor(lower->color, upper->color, t);
        EdgeFlags& flags = scanlines[static_cast<std::size_t>(y)];

        // 更新当前扫描线的左右标志点
        if (!flags.hasLeft || x < flags.leftX)
        {
            flags.hasLeft = true;
            flags.leftX = x;
            flags.leftColor = color;
        }

        if (!flags.hasRight || x > flags.rightX)
        {
            flags.hasRight = true;
            flags.rightX = x;
            flags.rightColor = color;
        }
    }
}

// 先标记三条边，再在每条扫描线的左右标志点之间插值颜色并填充
void fillTriangleEdgeFlag(
    const Vertex& first,
    const Vertex& second,
    const Vertex& third)
{
    // 初始化扫描线数组，每条扫描线包含左右标志点信息
    // 扫描线数组的大小为窗口高度，每个元素代表一条扫描线的左右标志点
    std::vector<EdgeFlags> scanlines(kWindowHeight);

    markEdge(first, second, scanlines);
    markEdge(second, third, scanlines);
    markEdge(third, first, scanlines);

    glBegin(GL_POINTS);

    // 遍历每条扫描线，根据左右标志点插值颜色并填充三角形
    for (int y = 0; y < kWindowHeight; ++y)
    {
        // 获取当前扫描线的左右标志点信息
        const EdgeFlags& flags = scanlines[static_cast<std::size_t>(y)];
        if (!flags.hasLeft || !flags.hasRight || flags.rightX <= flags.leftX)
        {
            continue;
        }

        // 计算当前扫描线的左右边界像素范围
        const int xStart = static_cast<int>(std::ceil(flags.leftX));
        const int xEnd = static_cast<int>(std::ceil(flags.rightX));

        // 左闭右开：绘制左标志点，不绘制最右侧像素
        for (int x = xStart; x < xEnd; ++x)
        {
            const float t = (static_cast<float>(x) - flags.leftX)
                / (flags.rightX - flags.leftX);
            const Color color = interpolateColor(
                flags.leftColor,
                flags.rightColor,
                t);

            glColor3f(color.red, color.green, color.blue);
            glVertex2i(x, y);
        }
    }

    glEnd();
}

// 创建正方形的四个顶点，按左下、右下、右上、左上的顺序排列
std::array<Vertex, 4> makeSquareVertices(int left)
{
    constexpr Color kRed{1.0f, 0.0f, 0.0f};
    constexpr Color kGreen{0.0f, 1.0f, 0.0f};
    constexpr Color kYellow{1.0f, 1.0f, 0.0f};
    constexpr Color kBlue{0.0f, 0.0f, 1.0f};

    // 计算正方形的四个顶点的坐标
    const float xMin = static_cast<float>(left);
    const float xMax = static_cast<float>(left + kSquareSize);
    const float yMin = static_cast<float>(kSquareBottom);
    const float yMax = static_cast<float>(kSquareBottom + kSquareSize);

    // 顶点顺序：左下、右下、右上、左上
    return {{
        {xMin, yMin, kRed},
        {xMax, yMin, kGreen},
        {xMax, yMax, kYellow},
        {xMin, yMax, kBlue},
    }};
}

void drawSquares()
{
    const std::array<Vertex, 4> leftSquare = makeSquareVertices(kLeftSquareX);
    const std::array<Vertex, 4> rightSquare = makeSquareVertices(kRightSquareX);

    // 左图：沿左下到右上的对角线，分成左上和右下两个三角形
    fillTriangleEdgeFlag(leftSquare[0], leftSquare[2], leftSquare[3]);
    fillTriangleEdgeFlag(leftSquare[0], leftSquare[1], leftSquare[2]);

    // 右图：沿左上到右下的对角线，分成右上和左下两个三角形
    fillTriangleEdgeFlag(rightSquare[3], rightSquare[1], rightSquare[2]);
    fillTriangleEdgeFlag(rightSquare[3], rightSquare[0], rightSquare[1]);

    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(126.0f, 445.0f, "Bottom-left to top-right diagonal");
    drawText(626.0f, 445.0f, "Top-left to bottom-right diagonal");
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0f);
    drawSquares();
    glFlush();
}

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
    glutCreateWindow("Two Triangle Splits with Color Interpolation");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return EXIT_SUCCESS;
}
