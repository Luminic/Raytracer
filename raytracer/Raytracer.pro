######################################################################
# Automatically generated by qmake (3.1) Sat Jun 27 17:34:32 2020
######################################################################

TEMPLATE = lib
TARGET = Raytracer

DEFINES += MAKE_RAYTRACER_LIBRARY

QT += core gui widgets
CONFIG += debug
CONFIG += C++17

OBJECTS_DIR = generated_files
MOC_DIR = generated_files

RESOURCES = resources.qrc
INCLUDEPATH += . ./src

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS +=  src/RaytracerGlobals.hpp \
			src/rendering/OpenGLWidget.hpp \
			src/rendering/OpenGLFunctions.hpp \
			src/rendering/Renderer.hpp \
			src/rendering/Shader.hpp \
			src/rendering/AbstractCamera.hpp \
			src/scene/Scene.hpp \
			src/scene/Node.hpp \
			src/scene/Mesh.hpp \
			src/scene/Vertex.hpp \
			src/materials/MaterialManager.hpp \
			src/materials/Material.hpp \
			src/materials/Texture.hpp \
			src/settings/Properties.hpp \
			src/settings/SceneHierarchy.hpp \
			src/settings/VectorView.hpp
			

SOURCES +=  src/rendering/OpenGLWidget.cpp \
			src/rendering/Renderer.cpp \
			src/rendering/Shader.cpp \
			src/rendering/AbstractCamera.cpp \
			src/scene/Scene.cpp \
			src/scene/Node.cpp \
			src/scene/Mesh.cpp \
			src/scene/Vertex.cpp \
			src/materials/MaterialManager.cpp \
			src/materials/Material.cpp \
			src/materials/Texture.cpp \
			src/settings/Properties.cpp \
			src/settings/SceneHierarchy.cpp \
			src/settings/VectorView.cpp
