#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

// macOS 使用系统自带的 GLUT，Windows 和 Linux 使用 FreeGLUT
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

namespace
{

constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 700;

struct CPoint2
{
    int x;
    int y;
};

// 与题图中的多边形同构：题图坐标平移并放大 50 倍
const CPoint2 kPolygon[] = {
    {50, 100},
    {-150, 300},
    {-250, 50},
    {-150, -250},
    {0, -50},
    {100, -250},
    {300, 150},
};

constexpr int kPointCount =
    static_cast<int>(sizeof(kPolygon) / sizeof(kPolygon[0]));

// 栅栏填充：参考老师给的资料中的边填充算法的逐边取补逻辑，
// 只把取补范围由“交点到包围盒右边界”改为“交点到栅栏”。
void fenceFill(const CPoint2* points, int count)
{
    if (points == nullptr || count < 3) // 至少需要三个点才能构成多边形
    {
        return;
    }

    // 计算多边形的包围盒
    int xMin = points[0].x;
    int xMax = points[0].x;
    int yMin = points[0].y;
    int yMax = points[0].y;

    // 计算包围盒的最小和最大坐标
    for (int i = 1; i < count; ++i)
    {
        xMin = std::min(xMin, points[i].x);
        xMax = std::max(xMax, points[i].x);
        yMin = std::min(yMin, points[i].y);
        yMax = std::max(yMax, points[i].y);
    }

    const int width = xMax - xMin;
    const int height = yMax - yMin;
    if (width <= 0 || height <= 0)
    {
        return;
    }

    // 创建一个像素缓冲区，用于存储每个像素的填充状态（0 表示未填充，1 表示已填充）
    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(width) * height,
        0);

    const int fenceX = points[4].x; // 栅栏的 x 坐标

    int ymin;
    int ymax;
    double xYmin;
    double inverseSlope;

    // 遍历多边形的每条边，计算扫描线与边的交点，并进行取补操作
    for (int i = 0; i < count; ++i)
    {
        const int next = (i + 1) % count;

        // 水平边不改变扫描线交点的奇偶性，不参与取补。
        if (points[i].y == points[next].y)
        {
            continue;
        }

        // 计算斜率的倒数 1/k = dx/dy。
        inverseSlope =
            static_cast<double>(points[i].x - points[next].x)
            / (points[i].y - points[next].y);

        // 确定 ymin、ymax 和 xYmin，确保 ymin < ymax。
        if (points[i].y < points[next].y)
        {
            ymin = points[i].y;
            ymax = points[next].y;
            xYmin = points[i].x;
        }
        else
        {
            ymin = points[next].y;
            ymax = points[i].y;
            xYmin = points[next].x;
        }

        // 使用 [ymin, ymax)，避免共享顶点被重复计算。
        for (int y = ymin; y < ymax; ++y)
        {
            const double stableX = xYmin + std::copysign(1.0e-9, xYmin);
            const int intersection = static_cast<int>(std::lround(stableX));

            int xBegin = std::min(intersection, fenceX);
            int xEnd = std::max(intersection, fenceX);
            xBegin = std::max(xBegin, xMin);
            xEnd = std::min(xEnd, xMax);

            const std::size_t row =
                static_cast<std::size_t>(y - yMin) * width;

            // 将边与栅栏之间的像素逐个取补。
            for (int x = xBegin; x < xEnd; ++x)
            {
                unsigned char& pixel = pixels[row + (x - xMin)];
                if (pixel == 1)
                {
                    pixel = 0;
                }
                else
                {
                    pixel = 1;
                }
            }

            // 计算下一条扫描线与当前边的交点。
            xYmin += inverseSlope;
        }
    }

    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_POINTS);
    for (int y = yMin; y < yMax; ++y)
    {
        const std::size_t row =
            static_cast<std::size_t>(y - yMin) * width;

        for (int x = xMin; x < xMax; ++x)
        {
            if (pixels[row + (x - xMin)] == 1)
            {
                glVertex2i(x, y);
            }
        }
    }
    glEnd();
}

void drawPolygon()
{
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    for (const CPoint2& point : kPolygon)
    {
        glVertex2i(point.x, point.y);
    }
    glEnd();
}

void drawFence()
{
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2i(kPolygon[4].x, kPolygon[3].y);
    glVertex2i(kPolygon[4].x, kPolygon[1].y);
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0f);
    fenceFill(kPolygon, kPointCount);
    drawPolygon();
    drawFence();

    glFlush();
}

void reshape(int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double halfWidth = static_cast<double>(width) / 2.0;
    const double halfHeight = static_cast<double>(height) / 2.0;
    glOrtho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0, 1.0);

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
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glShadeModel(GL_FLAT);
}

} // 匿名命名空间

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowSize(kWindowWidth, kWindowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Fence Fill Algorithm");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return EXIT_SUCCESS;
}
