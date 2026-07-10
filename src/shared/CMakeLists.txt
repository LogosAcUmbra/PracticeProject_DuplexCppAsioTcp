add_library(Shared STATIC
    src/placeholder.cpp
)

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# expose the include folder to targets linking against this library
target_include_directories(Shared PUBLIC include)

target_link_libraries(Shared PUBLIC 
    fmt::fmt
    spdlog::spdlog
    Boost::asio
    Boost::system
)

target_compile_definitions(Shared PUBLIC
    _WIN32_WINNT=0x0A00
    SPDLOG_FMT_EXTERNAL
    BOOST_ASIO_NO_DEPRECATED
)