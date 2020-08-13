#include "Texture.hpp"

#include <QImage>

namespace Rt {

    Texture::Texture(QObject* parent) : QObject(parent) {}

    Texture::~Texture() {
        glDeleteTextures(1, &id);
    }

    void Texture::load(const char* path) {
        QImage img = QImage(path).convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
        load(img);
    }

    void Texture::load(QImage img) {
        initializeOpenGLFunctions();

        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);

        set_params();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    }

    void Texture::create(unsigned int width, unsigned int height) {
        initializeOpenGLFunctions();

        glGenTextures(1, &id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);

        set_params();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
    }

    void Texture::set_params() {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);

        glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void Texture::resize(unsigned int width, unsigned int height) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);
    }

    unsigned int Texture::get_id() {
        return id;
    }

}