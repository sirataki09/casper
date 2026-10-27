#include <librealsense2/rs.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
struct Options
{
    int width = 848;
    int height = 480;
    int fps = 30;
    int stream_index = 1;
    int max_frames = 0;
    bool disable_emitter = false;
    bool list_only = false;
    std::string save_path = "d455_ir.png";
};

void print_usage()
{
    std::cout
        << "Usage: realsense_probe.exe [options]\n"
        << "  --width N            IR stream width (default: 848)\n"
        << "  --height N           IR stream height (default: 480)\n"
        << "  --fps N              IR stream FPS (default: 30)\n"
        << "  --stream N           infrared stream index 1 or 2 (default: 1)\n"
        << "  --frames N           stop automatically after N frames\n"
        << "  --save PATH          path used when S is pressed\n"
        << "  --disable-emitter    turn off the D455 depth emitter when supported\n"
        << "  --list               list available infrared profiles and exit\n"
        << "  --help                show this message\n";
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

        if (arg == "--width")
            options.width = parse_int("--width", next_value("--width"));
        else if (arg == "--height")
            options.height = parse_int("--height", next_value("--height"));
        else if (arg == "--fps")
            options.fps = parse_int("--fps", next_value("--fps"));
        else if (arg == "--stream")
            options.stream_index = parse_int("--stream", next_value("--stream"));
        else if (arg == "--frames")
            options.max_frames = parse_int("--frames", next_value("--frames"));
        else if (arg == "--save")
            options.save_path = next_value("--save");
        else if (arg == "--disable-emitter")
            options.disable_emitter = true;
        else if (arg == "--list")
            options.list_only = true;
        else if (arg == "--help" || arg == "-h")
        {
            print_usage();
            std::exit(0);
        }
        else
            throw std::runtime_error("Unknown option: " + arg);
    }

    if (options.width <= 0 || options.height <= 0 || options.fps <= 0)
        throw std::runtime_error("Width, height, and FPS must be positive.");
    if (options.stream_index != 1 && options.stream_index != 2)
        throw std::runtime_error("--stream must be 1 or 2.");
    return options;
}

std::string device_label(const rs2::device &device)
{
    std::string label = "Unknown RealSense device";
    if (device.supports(RS2_CAMERA_INFO_NAME))
        label = device.get_info(RS2_CAMERA_INFO_NAME);
    if (device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
        label += " (serial " + std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)) + ")";
    return label;
}

void print_infrared_profiles(const rs2::device &device)
{
    std::cout << "Available infrared profiles:\n";
    for (const rs2::sensor &sensor : device.query_sensors())
    {
        for (const rs2::stream_profile &profile : sensor.get_stream_profiles())
        {
            if (profile.stream_type() != RS2_STREAM_INFRARED)
                continue;
            const rs2::video_stream_profile video = profile.as<rs2::video_stream_profile>();
            std::cout << "  index=" << profile.stream_index()
                      << " " << video.width() << "x" << video.height()
                      << " @ " << profile.fps() << " FPS"
                      << " format=" << rs2_format_to_string(profile.format()) << '\n';
        }
    }
}

bool set_emitter_enabled(const rs2::device &device, bool enabled)
{
    for (const rs2::sensor &sensor : device.query_sensors())
    {
        if (!sensor.supports(RS2_OPTION_EMITTER_ENABLED))
            continue;
        sensor.set_option(RS2_OPTION_EMITTER_ENABLED, enabled ? 1.0f : 0.0f);
        std::cout << "Emitter: " << (enabled ? "enabled" : "disabled") << '\n';
        return true;
    }
    std::cout << "Emitter control is not supported by this device.\n";
    return false;
}
} // namespace

int main(int argc, char **argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        rs2::context context;
        const rs2::device_list devices = context.query_devices();
        if (devices.size() == 0)
        {
            std::cerr << "No RealSense device was found. Connect the D455 directly to a USB 3 port.\n";
            return 2;
        }

        const rs2::device device = devices.front();
        std::cout << "Device: " << device_label(device) << '\n';
        print_infrared_profiles(device);
        if (options.list_only)
            return 0;

        if (options.disable_emitter)
            set_emitter_enabled(device, false);

        rs2::pipeline pipeline(context);
        rs2::config config;
        if (device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
            config.enable_device(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
        config.enable_stream(RS2_STREAM_INFRARED,
                             options.stream_index,
                             options.width,
                             options.height,
                             RS2_FORMAT_Y8,
                             options.fps);

        const rs2::pipeline_profile pipeline_profile = pipeline.start(config);
        const rs2::video_stream_profile active_profile =
            pipeline_profile.get_stream(RS2_STREAM_INFRARED, options.stream_index)
                .as<rs2::video_stream_profile>();
        std::cout << "Active stream: infrared " << options.stream_index << ' '
                  << active_profile.width() << 'x' << active_profile.height()
                  << " @ " << active_profile.fps() << " FPS, Y8\n"
                  << "Controls: Q or Esc = quit, S = save current frame, E = toggle emitter\n";

        const std::string window_name = "D455 infrared preflight";
        cv::namedWindow(window_name, cv::WINDOW_NORMAL);
        auto fps_start = std::chrono::steady_clock::now();
        std::uint64_t total_frames = 0;
        std::uint64_t interval_frames = 0;
        bool emitter_enabled = !options.disable_emitter;

        while (options.max_frames == 0 || total_frames < static_cast<std::uint64_t>(options.max_frames))
        {
            const rs2::frameset frames = pipeline.wait_for_frames(5000);
            const rs2::video_frame infrared = frames.get_infrared_frame(options.stream_index);
            if (!infrared)
            {
                std::cerr << "An infrared frame was not present in the frameset.\n";
                continue;
            }

            cv::Mat image(cv::Size(infrared.get_width(), infrared.get_height()),
                          CV_8UC1,
                          const_cast<void *>(infrared.get_data()),
                          cv::Mat::AUTO_STEP);
            cv::imshow(window_name, image);
            ++total_frames;
            ++interval_frames;

            const auto now = std::chrono::steady_clock::now();
            const double elapsed = std::chrono::duration<double>(now - fps_start).count();
            if (elapsed >= 1.0)
            {
                std::cout << "frame=" << infrared.get_frame_number()
                          << " device_timestamp_ms=" << std::fixed << std::setprecision(3)
                          << infrared.get_timestamp()
                          << " measured_fps=" << std::setprecision(1)
                          << static_cast<double>(interval_frames) / elapsed << '\n';
                fps_start = now;
                interval_frames = 0;
            }

            const int key = cv::waitKey(1) & 0xff;
            if (key == 27 || key == 'q' || key == 'Q')
                break;
            if (key == 's' || key == 'S')
            {
                if (cv::imwrite(options.save_path, image))
                    std::cout << "Saved: " << options.save_path << '\n';
                else
                    std::cerr << "Failed to save: " << options.save_path << '\n';
            }
            if (key == 'e' || key == 'E')
            {
                emitter_enabled = !emitter_enabled;
                if (!set_emitter_enabled(device, emitter_enabled))
                    emitter_enabled = !emitter_enabled;
            }

            if (cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) < 1)
                break;
        }

        pipeline.stop();
        cv::destroyAllWindows();
        std::cout << "Captured " << total_frames << " infrared frames.\n";
        return total_frames > 0 ? 0 : 3;
    }
    catch (const rs2::error &error)
    {
        std::cerr << "RealSense error while calling " << error.get_failed_function()
                  << "(" << error.get_failed_args() << "): " << error.what() << '\n';
        return 4;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        print_usage();
        return 1;
    }
}
