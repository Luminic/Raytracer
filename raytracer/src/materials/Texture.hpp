#ifndef RT_TEXTURES_HPP
#define RT_TEXTURES_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>
#include <vector>

#include "RaytracerGlobals.hpp"

namespace Rt {

    struct RAYTRACER_LIB_EXPORT TextureOptions {
        GLenum texture_type;
        GLenum internal_format;
        GLenum format;
        GLenum type;
        // Stores pairs of pname and param to be used with glTexParameter
        std::vector<std::pair<GLenum, GLenum>> options;

        static TextureOptions default_2D_options();
        static TextureOptions default_cube_map_options();
        // Sets texture wrapping for all dimenstions for 2D/3D textures
        void set_texture_wrap(GLenum wrap, bool three_d=false);
        // Sets texture interpolation for both min & mag filters
        void set_texture_interpolation(GLenum interpolation);
    };


    class RAYTRACER_LIB_EXPORT Texture : public QObject, protected QOpenGLFunctions_4_5_Core {
        Q_OBJECT;

    public:
        Texture(QObject* parent=nullptr);
        ~Texture();

        void load(const char* path, const TextureOptions& texture_options);
        void load(QImage img, const TextureOptions& texture_options);
        void create(unsigned int width, unsigned int height, const TextureOptions& texture_options);

        // Warning: This WILL clear the image
        void resize(unsigned int width, unsigned int height);

        unsigned int get_id();

    private:
        void set_params();

        TextureOptions tex_options;
        unsigned int id;
    };

}

#endif