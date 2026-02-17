# 指定最低 CMake 版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(BattleSystem)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动收集所有源文件（.cpp 和 .h）
# 警告：file(GLOB) 在大型项目中可能不推荐，但对你现在够用
file(GLOB SOURCES "*.cpp" "*.h")
# file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")  # 如果你有子文件夹

# 生成可执行文件
add_executable(Result ${SOURCES})

# 如果有外部库（比如 pugiXML、SDL），需要链接
# 假设 pugiXML 的 .cpp 和 .h 就在项目里，直接包含即可，无需额外链接
# 如果是系统库或已安装的库，用 target_link_libraries