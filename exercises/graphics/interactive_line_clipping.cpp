#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

// macOS 使用系统自带的 GLUT，Windows 和 Linux 使用 FreeGLUT。
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

namespace
{
// 全局常量和类型定义。
constexpr int kInitialWindowWidth = 1000;
constexpr int kInitialWindowHeight = 760;
constexpr int kRayCount = 360;
constexpr double kPi = 3.14159265358979323846;
constexpr double kRayRadius = 300.0;
constexpr double kDefaultHalfWidth = 230.0;
constexpr double kDefaultHalfHeight = 130.0;
constexpr double kMinHalfExtent = 20.0;
constexpr double kMidpointTolerance = 1.0e-4;
constexpr int kMidpointMaxIterations = 80;
constexpr double kRotationStep = 5.0;
constexpr double kScaleFactor = 1.1;

constexpr int kLeftCode = 1;   // 左侧区域码：0001。
constexpr int kRightCode = 2;  // 右侧区域码：0010。
constexpr int kBottomCode = 4; // 下侧区域码：0100。
constexpr int kTopCode = 8;    // 上侧区域码：1000。

struct Point
{
    double x;
    double y;
};

struct Segment
{
    Point start;
    Point end;
};

// 三种裁剪算法都在矩形的局部坐标系中处理这个轴对齐窗口。
struct AxisAlignedWindow
{
    double left;
    double right;
    double bottom;
    double top;
};

// 屏幕中显示的裁剪窗口由中心、半宽、半高和旋转角共同确定。
struct ClipWindow
{
    Point center;
    double halfWidth;
    double halfHeight;
    double angleDegrees;
};

// 视图模式枚举。
enum class ViewMode
{
    BeforeClipping,
    AfterClipping,
};

// 裁剪算法枚举。
enum class ClipAlgorithm
{
    CohenSutherland,
    MidpointSubdivision,
    LiangBarsky,
};

// 裁剪区域形状枚举。圆是凸区域而不是多边形，因此使用独立的精确求交算法。
enum class ClipShape
{
    Rectangle,
    Triangle,
    Circle,
};

// GLUT 没有现成的按钮控件，因此算法选择按钮由矩形范围、标签和算法值组成。
// 绘制和点击检测共用同一组数据，避免按钮外观与实际响应区域不一致。
struct AlgorithmButton
{
    ClipAlgorithm algorithm;
    double left;
    double right;
    double bottom;
    double top;
    const char* label;
};

// 形状按钮只显示几何图标，不显示文字；按钮结构保存点击范围和对应形状。
struct ShapeButton
{
    ClipShape shape;
    double left;
    double right;
    double bottom;
    double top;
};

// 鼠标操作枚举。
enum class MouseOperation
{
    Idle,
    DrawingWindow,
    DraggingWindow,
    RotatingWindow,
};

// 全局状态变量的初始化
int gWindowWidth = kInitialWindowWidth;
int gWindowHeight = kInitialWindowHeight;
std::array<Segment, kRayCount> gRays{}; // 存储所有射线段。
ClipWindow gClipWindow{{0.0, 0.0}, kDefaultHalfWidth, kDefaultHalfHeight, 0.0}; // 当前裁剪窗口。
ClipWindow gWindowAtMouseDown = gClipWindow; // 鼠标按下时的裁剪窗口快照。
ViewMode gViewMode = ViewMode::BeforeClipping; // 当前视图模式。
ClipAlgorithm gClipAlgorithm = ClipAlgorithm::CohenSutherland; // 当前裁剪算法。
ClipShape gClipShape = ClipShape::Rectangle; // 当前裁剪区域形状。
MouseOperation gMouseOperation = MouseOperation::Idle; // 当前鼠标操作状态。
Point gMouseDownWorld{0.0, 0.0}; // 鼠标按下时的世界坐标。
double gRotationPointerAngle = 0.0; // 旋转操作时指针的角度。

// 角度单位转换函数。
// degreesToRadians: 将角度从度转换为弧度。
// radiansToDegrees: 将角度从弧度转换为度。
// normalizeAngle: 将角度归一化到 [-180, 180] 范围。
double degreesToRadians(double degrees)
{
    return degrees * kPi / 180.0;
}

double radiansToDegrees(double radians)
{
    return radians * 180.0 / kPi;
}

double normalizeAngle(double angleDegrees)
{
    // std::remainder 将角度限制在 [-180, 180]，避免持续旋转后数值不断增大。
    return std::remainder(angleDegrees, 360.0);
}

// 将屏幕坐标转换为世界坐标。
Point screenToWorld(int screenX, int screenY)
{
    // GLUT 鼠标坐标的原点位于左上角，y 轴向下；本程序的世界坐标原点
    // 位于窗口中心，y 轴向上，因此这里同时完成平移和 y 轴翻转。
    return {
        static_cast<double>(screenX) - gWindowWidth / 2.0,
        gWindowHeight / 2.0 - static_cast<double>(screenY),
    };
}

// 将世界坐标转换为裁剪窗口的局部坐标。
Point worldToLocal(const Point& point, const ClipWindow& window)
{
    // 先把点平移到以裁剪窗口中心为原点的位置，再旋转 -angle。
    // 变换后，任意方向的旋转矩形都会成为与坐标轴平行的矩形，
    // 因而可以继续使用老师给出的三种矩形裁剪算法。
    const double angle = degreesToRadians(window.angleDegrees);
    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);
    const double translatedX = point.x - window.center.x;
    const double translatedY = point.y - window.center.y;

    return {
        cosine * translatedX + sine * translatedY,
        -sine * translatedX + cosine * translatedY,
    };
}

// 将裁剪窗口的局部坐标转换为世界坐标。
Point localToWorld(const Point& point, const ClipWindow& window)
{
    // 局部坐标中的裁剪结果需要旋转 angle 并移回窗口中心，
    // 才能重新显示在原来的世界坐标系中。
    const double angle = degreesToRadians(window.angleDegrees);
    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);

    return {
        window.center.x + cosine * point.x - sine * point.y,
        window.center.y + sine * point.x + cosine * point.y,
    };
}

// 创建裁剪窗口的局部坐标轴对齐窗口。
AxisAlignedWindow makeLocalWindow(const ClipWindow& window)
{
    return {
        -window.halfWidth,
        window.halfWidth,
        -window.halfHeight,
        window.halfHeight,
    };
}

// 创建局部坐标系中的等腰三角形。顶点使用逆时针顺序，
// 这样每条有向边的左侧都对应三角形内部，便于统一进行半平面判断。
std::array<Point, 3> makeLocalTriangle(const ClipWindow& window)
{
    return {{
        {-window.halfWidth, -window.halfHeight},
        {window.halfWidth, -window.halfHeight},
        {0.0, window.halfHeight},
    }};
}

// 圆形区域内接于框选得到的包围盒，半径由较短的半轴决定。
double circleRadius(const ClipWindow& window)
{
    return std::min(window.halfWidth, window.halfHeight);
}

// 计算二维向量叉积。结果为正表示第二个向量位于第一个向量的左侧。
double crossProduct(const Point& first, const Point& second)
{
    return first.x * second.y - first.y * second.x;
}

// 判断局部坐标点是否位于逆时针凸多边形的所有内侧半平面中。
template <std::size_t VertexCount>
bool pointInsideConvexPolygon(
    const Point& point,
    const std::array<Point, VertexCount>& vertices)
{
    constexpr double kGeometryTolerance = 1.0e-9;

    for (std::size_t index = 0; index < VertexCount; ++index)
    {
        const Point& first = vertices[index];
        const Point& second = vertices[(index + 1) % VertexCount];
        const Point edge{second.x - first.x, second.y - first.y};
        const Point offset{point.x - first.x, point.y - first.y};

        if (crossProduct(edge, offset) < -kGeometryTolerance)
        {
            return false;
        }
    }

    return true;
}

// 使用凸多边形各边对应的内侧半平面裁剪线段。
// 线段写成 P(t)=start+t*direction，并逐边收缩 t 的可见区间。
template <std::size_t VertexCount>
bool convexPolygonClip(
    Point& start,
    Point& end,
    const std::array<Point, VertexCount>& vertices)
{
    constexpr double kGeometryTolerance = 1.0e-12;
    double lower = 0.0;
    double upper = 1.0;
    const Point originalStart = start;
    const Point direction{end.x - start.x, end.y - start.y};

    for (std::size_t index = 0; index < VertexCount; ++index)
    {
        const Point& first = vertices[index];
        const Point& second = vertices[(index + 1) % VertexCount];
        const Point edge{second.x - first.x, second.y - first.y};
        const Point startOffset{
            originalStart.x - first.x,
            originalStart.y - first.y,
        };
        const double constant = crossProduct(edge, startOffset);
        const double coefficient = crossProduct(edge, direction);

        // 系数接近零表示线段与当前边平行；若起点又位于外侧，
        // 整条线段都不可能进入这个半平面。
        if (std::abs(coefficient) <= kGeometryTolerance)
        {
            if (constant < 0.0)
            {
                return false;
            }
            continue;
        }

        const double parameter = -constant / coefficient;
        if (coefficient > 0.0)
        {
            lower = std::max(lower, parameter);
        }
        else
        {
            upper = std::min(upper, parameter);
        }

        if (lower > upper)
        {
            return false;
        }
    }

    start = {
        originalStart.x + lower * direction.x,
        originalStart.y + lower * direction.y,
    };
    end = {
        originalStart.x + upper * direction.x,
        originalStart.y + upper * direction.y,
    };
    return true;
}

// 使用线段参数方程与圆方程联立得到的二次方程精确裁剪圆形区域。
bool circleClip(Point& start, Point& end, double radius)
{
    constexpr double kGeometryTolerance = 1.0e-12;
    const Point originalStart = start;
    const Point direction{end.x - start.x, end.y - start.y};
    const double quadratic = direction.x * direction.x
        + direction.y * direction.y;

    // 退化为单点时，只需判断这个点是否位于圆内。
    if (quadratic <= kGeometryTolerance)
    {
        return start.x * start.x + start.y * start.y
            <= radius * radius + kGeometryTolerance;
    }

    const double linear = 2.0
        * (originalStart.x * direction.x + originalStart.y * direction.y);
    const double constant = originalStart.x * originalStart.x
        + originalStart.y * originalStart.y - radius * radius;
    double discriminant = linear * linear - 4.0 * quadratic * constant;

    if (discriminant < -kGeometryTolerance)
    {
        return false;
    }

    // 由于浮点舍入，理论上的相切情况可能得到极小负数，需要归零处理。
    discriminant = std::max(0.0, discriminant);
    const double squareRoot = std::sqrt(discriminant);
    const double firstRoot = (-linear - squareRoot) / (2.0 * quadratic);
    const double secondRoot = (-linear + squareRoot) / (2.0 * quadratic);
    const double lower = std::max(0.0, firstRoot);
    const double upper = std::min(1.0, secondRoot);

    if (lower > upper)
    {
        return false;
    }

    start = {
        originalStart.x + lower * direction.x,
        originalStart.y + lower * direction.y,
    };
    end = {
        originalStart.x + upper * direction.x,
        originalStart.y + upper * direction.y,
    };
    return true;
}

// 计算点相对于轴对齐窗口的区域码。
int computeOutCode(const Point& point, const AxisAlignedWindow& window)
{
    // 窗口边界上的点视为内部点，只有严格落在边界外时才设置区域码。
    int code = 0;

    if (point.x < window.left)
    {
        code |= kLeftCode;
    }
    else if (point.x > window.right)
    {
        code |= kRightCode;
    }

    if (point.y < window.bottom)
    {
        code |= kBottomCode;
    }
    else if (point.y > window.top)
    {
        code |= kTopCode;
    }

    return code;
}

// 使用 Cohen-Sutherland 算法裁剪线段。
bool cohenSutherlandClip(
    Point& start,
    Point& end,
    const AxisAlignedWindow& window)
{
    // 计算两个端点相对于裁剪窗口的区域码。
    int startCode = computeOutCode(start, window);
    int endCode = computeOutCode(end, window);

    while (true)
    {
        if ((startCode | endCode) == 0)
        {
            return true; // 两个端点都在窗口内，简取整条线段。
        }

        if ((startCode & endCode) != 0)
        {
            return false; // 两个端点位于同一条边界的外侧，简弃整条线段。
        }

        // 选择当前位于窗口外的端点，并计算其与窗口边界的交点。
        const int outsideCode = (startCode != 0) ? startCode : endCode;
        const double dx = end.x - start.x;
        const double dy = end.y - start.y;
        Point intersection{};

        // 根据区域码依次处理左、右、下、上边界，求交。
        if ((outsideCode & kLeftCode) != 0)
        {
            if (dx == 0.0)
            {
                return false;
            }
            intersection.x = window.left;
            intersection.y = start.y + dy * (window.left - start.x) / dx;
        }
        else if ((outsideCode & kRightCode) != 0)
        {
            if (dx == 0.0)
            {
                return false;
            }
            intersection.x = window.right;
            intersection.y = start.y + dy * (window.right - start.x) / dx;
        }
        else if ((outsideCode & kBottomCode) != 0)
        {
            if (dy == 0.0)
            {
                return false;
            }
            intersection.y = window.bottom;
            intersection.x = start.x + dx * (window.bottom - start.y) / dy;
        }
        else
        {
            if (dy == 0.0)
            {
                return false;
            }
            intersection.y = window.top;
            intersection.x = start.x + dx * (window.top - start.y) / dy;
        }

        // 只替换当前位于窗口外的端点，并重新编码。交点可能位于某条
        // 窗口边界的延长线上，因此不能在第一次求交后直接结束。
        if (outsideCode == startCode)
        {
            start = intersection;
            startCode = computeOutCode(start, window);
        }
        else
        {
            end = intersection;
            endCode = computeOutCode(end, window);
        }
    }
}

// 使用中点分割算法裁剪线段。
// 将位于窗口外的端点更新为二分逼近得到的可见边界点。
void midpointDivider(
    Point& outsidePoint,
    const Point& otherPoint,
    const AxisAlignedWindow& window)
{
    // outside 保存靠窗口外的一端，insideCandidate 保存另一端。
    // 每次检查前半段能否简弃，从而把交点始终夹在二分区间中。
    Point outside = outsidePoint;
    Point insideCandidate = otherPoint;

    for (int iteration = 0; iteration < kMidpointMaxIterations; ++iteration)
    {
        const double dx = insideCandidate.x - outside.x;
        const double dy = insideCandidate.y - outside.y;
        if (std::abs(dx) <= kMidpointTolerance
            && std::abs(dy) <= kMidpointTolerance)
        {
            break;
        }

        const Point midpoint{
            (outside.x + insideCandidate.x) / 2.0,
            (outside.y + insideCandidate.y) / 2.0,
        };

        // 若 outside 到 midpoint 的两端共享同一个非零区域位，
        // 这一半显然不可见，可以把窗口外端推进到 midpoint。
        if ((computeOutCode(outside, window)
             & computeOutCode(midpoint, window)) != 0)
        {
            outside = midpoint;
        }
        else
        {
            // 前半段不能简弃，交点仍在前半段内，因此收缩另一端。
            insideCandidate = midpoint;
        }
    }

    // 采用二分区间靠可见部分的一端，避免把近似的窗外点用于绘制。
    outsidePoint = insideCandidate;
}

// 使用中点分割算法裁剪线段。
// 返回 true 表示线段与窗口有可见部分，false 表示线段完全位于窗口外。
bool midpointSubdivisionClip(
    Point& start,
    Point& end,
    const AxisAlignedWindow& window)
{
    // 先用区域码简取或简弃，
    // 对剩余情况从一个窗外端点开始二分逼近最近的可见点。
    int startCode = computeOutCode(start, window);
    int endCode = computeOutCode(end, window);

    while (startCode != 0 || endCode != 0)
    {
        if ((startCode & endCode) != 0)
        {
            return false;
        }

        // midpointDivider 要求第一个参数位于窗口外；若 start 已在窗口内，
        // 就交换两个端点，从线段的另一端寻找可见边界。
        if (startCode == 0)
        {
            std::swap(start, end);
        }

        midpointDivider(start, end, window);
        startCode = computeOutCode(start, window);
        endCode = computeOutCode(end, window);

        // 二分后仍处于窗口外，表示线段只接近窗口边界而没有可见部分。
        if (startCode != 0)
        {
            return false;
        }
    }

    return true;
}

// 使用 Liang-Barsky 算法更新参数区间。
// 返回 true 表示参数区间仍然有效，false 表示线段与当前边界无可见部分。
bool liangBarskyClipTest(double p, double q, double& lower, double& upper)
{
    // p < 0 时产生始参下界，p > 0 时产生终参上界。
    // p == 0 表示线段与当前边界平行；q < 0 时整条线段位于外侧。
    if (p < 0.0)
    {
        const double parameter = q / p;
        if (parameter > upper)
        {
            return false;
        }
        lower = std::max(lower, parameter);
    }
    else if (p > 0.0)
    {
        const double parameter = q / p;
        if (parameter < lower)
        {
            return false;
        }
        upper = std::min(upper, parameter);
    }
    else if (q < 0.0)
    {
        return false;
    }

    return true;
}

// 使用 Liang-Barsky 算法裁剪线段。
bool liangBarskyClip(
    Point& start,
    Point& end,
    const AxisAlignedWindow& window)
{
    double lower = 0.0;
    double upper = 1.0;
    const double dx = end.x - start.x;
    const double dy = end.y - start.y;

    // 四次测试依次对应左、右、下、上边界。
    if (!liangBarskyClipTest(-dx, start.x - window.left, lower, upper)
        || !liangBarskyClipTest(dx, window.right - start.x, lower, upper)
        || !liangBarskyClipTest(-dy, start.y - window.bottom, lower, upper)
        || !liangBarskyClipTest(dy, window.top - start.y, lower, upper))
    {
        return false;
    }

    if (lower > upper)
    {
        return false;
    }

    // 两个裁剪端点都必须依据原始起点和原始方向向量计算。
    const Point originalStart = start;
    start = {
        originalStart.x + lower * dx,
        originalStart.y + lower * dy,
    };
    end = {
        originalStart.x + upper * dx,
        originalStart.y + upper * dy,
    };

    return true;
}

// 根据当前形状和算法裁剪线段。
bool clipSegment(const Segment& source, Segment& result)
{
    // 先把线段变换到裁剪区域的局部坐标系，在规则形状中完成裁剪，
    // 再把结果变换回世界坐标系，因此平移和旋转不会改变算法本身。
    Point localStart = worldToLocal(source.start, gClipWindow);
    Point localEnd = worldToLocal(source.end, gClipWindow);
    bool visible = false;

    switch (gClipShape)
    {
    case ClipShape::Rectangle:
    {
        // 老师给出的三种算法都是矩形裁剪算法，完整保留原来的切换逻辑。
        const AxisAlignedWindow localWindow = makeLocalWindow(gClipWindow);
        switch (gClipAlgorithm)
        {
        case ClipAlgorithm::CohenSutherland:
            visible = cohenSutherlandClip(localStart, localEnd, localWindow);
            break;
        case ClipAlgorithm::MidpointSubdivision:
            visible = midpointSubdivisionClip(localStart, localEnd, localWindow);
            break;
        case ClipAlgorithm::LiangBarsky:
            visible = liangBarskyClip(localStart, localEnd, localWindow);
            break;
        }
        break;
    }
    case ClipShape::Triangle:
        // 三角形是凸多边形，使用逐边收缩参数区间的半平面裁剪。
        visible = convexPolygonClip(
            localStart,
            localEnd,
            makeLocalTriangle(gClipWindow));
        break;
    case ClipShape::Circle:
        // 圆并不是多边形，因此直接求线段与圆的精确交点。
        visible = circleClip(
            localStart,
            localEnd,
            circleRadius(gClipWindow));
        break;
    }

    if (!visible)
    {
        return false;
    }

    result.start = localToWorld(localStart, gClipWindow);
    result.end = localToWorld(localEnd, gClipWindow);
    return true;
}

// 创建从画面中心向圆周辐射的线段。
void createRays()
{
    // 题目要求从画面中心向半径为 r 的圆周每隔 1 度绘制一条线，
    // 因而恰好生成 360 条彼此独立的原始线段。
    for (int degree = 0; degree < kRayCount; ++degree)
    {
        const double angle = degreesToRadians(static_cast<double>(degree));
        gRays[degree] = {
            {0.0, 0.0},
            {kRayRadius * std::cos(angle), kRayRadius * std::sin(angle)},
        };
    }
}

// 判断点是否位于裁剪窗口内。
bool pointInsideClipWindow(const Point& point)
{
    constexpr double kGeometryTolerance = 1.0e-9;
    const Point local = worldToLocal(point, gClipWindow);

    switch (gClipShape)
    {
    case ClipShape::Rectangle:
        return std::abs(local.x) <= gClipWindow.halfWidth
            && std::abs(local.y) <= gClipWindow.halfHeight;
    case ClipShape::Triangle:
        return pointInsideConvexPolygon(
            local,
            makeLocalTriangle(gClipWindow));
    case ClipShape::Circle:
    {
        const double radius = circleRadius(gClipWindow);
        return local.x * local.x + local.y * local.y
            <= radius * radius + kGeometryTolerance;
    }
    }

    return false;
}

// 获取裁剪窗口的四个角点。
std::array<Point, 4> clipWindowCorners()
{
    const std::array<Point, 4> localCorners{{
        {-gClipWindow.halfWidth, -gClipWindow.halfHeight},
        {gClipWindow.halfWidth, -gClipWindow.halfHeight},
        {gClipWindow.halfWidth, gClipWindow.halfHeight},
        {-gClipWindow.halfWidth, gClipWindow.halfHeight},
    }};

    std::array<Point, 4> worldCorners{};
    for (std::size_t index = 0; index < localCorners.size(); ++index)
    {
        worldCorners[index] = localToWorld(localCorners[index], gClipWindow);
    }
    return worldCorners;
}

// 绘制辐射线段。
void drawRays()
{
    if (gViewMode == ViewMode::BeforeClipping)
    {
        glColor3ub(85, 85, 85);
    }
    else
    {
        glColor3ub(20, 20, 20);
    }

    glLineWidth(1.0f);
    glBegin(GL_LINES);

    for (const Segment& ray : gRays)
    {
        Segment displayed = ray;
        if (gViewMode == ViewMode::AfterClipping && !clipSegment(ray, displayed))
        {
            continue;
        }

        glVertex2d(displayed.start.x, displayed.start.y);
        glVertex2d(displayed.end.x, displayed.end.y);
    }

    glEnd();
}

// 绘制裁剪窗口。
void drawClipWindow()
{
    glColor3ub(0, 125, 70);
    glLineWidth(3.0f);

    if (gClipShape == ClipShape::Rectangle)
    {
        const std::array<Point, 4> corners = clipWindowCorners();

        glBegin(GL_LINE_LOOP);
        for (const Point& corner : corners)
        {
            glVertex2d(corner.x, corner.y);
        }
        glEnd();

        // 四个角点既帮助用户观察旋转方向，也提示当前矩形可以缩放和旋转。
        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (const Point& corner : corners)
        {
            glVertex2d(corner.x, corner.y);
        }
        glEnd();
    }
    else if (gClipShape == ClipShape::Triangle)
    {
        const std::array<Point, 3> localVertices = makeLocalTriangle(gClipWindow);
        std::array<Point, 3> worldVertices{};
        for (std::size_t index = 0; index < localVertices.size(); ++index)
        {
            worldVertices[index] = localToWorld(localVertices[index], gClipWindow);
        }

        glBegin(GL_LINE_LOOP);
        for (const Point& vertex : worldVertices)
        {
            glVertex2d(vertex.x, vertex.y);
        }
        glEnd();

        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (const Point& vertex : worldVertices)
        {
            glVertex2d(vertex.x, vertex.y);
        }
        glEnd();
    }
    else
    {
        constexpr int kCircleSegments = 96;
        const double radius = circleRadius(gClipWindow);

        // 用足够密集的折线近似显示圆周；实际裁剪仍使用精确圆方程。
        glBegin(GL_LINE_LOOP);
        for (int index = 0; index < kCircleSegments; ++index)
        {
            const double angle = 2.0 * kPi * index / kCircleSegments;
            const Point world = localToWorld(
                {radius * std::cos(angle), radius * std::sin(angle)},
                gClipWindow);
            glVertex2d(world.x, world.y);
        }
        glEnd();

        // 四个方向点提示圆形同样可以平移和缩放。
        const std::array<Point, 4> localHandles{{
            {-radius, 0.0},
            {radius, 0.0},
            {0.0, -radius},
            {0.0, radius},
        }};
        glPointSize(7.0f);
        glBegin(GL_POINTS);
        for (const Point& handle : localHandles)
        {
            const Point world = localToWorld(handle, gClipWindow);
            glVertex2d(world.x, world.y);
        }
        glEnd();
    }

    // 在裁剪区域中心绘制一个小十字，拖动时更容易观察平移结果。
    constexpr double kCenterMarkRadius = 6.0;
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2d(gClipWindow.center.x - kCenterMarkRadius, gClipWindow.center.y);
    glVertex2d(gClipWindow.center.x + kCenterMarkRadius, gClipWindow.center.y);
    glVertex2d(gClipWindow.center.x, gClipWindow.center.y - kCenterMarkRadius);
    glVertex2d(gClipWindow.center.x, gClipWindow.center.y + kCenterMarkRadius);
    glEnd();
}

// 绘制文本。
void drawText(double x, double y, const std::string& text)
{
    glColor3ub(25, 25, 25);
    glRasterPos2d(x, y);
    for (unsigned char character : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
    }
}

// 绘制彩色文本。
void drawColoredText(
    double x,
    double y,
    const std::string& text,
    unsigned char red,
    unsigned char green,
    unsigned char blue)
{
    glColor3ub(red, green, blue);
    glRasterPos2d(x, y);
    for (unsigned char character : text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
    }
}

// 获取当前裁剪算法名称。
const char* algorithmName()
{
    switch (gClipAlgorithm)
    {
    case ClipAlgorithm::CohenSutherland:
        return "Cohen-Sutherland";
    case ClipAlgorithm::MidpointSubdivision:
        return "Midpoint Subdivision";
    case ClipAlgorithm::LiangBarsky:
        return "Liang-Barsky";
    }
    return "Unknown";
}

// 获取当前裁剪区域形状名称。
const char* shapeName()
{
    switch (gClipShape)
    {
    case ClipShape::Rectangle:
        return "Rectangle";
    case ClipShape::Triangle:
        return "Triangle";
    case ClipShape::Circle:
        return "Circle";
    }
    return "Unknown";
}

// 获取当前视图模式名称。
const char* viewName()
{
    return (gViewMode == ViewMode::BeforeClipping)
        ? "Before clipping"
        : "After clipping";
}

// 获取算法按钮的位置和尺寸。
std::array<AlgorithmButton, 3> algorithmButtons()
{
    constexpr double kButtonHeight = 25.0;
    constexpr double kButtonGap = 8.0;
    constexpr double kCohenWidth = 145.0;
    constexpr double kMidpointWidth = 170.0;
    constexpr double kLiangWidth = 125.0;

    const double firstLeft = -gWindowWidth / 2.0 + 18.0;
    const double buttonTop = gWindowHeight / 2.0 - 39.0;
    const double buttonBottom = buttonTop - kButtonHeight;
    const double secondLeft = firstLeft + kCohenWidth + kButtonGap;
    const double thirdLeft = secondLeft + kMidpointWidth + kButtonGap;

    return {{
        {
            ClipAlgorithm::CohenSutherland,
            firstLeft,
            firstLeft + kCohenWidth,
            buttonBottom,
            buttonTop,
            "Cohen-Sutherland",
        },
        {
            ClipAlgorithm::MidpointSubdivision,
            secondLeft,
            secondLeft + kMidpointWidth,
            buttonBottom,
            buttonTop,
            "Midpoint Subdivision",
        },
        {
            ClipAlgorithm::LiangBarsky,
            thirdLeft,
            thirdLeft + kLiangWidth,
            buttonBottom,
            buttonTop,
            "Liang-Barsky",
        },
    }};
}

// 获取形状按钮的位置和尺寸。按钮放在算法按钮同一行的右侧，
// 不移动已有按钮，也不改变画面主体与底部说明的位置。
std::array<ShapeButton, 3> shapeButtons()
{
    constexpr double kButtonSize = 29.0;
    constexpr double kButtonGap = 8.0;
    const double lastRight = gWindowWidth / 2.0 - 18.0;
    const double buttonTop = gWindowHeight / 2.0 - 39.0;
    const double buttonBottom = buttonTop - kButtonSize;
    const double circleLeft = lastRight - kButtonSize;
    const double triangleLeft = circleLeft - kButtonGap - kButtonSize;
    const double rectangleLeft = triangleLeft - kButtonGap - kButtonSize;

    return {{
        {
            ClipShape::Rectangle,
            rectangleLeft,
            rectangleLeft + kButtonSize,
            buttonBottom,
            buttonTop,
        },
        {
            ClipShape::Triangle,
            triangleLeft,
            triangleLeft + kButtonSize,
            buttonBottom,
            buttonTop,
        },
        {
            ClipShape::Circle,
            circleLeft,
            circleLeft + kButtonSize,
            buttonBottom,
            buttonTop,
        },
    }};
}

// 绘制算法按钮。
void drawAlgorithmButtons()
{
    const bool enabled = gClipShape == ClipShape::Rectangle;

    for (const AlgorithmButton& button : algorithmButtons())
    {
        const bool selected = enabled && button.algorithm == gClipAlgorithm;

        if (selected)
        {
            glColor3ub(0, 125, 70);
        }
        else if (!enabled)
        {
            glColor3ub(247, 247, 247);
        }
        else
        {
            glColor3ub(235, 235, 235);
        }

        glBegin(GL_QUADS);
        glVertex2d(button.left, button.bottom);
        glVertex2d(button.right, button.bottom);
        glVertex2d(button.right, button.top);
        glVertex2d(button.left, button.top);
        glEnd();

        if (enabled)
        {
            glColor3ub(35, 35, 35);
        }
        else
        {
            glColor3ub(185, 185, 185);
        }
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2d(button.left, button.bottom);
        glVertex2d(button.right, button.bottom);
        glVertex2d(button.right, button.top);
        glVertex2d(button.left, button.top);
        glEnd();

        if (selected)
        {
            drawColoredText(
                button.left + 9.0,
                button.bottom + 8.0,
                button.label,
                255,
                255,
                255);
        }
        else if (enabled)
        {
            drawText(
                button.left + 9.0,
                button.bottom + 8.0,
                button.label);
        }
        else
        {
            drawColoredText(
                button.left + 9.0,
                button.bottom + 8.0,
                button.label,
                150,
                150,
                150);
        }
    }
}

// 绘制三个无文字形状按钮。选中项沿用页面原有的绿色强调色，
// 图标本身分别是正方形、三角形和圆形。
void drawShapeButtons()
{
    constexpr int kCircleIconSegments = 24;

    for (const ShapeButton& button : shapeButtons())
    {
        const bool selected = button.shape == gClipShape;
        const double centerX = (button.left + button.right) / 2.0;
        const double centerY = (button.bottom + button.top) / 2.0;
        constexpr double kIconRadius = 7.0;

        if (selected)
        {
            glColor3ub(0, 125, 70);
        }
        else
        {
            glColor3ub(235, 235, 235);
        }
        glBegin(GL_QUADS);
        glVertex2d(button.left, button.bottom);
        glVertex2d(button.right, button.bottom);
        glVertex2d(button.right, button.top);
        glVertex2d(button.left, button.top);
        glEnd();

        glColor3ub(35, 35, 35);
        glLineWidth(1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2d(button.left, button.bottom);
        glVertex2d(button.right, button.bottom);
        glVertex2d(button.right, button.top);
        glVertex2d(button.left, button.top);
        glEnd();

        if (selected)
        {
            glColor3ub(255, 255, 255);
        }
        else
        {
            glColor3ub(35, 35, 35);
        }
        glLineWidth(1.5f);

        switch (button.shape)
        {
        case ClipShape::Rectangle:
            glBegin(GL_LINE_LOOP);
            glVertex2d(centerX - kIconRadius, centerY - kIconRadius);
            glVertex2d(centerX + kIconRadius, centerY - kIconRadius);
            glVertex2d(centerX + kIconRadius, centerY + kIconRadius);
            glVertex2d(centerX - kIconRadius, centerY + kIconRadius);
            glEnd();
            break;
        case ClipShape::Triangle:
            glBegin(GL_LINE_LOOP);
            glVertex2d(centerX, centerY + kIconRadius + 1.0);
            glVertex2d(centerX - kIconRadius - 1.0, centerY - kIconRadius);
            glVertex2d(centerX + kIconRadius + 1.0, centerY - kIconRadius);
            glEnd();
            break;
        case ClipShape::Circle:
            glBegin(GL_LINE_LOOP);
            for (int index = 0; index < kCircleIconSegments; ++index)
            {
                const double angle = 2.0 * kPi * index / kCircleIconSegments;
                glVertex2d(
                    centerX + kIconRadius * std::cos(angle),
                    centerY + kIconRadius * std::sin(angle));
            }
            glEnd();
            break;
        }
    }
}

// 处理算法按钮点击事件。
bool selectAlgorithmButton(const Point& point)
{
    for (const AlgorithmButton& button : algorithmButtons())
    {
        if (point.x >= button.left && point.x <= button.right
            && point.y >= button.bottom && point.y <= button.top)
        {
            // 非矩形区域不使用矩形专用算法，但仍消耗本次按钮点击，
            // 避免点击变灰按钮时误触发场景中的重新框选。
            if (gClipShape == ClipShape::Rectangle)
            {
                gClipAlgorithm = button.algorithm;
            }
            return true;
        }
    }
    return false;
}

// 处理形状按钮点击事件。
bool selectShapeButton(const Point& point)
{
    for (const ShapeButton& button : shapeButtons())
    {
        if (point.x >= button.left && point.x <= button.right
            && point.y >= button.bottom && point.y <= button.top)
        {
            gClipShape = button.shape;
            return true;
        }
    }
    return false;
}

// 绘制界面文本。
void drawInterfaceText()
{
    const double left = -gWindowWidth / 2.0 + 18.0;
    const double top = gWindowHeight / 2.0 - 24.0;
    const double bottom = -gWindowHeight / 2.0 + 20.0;

    std::ostringstream status;
    status << "View: " << viewName();
    if (gClipShape == ClipShape::Rectangle)
    {
        status << "    Active algorithm: " << algorithmName();
    }
    else if (gClipShape == ClipShape::Triangle)
    {
        status << "    Clipper: Convex polygon";
    }
    else
    {
        status << "    Clipper: Exact circle";
    }
    drawText(left, top, status.str());

    std::ostringstream windowStatus;
    windowStatus << std::fixed << std::setprecision(1)
                 << "Shape: " << shapeName()
                 << "    Window: " << gClipWindow.halfWidth * 2.0
                 << " x " << gClipWindow.halfHeight * 2.0
                 << "    Angle: " << gClipWindow.angleDegrees << " deg";
    drawText(left + 475.0, top, windowStatus.str());

    drawText(
        left,
        bottom + 20.0,
        "Click names/icons: algorithm/shape   Space: before/after   Left drag: select or move");
    drawText(
        left,
        bottom,
        "Wheel or +/-: scale   Right drag or Q/E: rotate   0: angle reset   R: reset   Esc: exit");
}

// 绘制界面。
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 始终先绘制射线，再绘制裁剪窗口和界面，使窗口边框不会被射线遮挡。
    drawRays();
    drawClipWindow();
    drawAlgorithmButtons();
    drawShapeButtons();
    drawInterfaceText();

    glutSwapBuffers();
}

// 处理窗口尺寸变化。
void reshape(int width, int height)
{
    gWindowWidth = std::max(width, 1);
    gWindowHeight = std::max(height, 1);

    glViewport(0, 0, gWindowWidth, gWindowHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // 保持一个世界坐标单位对应一个窗口像素，窗口缩放时图形尺寸不变。
    const double halfWidth = gWindowWidth / 2.0;
    const double halfHeight = gWindowHeight / 2.0;
    glOrtho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// 缩放裁剪窗口。
void scaleClipWindow(double factor)
{
    // 半宽和半高同时按相同比例变化，因此不会改变矩形的宽高比。
    gClipWindow.halfWidth = std::max(
        kMinHalfExtent,
        gClipWindow.halfWidth * factor);
    gClipWindow.halfHeight = std::max(
        kMinHalfExtent,
        gClipWindow.halfHeight * factor);
}

// 旋转裁剪窗口。
void rotateClipWindow(double deltaDegrees)
{
    gClipWindow.angleDegrees = normalizeAngle(
        gClipWindow.angleDegrees + deltaDegrees);
}

// 重置程序状态。
void resetProgram()
{
    gClipWindow = {{0.0, 0.0}, kDefaultHalfWidth, kDefaultHalfHeight, 0.0};
    gViewMode = ViewMode::BeforeClipping;
    gClipAlgorithm = ClipAlgorithm::CohenSutherland;
    gClipShape = ClipShape::Rectangle;
    gMouseOperation = MouseOperation::Idle;
}

// 处理键盘输入。
void keyboard(unsigned char key, int, int)
{
    switch (key)
    {
    case 'c':
    case 'C':
        if (gClipShape == ClipShape::Rectangle)
        {
            gClipAlgorithm = ClipAlgorithm::CohenSutherland;
        }
        break;
    case 'm':
    case 'M':
        if (gClipShape == ClipShape::Rectangle)
        {
            gClipAlgorithm = ClipAlgorithm::MidpointSubdivision;
        }
        break;
    case 'l':
    case 'L':
        if (gClipShape == ClipShape::Rectangle)
        {
            gClipAlgorithm = ClipAlgorithm::LiangBarsky;
        }
        break;
    case ' ':
        if (gMouseOperation == MouseOperation::Idle)
        {
            gViewMode = (gViewMode == ViewMode::BeforeClipping)
                ? ViewMode::AfterClipping
                : ViewMode::BeforeClipping;
        }
        break;
    case '+':
    case '=':
        scaleClipWindow(kScaleFactor);
        break;
    case '-':
    case '_':
        scaleClipWindow(1.0 / kScaleFactor);
        break;
    case 'q':
    case 'Q':
        rotateClipWindow(-kRotationStep);
        break;
    case 'e':
    case 'E':
        rotateClipWindow(kRotationStep);
        break;
    case '0':
        gClipWindow.angleDegrees = 0.0;
        break;
    case 'r':
    case 'R':
        resetProgram();
        break;
    case 27: // Esc 键使用 ASCII 码 27。
        std::exit(EXIT_SUCCESS);
    default:
        return;
    }

    glutPostRedisplay();
}

// 根据框选更新裁剪窗口。
void updateWindowFromSelection(const Point& current)
{
    // 鼠标可以沿任意方向框选，因此先使用两点的中点确定中心，
    // 再使用坐标差的绝对值确定半宽和半高。
    gClipWindow.center = {
        (gMouseDownWorld.x + current.x) / 2.0,
        (gMouseDownWorld.y + current.y) / 2.0,
    };
    gClipWindow.halfWidth = std::max(
        kMinHalfExtent,
        std::abs(current.x - gMouseDownWorld.x) / 2.0);
    gClipWindow.halfHeight = std::max(
        kMinHalfExtent,
        std::abs(current.y - gMouseDownWorld.y) / 2.0);
    gClipWindow.angleDegrees = 0.0;
}

// 处理鼠标输入。
void mouse(int button, int state, int screenX, int screenY)
{
    const Point world = screenToWorld(screenX, screenY);

    // GLUT 通常把滚轮上、下报告为按钮 3、4。键盘 +/- 同时作为兼容操作。
    if (state == GLUT_DOWN && button == 3)
    {
        scaleClipWindow(kScaleFactor);
        glutPostRedisplay();
        return;
    }
    if (state == GLUT_DOWN && button == 4)
    {
        scaleClipWindow(1.0 / kScaleFactor);
        glutPostRedisplay();
        return;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // 界面按钮优先于场景交互响应，点击按钮时不会误触发框选或拖动。
        if (selectShapeButton(world))
        {
            gMouseOperation = MouseOperation::Idle;
            glutPostRedisplay();
            return;
        }

        if (selectAlgorithmButton(world))
        {
            gMouseOperation = MouseOperation::Idle;
            glutPostRedisplay();
            return;
        }

        gMouseDownWorld = world;
        gWindowAtMouseDown = gClipWindow;

        if (pointInsideClipWindow(world))
        {
            // 在裁剪前后两种状态中都允许拖动已有窗口。
            gMouseOperation = MouseOperation::DraggingWindow;
        }
        else if (gViewMode == ViewMode::BeforeClipping)
        {
            // 只有裁剪前状态允许在空白处重新框选。裁剪后点击窗口外不会改变窗口。
            gMouseOperation = MouseOperation::DrawingWindow;
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
        if (gMouseOperation == MouseOperation::DrawingWindow)
        {
            const double selectedWidth = std::abs(world.x - gMouseDownWorld.x);
            const double selectedHeight = std::abs(world.y - gMouseDownWorld.y);

            // 过小的框选通常来自误点，恢复鼠标按下前的窗口，
            // 避免产生几乎不可见或无法再次选中的裁剪区域。
            if (selectedWidth < 2.0 * kMinHalfExtent
                || selectedHeight < 2.0 * kMinHalfExtent)
            {
                gClipWindow = gWindowAtMouseDown;
            }
            else
            {
                updateWindowFromSelection(world);
            }
        }
        gMouseOperation = MouseOperation::Idle;
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        if (pointInsideClipWindow(world))
        {
            gMouseOperation = MouseOperation::RotatingWindow;
            gMouseDownWorld = world;
            gWindowAtMouseDown = gClipWindow;
            gRotationPointerAngle = std::atan2(
                world.y - gClipWindow.center.y,
                world.x - gClipWindow.center.x);
        }
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        gMouseOperation = MouseOperation::Idle;
    }

    glutPostRedisplay();
}

// 处理鼠标移动。
void mouseMotion(int screenX, int screenY)
{
    const Point world = screenToWorld(screenX, screenY);

    switch (gMouseOperation)
    {
    case MouseOperation::DrawingWindow:
        updateWindowFromSelection(world);
        break;
    case MouseOperation::DraggingWindow:
        // 使用相对鼠标按下点的位移更新中心，保持窗口大小和角度不变。
        gClipWindow = gWindowAtMouseDown;
        gClipWindow.center.x += world.x - gMouseDownWorld.x;
        gClipWindow.center.y += world.y - gMouseDownWorld.y;
        break;
    case MouseOperation::RotatingWindow:
    {
        const double currentPointerAngle = std::atan2(
            world.y - gWindowAtMouseDown.center.y,
            world.x - gWindowAtMouseDown.center.x);
        const double rawDelta = currentPointerAngle - gRotationPointerAngle;

        // 用 atan2(sin, cos) 把角度差限制到 [-pi, pi]，
        // 避免鼠标跨过 -180/180 度时窗口突然跳转一整圈。
        const double wrappedDelta = std::atan2(
            std::sin(rawDelta),
            std::cos(rawDelta));
        gClipWindow = gWindowAtMouseDown;
        gClipWindow.angleDegrees = normalizeAngle(
            gWindowAtMouseDown.angleDegrees
            + radiansToDegrees(wrappedDelta));
        break;
    }
    case MouseOperation::Idle:
        return;
    }

    // 裁剪后拖动、缩放或旋转窗口时，display 会从原始 360 条射线重新裁剪，
    // 因此不会累积上一次裁剪造成的数据损失或数值误差。
    glutPostRedisplay();
}

// 初始化 OpenGL。
void initializeOpenGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glShadeModel(GL_FLAT);
}

} // 匿名命名空间

int main(int argc, char* argv[])
{
    createRays();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(kInitialWindowWidth, kInitialWindowHeight);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Interactive Line Clipping");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutMainLoop();

    return EXIT_SUCCESS;
}
