#ifndef RT_PROPERTIES_HPP
#define RT_PROPERTIES_HPP

#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTreeView>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QLabel>

#include "RaytracerGlobals.hpp"
#include "settings/SceneHierarchy.hpp"
#include "scene/Scene.hpp"
#include "scene/Node.hpp"
#include "scene/lights/AbstractLight.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT MaterialProperties : public QWidget {
        Q_OBJECT;

    public:
        MaterialProperties(QWidget* parent=nullptr);

    };

    class RAYTRACER_LIB_EXPORT NodeProperties : public QWidget {
        Q_OBJECT;

    public:
        NodeProperties(Node* node, QWidget* parent=nullptr);

    private:
        Node* node;
        QGridLayout* layout;

    };

    class RAYTRACER_LIB_EXPORT LightProperties : public QWidget {
        Q_OBJECT;

    public:
        LightProperties(AbstractLight* light, QWidget* parent=nullptr);

    private:
        AbstractLight* light;
        QGridLayout* layout;

    };
    
    class RAYTRACER_LIB_EXPORT Properties : public QTabWidget {
        Q_OBJECT;

        public:
            Properties(QWidget* parent=nullptr);

            void scene_selected(Scene* scene);
            void node_selected(Node* node);
            void mesh_selected(Mesh* mesh);
            void light_selected(AbstractLight* light);

        private:
            Scene* selected_scene;
            Node* selected_node;
            Mesh* selected_mesh;
            AbstractLight* selected_light;

            static constexpr int nr_tabs = 3;
            const QString tab_names[nr_tabs] = {"Scene", "Node", "Light"};
            enum TabTypeIndex {
                SCENE = 0,
                NODE = 1,
                LIGHT = 2
            };
            // Array of widget pointers
            QVBoxLayout* tab_layouts[nr_tabs];

    };

}

#endif