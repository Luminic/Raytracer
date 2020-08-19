#ifndef RT_OPENGL_FUNCTIONS
#define RT_OPENGL_FUNCTIONS

#include <QOpenGLFunctions_4_5_Core>
#include "RaytracerGlobals.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT OpenGLFunctions : public QObject, public QOpenGLFunctions_4_5_Core {
        Q_OBJECT;
    
    public:
        OpenGLFunctions() : ctx(nullptr), surf(nullptr) {}
        OpenGLFunctions(QOpenGLContext* ctx, QSurface* surf) : ctx(ctx), surf(surf) {}

        bool make_current() {return ctx->makeCurrent(surf);}
        void done_current() {ctx->doneCurrent();}

    private:
        QOpenGLContext* ctx;
        QSurface* surf;
    };

}

#endif