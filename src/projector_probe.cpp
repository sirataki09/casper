#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
struct Options
{
    int x = 1920;
    int y = 0;
    int width = 1024;
    int height = 768;
    int seconds = 0;
    bool fullscreen = true;
};

void print_usage()
{
    std::cout
        << "Usage: projector_probe.exe [options]\n"
        << "  --x N          top-left display coordinate (default: 1920)\n"
        << "  --y N          top-left display coordinate (default: 0)\n"
        << "  --width N      output width (default: 1024)\n"
        << "  --height N     output height (default: 768)\n"
        << "  --seconds N    close automatically after N seconds; 0 waits for a key\n"
        << "  --windowed     do not switch the output window to fullscreen\n"
        << "  --help         show this message\n";
}

int parse_int(const char *name, const char *value)
{
    try
    {
        return std::stoi(value);
    }
    catch (const std::exception &)
    {
        throw std::runtime_error(std::string("Invalid value for ") + name + ": " + value);
    }
}

Options parse_options(int argc, char **argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        auto next_value = [&](const char *name) -> const char *
        {
            if (i + 1 >= argc)
                throw std::runtime_error(std::string("Missing value for ") + name);
            return argv[++i];
        };

        if (arg == "--x")
            options.x = parse_int("--x", next_value("--x"));
        else if (arg == "--y")
            options.y = parse_int("--y", next_value("--y"));
        else if (arg == "--width")
            options.width = parse_int("--width", next_value("--width"));
        else if (arg == "--height")
            options.height = parse_int("--height", next_value("--height"));
        else if (arg == "--seconds")
            options.seconds = parse_int("--seconds", next_value("--seconds"));
        else if (arg == "--windowed")
            options.fullscreen = false;
        else if (arg == "--help" || arg == "-h")
        {
            print_usage();
            std::exit(0);
        }
        else
            throw std::runtime_error("Unknown option: " + arg);
    }
    if (options.width <= 0 || options.height <= 0 || options.seconds < 0)
        throw std::runtime_error("Width and height must be positive; seconds cannot be negative.");
    return options;
}

cv::Mat make_checkerboard(int width, int height)
{
    cv::Mat image(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
    const int cells_x = 16;
    const int cells_y = 12;
    const int cell_width = std::max(1, width / cells_x);
    const int cell_height = std::max(1, height / cells_y);
    for (int row = 0; row < cells_y; ++row)
    {
        for (int column = 0; column < cells_x; ++column)
        {
            if ((row + column) % 2 == 0)
                continue;
            cv::rectangle(image,
                          cv::Rect(column * cell_width,
                                   row * cell_height,
                                   std::min(cell_width, width - column * cell_width),
                                   std::min(cell_height, height - row * cell_height)),
                          cv::Scalar(0, 0, 0),
                          cv::FILLED);
        }
    }
    cv::line(image, cv::Point(width / 2, 0), cv::Point(width / 2, height - 1), cv::Scalar(0, 0, 255), 3);
    cv::line(image, cv::Point(0, height / 2), cv::Point(width - 1, height / 2), cv::Scalar(0, 0, 255), 3);
    cv::rectangle(image, cv::Rect(0, 0, width, height), cv::Scalar(0, 255, 0), 6);
    return image;
}

cv::Mat make_color_bars(int width, int height)
{
    cv::Mat image(height, width, CV_8UC3);
    const cv::Scalar colors[] = {
        {255, 255, 255}, {0, 255, 255}, {255, 255, 0}, {0, 255, 0},
        {255, 0, 255},   {0, 0, 255},   {255, 0, 0},   {0, 0, 0}};
    const int count = static_cast<int>(sizeof(colors) / sizeof(colors[0]));
    for (int index = 0; index < count; ++index)
    {
        const int left = width * index / count;
        const int right = width * (index + 1) / count;
        cv::rectangle(image, cv::Rect(left, 0, right - left, height), colors[index], cv::FILLED);
    }
    return image;
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        const std::string window_name = "Casper projector preflight";
        cv::namedWindow(window_name, cv::WINDOW_NORMAL);
        cv::moveWindow(window_name, options.x, options.y);
        cv::resizeWindow(window_name, options.width, options.height);
        cv::waitKey(100);
        if (options.fullscreen)
            cv::setWindowProperty(window_name, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

        const cv::Mat checkerboard = make_checkerboard(options.width, options.height);
        const cv::Mat color_bars = make_color_bars(options.width, options.height);
        const cv::Mat white(options.height, options.width, CV_8UC3, cv::Scalar(255, 255, 255));
        const cv::Mat black(options.height, options.width, CV_8UC3, cv::Scalar(0, 0, 0));
        const cv::Mat *current = &checkerboard;

        std::cout << "Output origin: (" << options.x << ", " << options.y << ")\n"
                  << "Output size: " << options.width << 'x' << options.height << '\n'
                  << "Controls: 1=checkerboard, 2=color bars, 3=white, 4=black, Q/Esc=quit\n";

        const auto start = std::chrono::steady_clock::now();
        while (true)
        {
            cv::imshow(window_name, *current);
            const int key = cv::waitKey(16) & 0xff;
            if (key == 27 || key == 'q' || key == 'Q')
                break;
            if (key == '1')
                current = &checkerboard;
            else if (key == '2')
                current = &color_bars;
            else if (key == '3')
                current = &white;
            else if (key == '4')
                current = &black;

            if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1)
                break;
            if (options.seconds > 0)
            {
                const double elapsed = std::chrono::duration<double>(
                                           std::chrono::steady_clock::now() - start)
                                           .count();
                if (elapsed >= options.seconds)
                    break;
            }
        }
        cv::destroyAllWindows();
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        print_usage();
        return 1;
    }
}
