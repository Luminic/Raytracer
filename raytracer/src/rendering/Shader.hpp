#ifndef RT_SHADER_HPP
#define RT_SHADER_HPP

#include <QObject>

#include <glm/glm.hpp>

#include "RaytracerGlobals.hpp"
#include "rendering/OpenGLFunctions.hpp"

namespace Rt {

    struct ShaderStage {
        GLenum type;
        const char* path;
    };


    class Shader : public QObject {
        Q_OBJECT;

    public:
        Shader(QObject* parent=nullptr);
        ~Shader();

        void initialize(OpenGLFunctions* gl);
        void load_shaders(ShaderStage shaders[], unsigned int nr_shaders);
        bool validate();

        unsigned int get_id() const;

        // The following functions assume the opengl context is already current
        void set_bool(const char* name, bool value);
        void set_int(const char* name, int value);
        void set_uint(const char* name, unsigned int value);
        void set_float(const char* name, float value);
        void set_vec3(const char* name, const glm::vec3 &value);
        void set_mat4(const char* name, const glm::mat4 &value);

    private:
        unsigned int id;
        OpenGLFunctions* gl;
    };

}

#endif