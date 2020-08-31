#ifndef RT_SCENE_HIERARCHY_HPP
#define RT_SCENE_HIERARCHY_HPP

#include <QTreeView>
#include <QStandardItemModel>

#include "RaytracerGlobals.hpp"
#include "scene/Node.hpp"
#include "scene/Scene.hpp"

namespace Rt {
    // Constructs a node tree in parallel to the node tree in scene
    // This is because I don't want to clutter the node class with the
    // implementation of a QAbstractItemModel
    class RAYTRACER_LIB_EXPORT SceneHierarchy : public QTreeView {
        Q_OBJECT;
    
    public:
        SceneHierarchy(QWidget* parent=nullptr);

        void add_scene(Scene* scene);

        // Returns `true` and emits `node_selected` if node can be found in scene hierarchy
        // Otherwise returns `false` and nothing happens (not yet implemented: will always return `true`)
        // Pass in a nullptr to clear selection
        bool select_node(Node* node);
    
    signals:
        // Will also be emitted when scene is deselected with a nullptr as scene
        // void scene_selected(Scene* scene);
        void node_selected(Node* node);

    private:
        QStandardItemModel* node_model;
        QStandardItem* create_item(QObject* data);
        QStandardItem* traverse_node_tree(Node* node);

        Node* selected_node;
    };

}

#endif