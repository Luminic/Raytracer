#include "Texture.hpp"

#include <QImage>

namespace Rt {

    TextureOptions TextureOptions::default_2D_options() {
        return TextureOptions{
            GL_TEXTURE_2D,
            GL_RGBA32F,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
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

    void Texture::load(const char* path, const TextureOptions& tex_options) {
        QImage img = QImage(path).convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
        load(img, tex_options);
    }

    void Texture::load(QImage img, const TextureOptions& tex_options) {
        initializeOpenGLFunctions();
        this->tex_options = tex_options;

        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_options.texture_type, id);

        set_params();
        glTexImage2D(tex_options.texture_type, 0, tex_options.internal_format, img.width(), img.height(), 0, tex_options.format, tex_options.type, img.bits());
    }

    void Texture::create(unsigned int width, unsigned int height, const TextureOptions& texture_options) {
        initializeOpenGLFunctions();
        this->tex_options = texture_options;

        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_options.texture_type, id);

        set_params();
        glTexImage2D(tex_options.texture_type, 0, tex_options.internal_format, width, height, 0, tex_options.format, tex_options.type, nullptr);
    }

    void Texture::set_params() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_options.texture_type, id);

        for (auto options_pair : tex_options.options) {
            glTextureParameteri(id, options_pair.first, options_pair.second);
        }
    }

    void Texture::resize(unsigned int width, unsigned int height) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tex_options.texture_type, id);
        glTexImage2D(tex_options.texture_type, 0, tex_options.internal_format, width, height, 0, tex_options.format, tex_options.type, nullptr);
    }

    unsigned int Texture::get_id() {
        return id;
    }

}