#include "Properties.hpp"

#include <QLineEdit>
#include <QFrame>
#include <QToolBox>
#include <QDebug>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene/Node.hpp"
#include "settings/VectorView.hpp"

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
        QGridLayout* transformations_layout = new QGridLayout(transformations);

        qDebug() << node;

        transformations_layout->addWidget(new QLabel(tr("Pos")), 0, 0);
        Vector3View* translation_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        translation_view->set_value_glm(node->get_translation());
        connect(translation_view, &Vector3View::value_changed_glm, node, &Node::set_translation);
        connect(node, &Node::translation_changed, translation_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(translation_view, 0, 1);

        transformations_layout->addWidget(new QLabel(tr("Rot")), 1, 0);
        Vector3View* rotation_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        rotation_view->set_value_glm(node->get_euler_angle_rotation());
        connect(rotation_view, &Vector3View::value_changed_glm, node, &Node::set_rotation);
        connect(node, &Node::rotation_changed, rotation_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(rotation_view, 1, 1);

        transformations_layout->addWidget(new QLabel(tr("Sca")), 2, 0);
        Vector3View* scale_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        scale_view->set_value_glm(node->get_scale());
        connect(scale_view, &Vector3View::value_changed_glm, node, &Node::set_scale);
        connect(node, &Node::scale_changed, scale_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(scale_view, 2, 1);
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
        QLayoutItem* layoutitem = tab_layouts[NODE]->takeAt(0);
        if (layoutitem) {
            delete layoutitem->widget();
            delete layoutitem;
        }
        if (node) {
            tab_layouts[NODE]->addWidget(new NodeProperties(node, this));
        }
    }

    void Properties::mesh_selected(Mesh* mesh) {
        selected_mesh = mesh;
    }


}