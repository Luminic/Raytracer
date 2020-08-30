#ifndef RT_VECTOR_VIEW
#define RT_VECTOR_VIEW

#include <QWidget>
#include <QDoubleSpinBox>
#include <QBoxLayout>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "RaytracerGlobals.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT VectorView : public QWidget {
        Q_OBJECT;
    
    public:
        VectorView(int n, QBoxLayout::Direction dir, QWidget* parent=nullptr);
        virtual ~VectorView();

        virtual int dimensions() const;

        virtual void set_value(const float* value);
        virtual void set_value_at(float value, int i);

        virtual const float* value() const;

        virtual void set_decimals(int dec);
        virtual int decimals() const;
    
    signals:
        void value_changed(const float* value, int n);
    
    private:
        const int n;
        float* data;

        QDoubleSpinBox* spinboxes;
        int spinbox_decimals;
    };


    // Convenience class to interface with glm::vec3 functions nicely
    class RAYTRACER_LIB_EXPORT Vector3View : public VectorView {
        Q_OBJECT;

    public:
        Vector3View(QBoxLayout::Direction dir, QWidget* parent=nullptr) :
            VectorView(3, dir, parent)
        {
            connect(this, &VectorView::value_changed, this, [=](const float* value, int n){
                Q_UNUSED(n);
                emit value_changed_glm(glm::make_vec3(value));
            });
        }

        virtual ~Vector3View() {}

        virtual void set_value_glm(const glm::vec3& value) {set_value(glm::value_ptr(value));}
    
    signals:
        virtual void value_changed_glm(const glm::vec3& value);
    };


    // Convenience class to interface with glm::vec4 functions nicely
    class RAYTRACER_LIB_EXPORT Vector4View : public VectorView {
        Q_OBJECT;

    public:
        Vector4View(QBoxLayout::Direction dir, QWidget* parent=nullptr) :
            VectorView(4, dir, parent)
        {
            connect(this, &VectorView::value_changed, this, [=](const float* value, int n){
                Q_UNUSED(n);
                emit value_changed_glm(glm::make_vec4(value));
            });
        }

        virtual ~Vector4View() {}

        virtual void set_value_glm(const glm::vec4& value) {set_value(glm::value_ptr(value));}
    
    signals:
        virtual void value_changed_glm(const glm::vec4& value);
    };

}

#endif