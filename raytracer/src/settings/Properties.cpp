#include "Properties.hpp"

#include <QLineEdit>
#include <QFrame>
#include <QToolBox>
#include <QDebug>

#include "scene/Node.hpp"

namespace Rt {

    NodeProperties::NodeProperties(Node* node, QWidget* parent) :
        QWidget(parent), node(node)
    {
        layout = new QGridLayout(this);

        QLineEdit* name = new QLineEdit(node->objectName(), this);
        layout->addWidget(name, 0,0);
        connect(node, &Node::objectNameChanged, name, &QLineEdit::setText);
        connect(name, &QLineEdit::textEdited, node, &Node::setObjectName);

        QFrame* line_divider = new QFrame(this);
        layout->addWidget(line_divider, 1,0);
        line_divider->setFrameShape(QFrame::HLine);

        QToolBox* properties = new QToolBox(this);
        layout->addWidget(properties, 2,0);

        QWidget* transformations = new QWidget(properties);
        properties->addItem(transformations, tr("Transformations"));

        QWidget* visibility = new QWidget(properties);
        properties->addItem(visibility, tr("Visibility"));
    }


    Properties::Properties(QWidget* parent) : QTabWidget(parent) {
        setTabPosition(QTabWidget::West);

        for (int i=0; i<nr_tabs; i++) {
            QWidget* container_tab = new QWidget(this);
            QVBoxLayout* container_layout = new QVBoxLayout(container_tab);
            tab_layouts[i] = container_layout;
            addTab(container_tab, tab_names[i]);
        }

        selected_scene = nullptr;
        selected_node = nullptr;
        selected_scene = nullptr;
    }

    void Properties::scene_selected(Scene* scene) {
        selected_scene = scene;
    }

    void Properties::node_selected(Node* node) {
        selected_node = node;
        delete tab_layouts[NODE]->takeAt(0);
        if (node)
            tab_layouts[NODE]->addWidget(new NodeProperties(node, this));
    }

    void Properties::mesh_selected(Mesh* mesh) {
        selected_mesh = mesh;
    }


}