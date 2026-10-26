cmake_minimum_required(VERSION 3.16)

project(Calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# moc - генератор кода Qt, нужен для классов с макросом Q_OBJECT
set(CMAKE_AUTOMOC ON)

# подойдёт и Qt 6, и Qt 5 - что найдётся
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# WIN32 - чтобы в Windows вместе с окном не открывалась консоль
add_executable(calculator WIN32
    src/main.cpp
    src/Calculator.h
    src/Calculator.cpp
)

target_link_libraries(calculator PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# для компилятора Microsoft: исходники в кодировке UTF-8 (русские надписи)
if(MSVC)
    target_compile_options(calculator PRIVATE /utf-8)
endif()
