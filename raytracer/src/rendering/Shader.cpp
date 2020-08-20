#include "Shader.hpp"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <vector>

#include <glm/gtc/type_ptr.hpp>

namespace Rt {

    QString text_content(const char* path) {
        QFile file(path);
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream in(&file);
        return in.readAll();
    }


    Shader::Shader(QObject* parent) : QObject(parent) {
        id = 0;
        gl = nullptr;
    }


    Shader::~Shader() {
        if (id) {
            gl->make_current();
            gl->glDeleteProgram(id);
        }
    }

    void Shader::initialize(OpenGLFunctions* gl) {
        this->gl = gl;
    }

    void Shader::load_shaders(ShaderStage shaders[], unsigned int nr_shaders) {
        gl->make_current();
        id = gl->glCreateProgram();
        std::vector<unsigned int> compiled_shaders; // Used for shader cleanup

        for (unsigned int i=0; i<nr_shaders; i++) {
            std::string c = text_content(shaders[i].path).toStdString();
            const char* shader_code = c.c_str();

            unsigned int shader = gl->glCreateShader(shaders[i].type);

            gl->glShaderSource(shader, 1, &shader_code, NULL);
            gl->glCompileShader(shader);

            int compiled;
            gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                int max_len;
                gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);

                std::vector<char> error_log(max_len);
                gl->glGetShaderInfoLog(shader, max_len, &max_len, error_log.data());

                std::string err_msg = "Shader Compilation Failed for ";
                err_msg += shaders[i].path;
                err_msg += ":\n";
                err_msg += std::string(error_log.begin(), error_log.begin()+max_len);
                qWarning(err_msg.c_str());

                gl->glDeleteShader(shader);
            } else {
                gl->glAttachShader(id, shader);
                compiled_shaders.push_back(shader);
            }
        }

        gl->glLinkProgram(id);

        // Detach and delete shaders after linking because they are no longer needed
        for (auto shader : compiled_shaders) {
            gl->glDetachShader(id, shader);
            gl->glDeleteShader(shader);
        }

        int linked;
        gl->glGetProgramiv(id, GL_LINK_STATUS, &linked);
        if (!linked) {
            int max_len;
            gl->glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_len);
        
            std::vector<char> error_log(max_len);
            gl->glGetProgramInfoLog(id, max_len, &max_len, error_log.data());

            std::string err_msg = "Shader Linking Failed";
            err_msg += std::string(error_log.begin(), error_log.begin()+max_len);
            qWarning(err_msg.c_str());
        }
    }


    bool Shader::validate() {
        gl->make_current();
        gl->glValidateProgram(id);

        int valid;
        gl->glGetProgramiv(id, GL_VALIDATE_STATUS, &valid);
        if (!valid) {
            int max_len;
            gl->glGetProgramiv(id, GL_INFO_LOG_LENGTH, &max_len);
            std::vector<char> error_log(max_len);
            gl->glGetProgramInfoLog(id, max_len, &max_len, error_log.data());
            qWarning(std::string(error_log.begin(), error_log.begin()+max_len).c_str());
        }
        return (bool) valid;
    }


    unsigned int Shader::get_id() const {
        return id;
    }


    void Shader::set_bool(const char* name, bool value) {
        unsigned int loc = gl->glGetUniformLocation(id, name);
        gl->glUniform1i(loc, (int)value);
    }


    void Shader::set_int(const char* name, int value) {
        unsigned int loc = gl->glGetUniformLocation(id, name);
        gl->glUniform1i(loc, value);
    }


    void Shader::set_float(const char* name, float value) {
        unsigned int loc = gl->glGetUniformLocation(id, name);
        gl->glUniform1f(loc, value);
    }


    void Shader::set_vec3(const char* name, const glm::vec3 &value) {
        unsigned int loc = gl->glGetUniformLocation(id, name);
        gl->glUniform3fv(loc, 1, &value[0]);
    }


    void Shader::set_mat4(const char* name, const glm::mat4 &value) {
        unsigned int loc = gl->glGetUniformLocation(id, name);
        gl->glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
    }

}