#include "Properties.hpp"

#include <QLineEdit>
#include <QFrame>
#include <QToolBox>
#include <QComboBox>
#include <QMetaEnum>
#include <QDebug>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene/lights/SunLight.hpp"
#include "settings/VectorView.hpp"

namespace Rt {

    NodeProperties::NodeProperties(Node* node, QWidget* parent) :
        QWidget(parent), node(node)
    {
        layout = new QGridLayout(this);

        QLineEdit* name = new QLineEdit(node->objectName(), this);
        layout->addWidget(name, 0,0);
        connect(node, &QWidget::objectNameChanged, name, &QLineEdit::setText);
        connect(name, &QLineEdit::editingFinished, this, [=](){node->setObjectName(name->text());});

        QFrame* line_divider = new QFrame(this);
        layout->addWidget(line_divider, 1,0);
        line_divider->setFrameShape(QFrame::HLine);

        QToolBox* properties = new QToolBox(this);
        layout->addWidget(properties, 2,0);

        QWidget* transformations = new QWidget(properties);
        QGridLayout* transformations_layout = new QGridLayout(transformations);

        transformations_layout->addWidget(new QLabel(tr("Pos"), transformations), 0, 0);
        Vector3View* translation_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        translation_view->set_value_glm(node->get_translation());
        connect(translation_view, &Vector3View::value_changed_glm, node, &Node::set_translation);
        connect(node, &Node::translation_changed, translation_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(translation_view, 0, 1);

        transformations_layout->addWidget(new QLabel(tr("Rot"), transformations), 1, 0);
        Vector3View* rotation_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        rotation_view->set_value_glm(node->get_euler_angle_rotation());
        connect(rotation_view, &Vector3View::value_changed_glm, node, &Node::set_rotation);
        connect(node, &Node::rotation_changed, rotation_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(rotation_view, 1, 1);

        transformations_layout->addWidget(new QLabel(tr("Sca"), transformations), 2, 0);
        Vector3View* scale_view = new Vector3View(QBoxLayout::Direction::Down, transformations);
        scale_view->set_value_glm(node->get_scale());
        connect(scale_view, &Vector3View::value_changed_glm, node, &Node::set_scale);
        connect(node, &Node::scale_changed, scale_view, &Vector3View::set_value_glm);
        transformations_layout->addWidget(scale_view, 2, 1);
        properties->addItem(transformations, tr("Transformations"));

        QWidget* visibility = new QWidget(properties);
        properties->addItem(visibility, tr("Visibility"));
    }

    LightProperties::LightProperties(AbstractLight* light, QWidget* parent) :
        QWidget(parent),
        light(light)
    {
        layout = new QGridLayout(this);

        QLineEdit* name = new QLineEdit(light->objectName(), this);
        layout->addWidget(name, 0,0);
        connect(light, &QWidget::objectNameChanged, name, &QLineEdit::setText);
        connect(name, &QLineEdit::editingFinished, this, [=](){light->setObjectName(name->text());});

        QFrame* line_divider = new QFrame(this);
        layout->addWidget(line_divider, 1,0);
        line_divider->setFrameShape(QFrame::HLine);

        QToolBox* properties = new QToolBox(this);
        layout->addWidget(properties, 2,0);

        QWidget* lighting = new QWidget(this);
        QGridLayout* lighting_layout = new QGridLayout(lighting);
        lighting_layout->addWidget(new QLabel(tr("Radiance"), lighting), 0, 0);
        Vector3View* radiance_view = new Vector3View(QBoxLayout::Direction::Down, lighting);
        radiance_view->set_value_glm(light->get_radiance());
        connect(radiance_view, &Vector3View::value_changed_glm, light, &AbstractLight::set_radiance);
        connect(light, &AbstractLight::radiance_changed, radiance_view, &Vector3View::set_value_glm);
        lighting_layout->addWidget(radiance_view, 0, 1);

        lighting_layout->addWidget(new QLabel(tr("Ambient"), lighting), 1, 0);
        QDoubleSpinBox* ambient_multiplier = new QDoubleSpinBox(lighting);
        ambient_multiplier->setDecimals(2);
        ambient_multiplier->setSingleStep(0.05);
        ambient_multiplier->setValue(light->get_ambient_multiplier());
        connect(ambient_multiplier, QOverload<double>::of(&QDoubleSpinBox::valueChanged), light, &AbstractLight::set_ambient_multiplier);
        connect(light, &AbstractLight::ambient_multiplier_changed, ambient_multiplier, &QDoubleSpinBox::setValue);
        lighting_layout->addWidget(ambient_multiplier, 1, 1);

        properties->addItem(lighting, tr("Lighting"));

        QWidget* visibility = new QWidget(this);
        QGridLayout* visibility_layout = new QGridLayout(visibility);
        visibility_layout->addWidget(new QLabel(tr("Visibility"), visibility), 0, 0);
        QComboBox* visibility_combo = new QComboBox(visibility);
        QMetaEnum v_meta_enum = QMetaEnum::fromType<AbstractLight::Visibility>();
        for (int i=0; i<v_meta_enum.keyCount(); i++) {
            visibility_combo->addItem(v_meta_enum.key(i));
        }
        visibility_combo->setCurrentIndex(light->get_visibility());
        connect(visibility_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){light->set_visibility((AbstractLight::Visibility)index);});
        connect(light, &AbstractLight::visibility_changed, visibility_combo, &QComboBox::setCurrentIndex);
        visibility_layout->addWidget(visibility_combo, 0, 1);

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
        selected_mesh = nullptr;
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

        if (node->get_node_type() == Node::NodeType::LIGHT) {
            light_selected(reinterpret_cast<AbstractLight*>(node));
        } else {
            light_selected(nullptr);
        }
    }

    void Properties::mesh_selected(Mesh* mesh) {
        selected_mesh = mesh;
    }

    void Properties::light_selected(AbstractLight* light) {
        selected_light = light;
        QLayoutItem* layoutitem = tab_layouts[LIGHT]->takeAt(0);
        if (layoutitem) {
            delete layoutitem->widget();
            delete layoutitem;
        }
        if (light) {
            tab_layouts[LIGHT]->addWidget(new LightProperties(light, this));
        }
    }

}