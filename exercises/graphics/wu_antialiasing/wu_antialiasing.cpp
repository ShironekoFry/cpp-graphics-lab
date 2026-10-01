#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// macOS使用系统自带的GLUT，Windows和Linux使用FreeGLUT
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <GLUT/glut.h>
#include <ImageIO/ImageIO.h>
#else
#include <GL/freeglut.h>
#endif

namespace
{

constexpr int kInitialWindowWidth = 1400;
constexpr int kInitialWindowHeight = 760;
constexpr int kPlotColumns = 4;
constexpr int kPlotRows = 2;
constexpr const char* kOutputImagePath =
    "exercises/graphics/wu_antialiasing/wu_antialiasing_result.png";

constexpr double kPlotLeft = -15.0;
constexpr double kPlotRight = 115.0;
constexpr double kPlotBottom = -65.0;
constexpr double kPlotTop = 65.0;

int gWindowWidth = kInitialWindowWidth;
int gWindowHeight = kInitialWindowHeight;
bool gImageSaved = false;

struct Point
{
    int x;
    int y;
};

struct LineCase
{
    const char* title;
    Point start;
    Point end;
};

constexpr std::array<LineCase, 8> kLineCases{{
    {"k = 0", {0, 0}, {90, 0}},
    {"k = 0.25", {0, 0}, {80, 20}},
    {"k = 0.5", {0, 0}, {80, 40}},
    {"k = 1", {0, 0}, {55, 55}},
    {"k = 2", {0, 0}, {27, 54}},
    {"k = -0.5", {0, 0}, {80, -40}},
    {"k = -2", {0, 0}, {27, -54}},
    {"vertical", {0, 0}, {0, 55}},
}};

int sign(int value)
{
    return (value > 0) - (value < 0);
}

void drawText(float x, float y, const std::string& text, void* font)
{
    glRasterPos2f(x, y);
    for (const unsigned char character : text)
    {
        glutBitmapCharacter(font, character);
    }
}

void drawAxes()
{
    // 使用浅灰色绘制坐标轴，避免与黑色直线混淆
    glColor3f(0.68f, 0.68f, 0.68f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(kPlotLeft + 4.0, 0.0);
    glVertex2f(kPlotRight - 4.0, 0.0);
    glVertex2f(0.0, kPlotBottom + 4.0);
    glVertex2f(0.0, kPlotTop - 8.0);
    glEnd();

    // 每20个坐标单位绘制一个短刻度
    glBegin(GL_LINES);
    for (int x = 0; x <= 100; x += 20)
    {
        glVertex2f(static_cast<float>(x), -1.5f);
        glVertex2f(static_cast<float>(x), 1.5f);
    }
    for (int y = -60; y <= 60; y += 20)
    {
        glVertex2f(-1.5f, static_cast<float>(y));
        glVertex2f(1.5f, static_cast<float>(y));
    }
    glEnd();

    // 只标出关键刻度，保持八个子图清晰
    glColor3f(0.35f, 0.35f, 0.35f);
    drawText(-4.0f, -7.0f, "0", GLUT_BITMAP_HELVETICA_10);
    drawText(47.0f, -7.0f, "50", GLUT_BITMAP_HELVETICA_10);
    drawText(96.0f, -7.0f, "100", GLUT_BITMAP_HELVETICA_10);
    drawText(-12.0f, 48.0f, "50", GLUT_BITMAP_HELVETICA_10);
    drawText(-15.0f, -52.0f, "-50", GLUT_BITMAP_HELVETICA_10);
    drawText(108.0f, 4.0f, "x", GLUT_BITMAP_HELVETICA_12);
    drawText(3.0f, 57.0f, "y", GLUT_BITMAP_HELVETICA_12);
}

void submitWeightedPixel(int x, int y, double coverage)
{
    if (coverage <= 0.0)
    {
        return;
    }

    coverage = std::clamp(coverage, 0.0, 1.0);
    const float gray = static_cast<float>(1.0 - coverage);
    glColor3f(gray, gray, gray);
    glVertex2i(x, y);
}

void drawWuLine(Point start, Point end)
{
    const int dx = end.x - start.x;
    const int dy = end.y - start.y;
    const int absDx = std::abs(dx);
    const int absDy = std::abs(dy);

    glPointSize(3.0f);
    glBegin(GL_POINTS);

    if (absDx == 0 && absDy == 0)
    {
        submitWeightedPixel(start.x, start.y, 1.0);
        glEnd();
        return;
    }

    if (absDx >= absDy)
    {
        // 缓斜线以x为主位移方向，每列选择上下两个相邻像素
        for (int step = 0; step <= absDx; ++step)
        {
            const double t = static_cast<double>(step) / absDx;
            const int x = start.x + step * sign(dx);
            const double idealY = start.y + t * dy;
            const int lowerY = static_cast<int>(std::floor(idealY));
            const double fraction = idealY - lowerY;

            submitWeightedPixel(x, lowerY, 1.0 - fraction);
            submitWeightedPixel(x, lowerY + 1, fraction);
        }
    }
    else
    {
        // 陡斜线以y为主位移方向，每行选择左右两个相邻像素
        for (int step = 0; step <= absDy; ++step)
        {
            const double t = static_cast<double>(step) / absDy;
            const int y = start.y + step * sign(dy);
            const double idealX = start.x + t * dx;
            const int leftX = static_cast<int>(std::floor(idealX));
            const double fraction = idealX - leftX;

            submitWeightedPixel(leftX, y, 1.0 - fraction);
            submitWeightedPixel(leftX + 1, y, fraction);
        }
    }

    glEnd();
}

void configurePlotProjection()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(kPlotLeft, kPlotRight, kPlotBottom, kPlotTop, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawPlot(const LineCase& lineCase)
{
    configurePlotProjection();
    drawAxes();
    drawWuLine(lineCase.start, lineCase.end);

    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(43.0f, 59.0f, lineCase.title, GLUT_BITMAP_HELVETICA_18);
}

bool saveFramebufferAsPng(const std::filesystem::path& outputPath)
{
#ifndef __APPLE__
    std::cerr << "PNG saving with ImageIO is only available on macOS.\n";
    return false;
#else
    if (gWindowWidth <= 0 || gWindowHeight <= 0)
    {
        return false;
    }

    std::error_code directoryError;
    std::filesystem::create_directories(outputPath.parent_path(), directoryError);
    if (directoryError)
    {
        std::cerr << "Failed to create output directory: "
                  << directoryError.message() << '\n';
        return false;
    }

    // OpenGL从左下角开始返回像素，PNG需要从左上角开始排列
    const std::size_t rowSize = static_cast<std::size_t>(gWindowWidth) * 3;
    const std::size_t imageSize = rowSize * static_cast<std::size_t>(gWindowHeight);
    std::vector<unsigned char> openGlPixels(imageSize);
    std::vector<unsigned char> pngPixels(imageSize);

    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(
        0,
        0,
        gWindowWidth,
        gWindowHeight,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        openGlPixels.data());

    for (int y = 0; y < gWindowHeight; ++y)
    {
        const std::size_t sourceOffset =
            static_cast<std::size_t>(gWindowHeight - 1 - y) * rowSize;
        const std::size_t destinationOffset = static_cast<std::size_t>(y) * rowSize;
        std::copy_n(
            openGlPixels.data() + sourceOffset,
            rowSize,
            pngPixels.data() + destinationOffset);
    }

    CGDataProviderRef provider = CGDataProviderCreateWithData(
        nullptr,
        pngPixels.data(),
        imageSize,
        nullptr);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    if (provider == nullptr || colorSpace == nullptr)
    {
        if (provider != nullptr)
        {
            CGDataProviderRelease(provider);
        }
        if (colorSpace != nullptr)
        {
            CGColorSpaceRelease(colorSpace);
        }
        std::cerr << "Failed to create PNG image data.\n";
        return false;
    }

    CGImageRef image = CGImageCreate(
        static_cast<std::size_t>(gWindowWidth),
        static_cast<std::size_t>(gWindowHeight),
        8,
        24,
        rowSize,
        colorSpace,
        kCGBitmapByteOrderDefault,
        provider,
        nullptr,
        false,
        kCGRenderingIntentDefault);

    const std::string pathString = outputPath.string();
    CFURLRef outputUrl = CFURLCreateFromFileSystemRepresentation(
        nullptr,
        reinterpret_cast<const UInt8*>(pathString.c_str()),
        static_cast<CFIndex>(pathString.size()),
        false);

    CGImageDestinationRef destination = nullptr;
    if (image != nullptr && outputUrl != nullptr)
    {
        destination = CGImageDestinationCreateWithURL(
            outputUrl,
            CFSTR("public.png"),
            1,
            nullptr);
    }

    bool saved = false;
    if (destination != nullptr)
    {
        CGImageDestinationAddImage(destination, image, nullptr);
        saved = CGImageDestinationFinalize(destination);
    }

    if (destination != nullptr)
    {
        CFRelease(destination);
    }
    if (outputUrl != nullptr)
    {
        CFRelease(outputUrl);
    }
    if (image != nullptr)
    {
        CGImageRelease(image);
    }
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);

    if (!saved)
    {
        std::cerr << "Failed to save PNG image: " << outputPath << '\n';
    }

    return saved;
#endif
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    const int plotWidth = gWindowWidth / kPlotColumns;
    const int plotHeight = gWindowHeight / kPlotRows;

    for (std::size_t index = 0; index < kLineCases.size(); ++index)
    {
        const int column = static_cast<int>(index) % kPlotColumns;
        const int rowFromTop = static_cast<int>(index) / kPlotColumns;
        const int rowFromBottom = kPlotRows - 1 - rowFromTop;

        glViewport(
            column * plotWidth,
            rowFromBottom * plotHeight,
            plotWidth,
            plotHeight);

        drawPlot(kLineCases[index]);
    }

    // 第一次完成整张大图后自动保存当前后缓冲区
    if (!gImageSaved)
    {
        gImageSaved = saveFramebufferAsPng(kOutputImagePath);
        if (gImageSaved)
        {
            std::cout << "Saved image to " << kOutputImagePath << '\n';
        }
    }

    glutSwapBuffers();
}

void reshape(int width, int height)
{
    gWindowWidth = std::max(width, kPlotColumns);
    gWindowHeight = std::max(height, kPlotRows);
    glutPostRedisplay();
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
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(kInitialWindowWidth, kInitialWindowHeight);
    glutInitWindowPosition(80, 80);
    glutCreateWindow("Wu Antialiasing for Different Slopes");

    initializeOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return EXIT_SUCCESS;
}
