#include "Texture.hpp"

#include <QImage>

namespace Rt {

    TextureOptions TextureOptions::default_2D_options() {
        return TextureOptions{
            GL_TEXTURE_2D,
            GL_RGBA32F,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            1,
            {
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MIN_FILTER, GL_NEAREST),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER, GL_NEAREST)
            }
        };
    }

    TextureOptions TextureOptions::default_cube_map_options() {
        return TextureOptions{
            GL_TEXTURE_CUBE_MAP,
            GL_RGBA32F,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            1,
            {
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MIN_FILTER, GL_LINEAR),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            }
        };
    }

    void TextureOptions::set_texture_wrap(GLenum wrap, bool three_d) {
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, wrap));
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, wrap));

        if (three_d)
            options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_R, wrap));
    }

    void TextureOptions::set_texture_interpolation(GLenum interpolation) {
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_MIN_FILTER, interpolation));
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER, interpolation));
    }
    

    Texture::Texture(QObject* parent) : QObject(parent) {}

    Texture::~Texture() {
        glDeleteTextures(1, &id);
    }

    void Texture::initialize(OpenGLFunctions* gl) {
        this->gl = gl;
    }

    void Texture::load(const char* path, const TextureOptions& tex_options) {
        QImage img = QImage(path).convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
        load(img, tex_options);
    }

    void Texture::load(QImage img, const TextureOptions& tex_options) {
        create(img.width(), img.height(), tex_options);
        gl->glTextureSubImage2D(id, 0, 0, 0, img.width(), img.height(), tex_options.format, tex_options.type, img.constBits());
    }

    void Texture::create(unsigned int width, unsigned int height, const TextureOptions& texture_options) {
        gl->make_current();
        this->tex_options = texture_options;

        gl->glCreateTextures(tex_options.texture_type, 1, &id);

        set_params();
        gl->glTextureStorage2D(id, tex_options.levels, tex_options.internal_format, width, height);
    }

    void Texture::set_params() {
        for (auto options_pair : tex_options.options) {
            gl->glTextureParameteri(id, options_pair.first, options_pair.second);
        }
    }

    void Texture::resize(unsigned int width, unsigned int height) {
        gl->make_current();
        glDeleteTextures(1, &id);
        gl->glCreateTextures(tex_options.texture_type, 1, &id);
        gl->glTextureStorage2D(id, tex_options.levels, tex_options.internal_format, width, height);
        set_params();
    }

    unsigned int Texture::get_id() {
        return id;
    }

}