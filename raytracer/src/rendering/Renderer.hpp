#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "RaytracerGlobals.hpp"

#include "AbstractCamera.hpp"
#include "Shader.hpp"
#include "materials/Texture.hpp"
#include "scene/Vertex.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT Renderer : public QObject, protected QOpenGLFunctions_4_5_Core {
        Q_OBJECT;

    public:
        Renderer(QObject* parent=nullptr);
        ~Renderer();
        void initialize(unsigned int width, unsigned int height);
        void resize(unsigned int width, unsigned int height);

        Texture* render();

        void set_camera(AbstractCamera* new_camera);
        AbstractCamera* get_camera();

    private:
        AbstractCamera* camera;

        Shader render_shader;
        int work_group_size[3];
        Texture render_result;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        unsigned int vertex_ssbo;
        unsigned int index_ssbo;

        unsigned int width;
        unsigned int height;
    };

}

#endif