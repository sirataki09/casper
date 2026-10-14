cmake_policy(SET CMP0167 NEW)

project(casper VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(CASPER_MACOS_SIMULATION "Build without Windows-only hardware SDKs" ON)
if(NOT CASPER_MACOS_SIMULATION)
    message(FATAL_ERROR "The native hardware build is not available on macOS. Configure with CASPER_MACOS_SIMULATION=ON.")
endif()

if(NOT DEFINED Python_EXECUTABLE)
    if(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/bin/python")
        set(Python_EXECUTABLE "$ENV{CONDA_PREFIX}/bin/python" CACHE FILEPATH "Python from the active Conda environment")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/dm/bin/python")
        set(Python_EXECUTABLE "${CMAKE_SOURCE_DIR}/dm/bin/python" CACHE FILEPATH "Python from the dm virtual environment")
    endif()
endif()

find_package(Python 3.11 COMPONENTS Interpreter Development NumPy REQUIRED)
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE CASPER_PYTHON_SITE_PACKAGES
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
if(NOT EXISTS "${CASPER_PYTHON_SITE_PACKAGES}")
    message(FATAL_ERROR "Python site-packages directory does not exist: ${CASPER_PYTHON_SITE_PACKAGES}")
endif()
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs video geometry calib)
if(OpenCV_VERSION VERSION_LESS 4.6)
    message(FATAL_ERROR "Casper requires OpenCV 4.6 or newer; found ${OpenCV_VERSION}")
endif()
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED)

add_executable(casper
    src/main.cpp
    src/display.cpp
    src/camera.cpp
    src/gl_camera.cpp
    src/text.cpp
    src/timer.cpp
    src/leapCPP_stub.cpp
    src/texture.cpp
    src/diffuse.cpp
    src/base64.cpp
    src/skinned_model.cpp
    src/skinned_shader.cpp
    src/post_process.cpp
    src/utils.cpp
    src/cnpy.cpp
    src/point_cloud.cpp
    src/fbo.cpp
    src/quad.cpp
    src/helpers.cpp
    src/kalman.cpp
    src/grid.cpp
    src/user_study.cpp
    src/MidiControllerAPI_stub.cpp
    src/guess_pose_game.cpp
    src/guess_char_game.cpp
    src/guess_animal_game.cpp
    src/dear_widgets.cpp
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_tables.cpp
    third-party/imgui/imgui_widgets.cpp
    third-party/imgui/imgui_demo.cpp
    third-party/imgui/backends/imgui_impl_glfw.cpp
    third-party/imgui/backends/imgui_impl_opengl3.cpp
    third-party/imgui/misc/cpp/imgui_stdlib.cpp
    third-party/glad4.3/src/glad.c
)

target_compile_definitions(casper PRIVATE
    CASPER_SIMULATION_ONLY=1
    CASPER_PYTHON_SITE_PACKAGES="${CASPER_PYTHON_SITE_PACKAGES}"
    GL_SILENCE_DEPRECATION=1
)

target_include_directories(casper PRIVATE
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/third-party/glad4.3/include"
    "${PROJECT_SOURCE_DIR}/third-party/glm"
    "${PROJECT_SOURCE_DIR}/third-party/stb"
    "${PROJECT_SOURCE_DIR}/third-party/imgui"
    "${PROJECT_SOURCE_DIR}/third-party/imgui/backends"
    "${PROJECT_SOURCE_DIR}/third-party/imgui/misc/cpp"
    "${PROJECT_SOURCE_DIR}/third-party/libicp"
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(casper PRIVATE
    ${OpenCV_LIBS}
    OpenGL::GL
    glfw
    assimp::assimp
    Freetype::Freetype
    ZLIB::ZLIB
    Python::Python
    Python::NumPy
)

add_subdirectory("${PROJECT_SOURCE_DIR}/third-party/NVTX-3.1.0-c-cpp")
target_link_libraries(casper PRIVATE nvtx3-cpp)

add_custom_command(TARGET casper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/src/predict.py"
            "$<TARGET_FILE_DIR:casper>/predict.py"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/src/chatgpt.py"
            "$<TARGET_FILE_DIR:casper>/chatgpt.py"
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PROJECT_SOURCE_DIR}/debug/video"
)

message(STATUS "Configuring Casper macOS simulation build")
message(STATUS "Python: ${Python_EXECUTABLE}")
message(STATUS "Python packages: ${CASPER_PYTHON_SITE_PACKAGES}")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
