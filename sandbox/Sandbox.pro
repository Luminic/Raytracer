######################################################################
# Automatically generated by qmake (3.1) Sat Jun 27 17:34:32 2020
######################################################################

TEMPLATE = app
TARGET = Sandbox

QT += core gui widgets
CONFIG += debug
CONFIG += C++17

OBJECTS_DIR = generated_files
MOC_DIR = generated_files

INCLUDEPATH += . ./src 

DEPENDPATH += ../raytracer ../raytracer/src
INCLUDEPATH += ../raytracer ../raytracer/src

QMAKE_LFLAGS += -Wl,-rpath,"$$PWD/../raytracer"
LIBS +=  -L../raytracer/ -lRaytracer


# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS +=  src/MainWindow.hpp \
			src/Viewport.hpp \
			src/Camera.hpp \
			src/CameraController.hpp
			

SOURCES +=  src/main.cpp \
			src/MainWindow.cpp \
			src/Viewport.cpp \
			src/Camera.cpp \
			src/CameraController.cpp