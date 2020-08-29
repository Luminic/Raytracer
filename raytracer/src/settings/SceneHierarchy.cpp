#include "SceneHierarchy.hpp"

#include <QDebug>

namespace Rt {

    SceneHierarchy::SceneHierarchy(QWidget* parent) : QTreeView(parent) {
        node_model = new QStandardItemModel(this);
        node_model->setHorizontalHeaderLabels(QStringList(tr("Scene Hierarchy")));
        setModel(node_model);

        connect(this, &SceneHierarchy::clicked, this, 
            [=](const QModelIndex &index) {
                QStandardItem* item = node_model->itemFromIndex(index);
                QVariant data = item->data();
                if (data.canConvert<Node*>()) {
                    select_node(data.value<Node*>());
                }
            }
        );

        selected_node = nullptr;
    }

    void SceneHierarchy::add_scene(Scene* scene) {
        node_model->appendRow(traverse_node_tree(scene));
    }

    bool SceneHierarchy::select_node(Node* node) {
        // TODO: Fail (return `false`) if node not in scene hierarchy
        
        // In case the selected node is deleted, clear selection
        connect(node, &Node::destroyed, this, [=](){select_node(nullptr);});

        selected_node = node;
        emit node_selected(node);
        return true;
    }

    QStandardItem* find_item_in_rows_from_value(QStandardItem* item, void* value) {
        int row = 0;
        QStandardItem* row_item = nullptr;
        do {
            row_item = item->child(row);
            if (row_item->data().value<void*>() == value) {
                return row_item;
            }
            row++;
        } while (row_item != nullptr);
        return nullptr;
    }

    QStandardItem* SceneHierarchy::create_item(QObject* data) {
        QStandardItem* item = new QStandardItem(data->objectName());
        connect(data, &QObject::objectNameChanged, this, [=](const QString& n){item->setText(n);});
        item->setEditable(false);

        QVariant item_data;
        item_data.setValue(data);
        item->setData(item_data);
        return item;
    }

    QStandardItem* SceneHierarchy::traverse_node_tree(Node* node) {
        QStandardItem* item = create_item(node);

        for (auto& child_node : node->get_child_nodes()) {
            item->appendRow(traverse_node_tree(child_node.get()));
        }

        for (auto& child_mesh : node->get_child_meshes()) {
            item->appendRow(create_item(child_mesh.get()));
        }

        connect(node, &Node::added_child_node, this, [=](std::shared_ptr<Node> child_node){
            item->appendRow(traverse_node_tree(child_node.get()));
        });

        connect(node, &Node::removed_child_node, this, [=](std::shared_ptr<Node> child_node){
            QStandardItem* corresponding_item = find_item_in_rows_from_value(item, child_node.get());
            if (corresponding_item) {
                item->removeRow(corresponding_item->row());
            } else {
                qWarning("Failed to find removed node from Properties node tree");
            }
        });

        connect(node, &Node::added_child_mesh, this, [=](std::shared_ptr<Mesh> child_mesh){
            item->appendRow(create_item(child_mesh.get()));
        });

        connect(node, &Node::removed_child_mesh, this, [=](std::shared_ptr<Mesh> child_mesh){
            QStandardItem* corresponding_item = find_item_in_rows_from_value(item, child_mesh.get());
            if (corresponding_item) {
                item->removeRow(corresponding_item->row());
            } else {
                qWarning("Failed to find removed mesh from Properties node tree");
            }
        });

        return item;
    }

}