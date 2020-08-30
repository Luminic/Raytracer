#ifndef RT_NODE_HPP
#define RT_NODE_HPP

#include <QObject>
#include <vector>
#include <memory>

#include "RaytracerGlobals.hpp"
#include "scene/Mesh.hpp"

namespace Rt {

    class RAYTRACER_LIB_EXPORT Node : public QObject {
        Q_OBJECT;

    public:
        Node();
        Node(std::shared_ptr<Mesh> child_mesh);
        Node(const std::vector<std::shared_ptr<Node>>& child_nodes, const std::vector<std::shared_ptr<Mesh>>& child_meshes);
        virtual ~Node();

        // ===== Node hierarchy =====

        virtual void add_node(std::shared_ptr<Node> node);
        // Returns true if the node was found and removed
        // Returns false if the node was not found
        virtual bool remove_node(std::shared_ptr<Node> node);

        virtual void add_mesh(std::shared_ptr<Mesh> mesh);
        // Returns true if the node was found and removed
        // Returns false if the node was not found
        virtual bool remove_mesh(std::shared_ptr<Mesh> mesh);

        // Returns the node's immediate child nodes
        virtual const std::vector<std::shared_ptr<Node>>& get_child_nodes() const;
        // Returns all the node's child nodes (including grand children, great grand children, etc)
        virtual std::vector<std::shared_ptr<Node>> get_all_child_nodes() const;
        // Adds the child nodes from get_all_child_nodes() to all_child_nodes
        virtual void get_all_child_nodes(std::vector<std::shared_ptr<Node>>& all_child_nodes) const;

        // Returns the node's immediate child meshes
        virtual const std::vector<std::shared_ptr<Mesh>>& get_child_meshes() const;
        // Returns all the node's child meshes (including grand children, great grand children, etc)
        virtual std::vector<std::shared_ptr<Mesh>> get_all_child_meshes() const;
        // Adds the child meshes from get_all_child_meshes() to all_child_meshes
        virtual void get_all_child_meshes(std::vector<std::shared_ptr<Mesh>>& all_child_meshes) const;


        // ===== Transformations =====

        virtual glm::vec3 get_translation() const;
        virtual void set_translation(const glm::vec3& new_translation);

        virtual glm::vec3 get_euler_angle_rotation() const;
        virtual void set_rotation(const glm::vec3& new_euler_angle_rotation);

        virtual glm::vec3 get_scale() const;
        virtual void set_scale(const glm::vec3& new_scale);

        virtual glm::mat4 get_transformation() const;

        // Make transformation permananently equal the matrix from get_transformation
        // loc, rot, and scale vectors will be set to 0
        virtual void apply_transformations();

    signals:
        void translation_changed(const glm::vec3&);
        void rotation_changed(const glm::vec3&);
        void scale_changed(const glm::vec3&);
        void transformation_changed(const glm::mat4&);

        void added_child_node(std::shared_ptr<Node> node);
        void added_child_mesh(std::shared_ptr<Mesh> mesh);
        void removed_child_node(std::shared_ptr<Node> node);
        void removed_child_mesh(std::shared_ptr<Mesh> mesh);

    private:
        void init();

        std::vector<std::shared_ptr<Node>> child_nodes;
        std::vector<std::shared_ptr<Mesh>> child_meshes;

        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
        glm::mat4 transformation;
    };

}

#endif