#include <algorithm>
#include <array>
#include <cmath>
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

constexpr int kWindowWidth = 700;
constexpr int kWindowHeight = 700;
constexpr float kCenterX = 350.0f;
constexpr float kCenterY = 350.0f;
constexpr float kHexagonRadius = 245.0f;
constexpr float kPi = 3.14159265358979323846f;

struct Color
{
    float red;
    float green;
    float blue;
};

struct Vertex
{
    float x;
    float y;
    Color color;
};

constexpr Color kWhite{1.0f, 1.0f, 1.0f};

constexpr std::array<Color, 6> kVertexColors{{
    {1.0f, 0.0f, 0.0f}, // 红色
    {1.0f, 1.0f, 0.0f}, // 黄色
    {0.0f, 1.0f, 0.0f}, // 绿色
    {0.0f, 1.0f, 1.0f}, // 青色
    {0.0f, 0.0f, 1.0f}, // 蓝色
    {1.0f, 0.0f, 1.0f}, // 品红色
}};

// 生成正六边形的六个顶点
std::array<Vertex, 6> makeHexagonVertices()
{
    std::array<Vertex, 6> vertices{};

    for (std::size_t index = 0; index < vertices.size(); ++index)
    {
        const float angle = static_cast<float>(index) * kPi / 3.0f;
        vertices[index] = {
            kCenterX + kHexagonRadius * std::cos(angle),
            kCenterY + kHexagonRadius * std::sin(angle),
            kVertexColors[index],
        };
    }

    return vertices;
}

// 使用重心坐标判断像素是否在三角形内，并计算该像素的插值颜色
bool calculateBarycentricColor(
    float x,
    float y,
    const Vertex& first, // 三角形的第一个顶点
    const Vertex& second, // 三角形的第二个顶点
    const Vertex& third, // 三角形的第三个顶点
    Color& color)
{
    // 计算三角形面积的公式，用于判断点是否在三角形内
    const float denominator =
        (second.y - third.y) * (first.x - third.x)
        + (third.x - second.x) * (first.y - third.y);
    // 如果分母接近于零，说明无法构成有效三角形
    if (std::abs(denominator) < 1.0e-6f)
    {
        return false;
    }
    // 计算重心坐标 alpha, beta, gamma
    const float alpha =
        ((second.y - third.y) * (x - third.x)
        + (third.x - second.x) * (y - third.y))
        / denominator;
    const float beta =
        ((third.y - first.y) * (x - third.x)
        + (first.x - third.x) * (y - third.y))
        / denominator;
    const float gamma = 1.0f - alpha - beta;
    // 判断点是否在三角形内部，考虑一定的容差
    constexpr float kInsideTolerance = -1.0e-5f;
    if (alpha < kInsideTolerance
        || beta < kInsideTolerance
        || gamma < kInsideTolerance)
    {
        return false;
    }
    // 计算插值颜色
    color = {
        alpha * first.color.red
            + beta * second.color.red
            + gamma * third.color.red,
        alpha * first.color.green
            + beta * second.color.green
            + gamma * third.color.green,
        alpha * first.color.blue
            + beta * second.color.blue
            + gamma * third.color.blue,
    };

    return true;
}

// 使用重心坐标填充三角形
void fillTriangleBarycentric(
    const Vertex& first,
    const Vertex& second,
    const Vertex& third)
{
    // 计算三角形的边界框，用于确定需要扫描的像素范围
    const int xMin = static_cast<int>(std::floor(
        std::min({first.x, second.x, third.x})));
    const int xMax = static_cast<int>(std::ceil(
        std::max({first.x, second.x, third.x})));
    const int yMin = static_cast<int>(std::floor(
        std::min({first.y, second.y, third.y})));
    const int yMax = static_cast<int>(std::ceil(
        std::max({first.y, second.y, third.y})));

    glBegin(GL_POINTS);

    for (int y = yMin; y <= yMax; ++y)
    {
        for (int x = xMin; x <= xMax; ++x)
        {
            Color color{};

            // 使用像素中心进行三角形内点判断和颜色插值
            if (calculateBarycentricColor(
                    static_cast<float>(x) + 0.5f,
                    static_cast<float>(y) + 0.5f,
                    first,
                    second,
                    third,
                    color))
            {
                glColor3f(color.red, color.green, color.blue); // 设置当前像素的颜色
                glVertex2i(x, y); // 绘制当前像素
            }
        }
    }

    glEnd();
}

// 绘制平滑的正六边形
void drawSmoothHexagon()
{
    const Vertex center{kCenterX, kCenterY, kWhite};
    const std::array<Vertex, 6> vertices = makeHexagonVertices();

    // 将正六边形划分为六个三角形，每个三角形都使用重心坐标填充
    for (std::size_t index = 0; index < vertices.size(); ++index)
    {
        const std::size_t next = (index + 1) % vertices.size();
        fillTriangleBarycentric(center, vertices[index], vertices[next]); // 填充由中心和两个相邻顶点组成的三角形
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0f);
    drawSmoothHexagon();
    glFlush();
}

void reshape(int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);

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
    glutCreateWindow("Barycentric Smooth Hexagon");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return EXIT_SUCCESS;
}
