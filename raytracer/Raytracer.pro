######################################################################
# Automatically generated by qmake (3.1) Sat Jun 27 17:34:32 2020
######################################################################

TEMPLATE = app
TARGET = Raytracer

QT += core gui widgets
CONFIG += debug
CONFIG += C++17

OBJECTS_DIR = generated_files
MOC_DIR = generated_files

INCLUDEPATH += . ./src/rendering ./src/materials ./src/scene

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS +=  src/MainWindow.hpp \
			src/Viewport.hpp \
			src/CameraController.hpp \
			src/rendering/OpenGLWidget.hpp \
			src/rendering/Renderer.hpp \
			src/rendering/Shader.hpp \
			src/rendering/Camera.hpp \
			src/rendering/Vertex.hpp \
			src/materials/Texture.hpp
			

SOURCES +=  src/main.cpp \
			src/MainWindow.cpp \
			src/Viewport.cpp \
			src/CameraController.cpp \
			src/rendering/OpenGLWidget.cpp \
			src/rendering/Renderer.cpp \
			src/rendering/Shader.cpp \
			src/rendering/Camera.cpp \
			src/rendering/Vertex.cpp \
			src/materials/Texture.cpp
