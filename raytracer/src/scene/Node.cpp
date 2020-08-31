#include "Node.hpp"

#include <QDebug>
#include <glm/gtc/matrix_transform.hpp>

namespace Rt {

    Node::Node() {
        init();
    }

    Node::Node(std::shared_ptr<Mesh> child_mesh) {
        child_meshes.push_back(child_mesh);
        init();
    }

    Node::Node(const std::vector<std::shared_ptr<Node>>& child_nodes, const std::vector<std::shared_ptr<Mesh>>& child_meshes) :
        child_nodes(child_nodes),
        child_meshes(child_meshes)
    {
        init();
    }

    void Node::init() {
        transformation = glm::mat4(1.0f);
        translation = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
        rotation = glm::vec3(0.0f);

        node_type = NodeType::NODE;
        setObjectName("Node");
    }

    Node::~Node() {}

    Node::NodeType Node::get_node_type() const {
        return node_type;
    }

    // ===== Node hierarchy =====

    void Node::add_node(std::shared_ptr<Node> node) {
        child_nodes.push_back(node);
        emit added_child_node(node);
    }
    bool Node::remove_node(std::shared_ptr<Node> node) {
        return false;
    }

    void Node::add_mesh(std::shared_ptr<Mesh> mesh) {
        child_meshes.push_back(mesh);
        emit added_child_mesh(mesh);
    }
    bool Node::remove_mesh(std::shared_ptr<Mesh> mesh) {
        return false;
    }

    const std::vector<std::shared_ptr<Node>>& Node::get_child_nodes() const {
        return child_nodes;
    }
    std::vector<std::shared_ptr<Node>> Node::get_all_child_nodes() const {
        std::vector<std::shared_ptr<Node>> all_child_nodes;
        get_all_child_nodes(all_child_nodes);
        return all_child_nodes;
    }
    void Node::get_all_child_nodes(std::vector<std::shared_ptr<Node>>& all_child_nodes) const {
        const std::vector<std::shared_ptr<Node>>& current_child_nodes = get_child_nodes();
        all_child_nodes.insert(std::end(all_child_nodes), std::begin(current_child_nodes), std::end(current_child_nodes));
        for (auto child_node : current_child_nodes) {
            child_node->get_all_child_nodes(all_child_nodes);
        }
    }

    const std::vector<std::shared_ptr<Mesh>>& Node::get_child_meshes() const {
        return child_meshes;
    }
    std::vector<std::shared_ptr<Mesh>> Node::get_all_child_meshes() const {
        std::vector<std::shared_ptr<Mesh>> all_child_meshes;
        get_all_child_meshes(all_child_meshes);
        return all_child_meshes;
    }
    void Node::get_all_child_meshes(std::vector<std::shared_ptr<Mesh>>& all_child_meshes) const {
        const std::vector<std::shared_ptr<Mesh>>& current_child_meshes = get_child_meshes();
        all_child_meshes.insert(std::begin(all_child_meshes), std::begin(current_child_meshes), std::end(current_child_meshes));
        for (auto child_node : get_child_nodes()) {
            child_node->get_all_child_meshes(all_child_meshes);
        }
    }


    // ===== Transformations =====

    glm::vec3 Node::get_translation() const {
        return translation;
    }
    void Node::set_translation(const glm::vec3& new_translation) {
        translation = new_translation;
        emit translation_changed(translation);
    }

    glm::vec3 Node::get_euler_angle_rotation() const {
        return rotation;
    }
    void Node::set_rotation(const glm::vec3& new_euler_angle_rotation) {
        rotation = new_euler_angle_rotation;
        emit rotation_changed(rotation);
    }

    glm::vec3 Node::get_scale() const {
        return scale;
    }
    void Node::set_scale(const glm::vec3& new_scale) {
        scale = new_scale;
        emit scale_changed(scale);
    }

    glm::mat4 Node::get_transformation() const {
        glm::mat4 final_transformation = glm::translate(transformation, get_translation());
        glm::vec3 euler_angles_rotation = get_euler_angle_rotation();
        final_transformation = glm::rotate(final_transformation, euler_angles_rotation.x, glm::vec3(1.0f,0.0f,0.0f));
        final_transformation = glm::rotate(final_transformation, euler_angles_rotation.y, glm::vec3(0.0f,1.0f,0.0f));
        final_transformation = glm::rotate(final_transformation, euler_angles_rotation.z, glm::vec3(0.0f,0.0f,1.0f));
        final_transformation = glm::scale(final_transformation, get_scale());
        return final_transformation;
    }

    void Node::apply_transformations() {
        transformation = get_transformation();
        emit transformation_changed(transformation);
    }

}