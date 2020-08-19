#include "Renderer.hpp"
#include <QDebug>

namespace Rt {

    uint32_t round_up_to_pow_2(uint32_t x) {
        /*
        In C++20 we can use:
            #include <bit>
            std::bit_ceil(x)
        */
        // Current implementation from https://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        x++;
        return x;
    }

    Renderer::Renderer(QObject* parent) : QObject(parent) {
        camera = nullptr;
        scene = nullptr;
        prev_width = 0;
        prev_height = 0;
    }

    Renderer::~Renderer() {
        gl->make_current();
        gl->glDeleteBuffers(1, &vertex_ssbo);
        gl->glDeleteBuffers(1, &static_vertex_ssbo);
        gl->glDeleteBuffers(1, &static_index_ssbo);
        gl->glDeleteBuffers(1, &dynamic_vertex_ssbo);
        gl->glDeleteBuffers(1, &dynamic_index_ssbo);
        gl->glDeleteBuffers(1, &mesh_ssbo);
        gl->glDeleteBuffers(1, &material_ssbo);
    }

    void Renderer::initialize(OpenGLFunctions* gl) {
        this->gl = gl;
        gl->make_current();
        gl->initializeOpenGLFunctions();

        // Setup the render shader
        ShaderStage comp_shader{GL_COMPUTE_SHADER, ":/src/rendering/shaders/raytrace.glsl"};

        render_shader.load_shaders(&comp_shader, 1);
        render_shader.validate();

        gl->glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);

        // Set up the SSBOs
        gl->glGenBuffers(1, &vertex_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
        vertex_ssbo_size = 0;

        gl->glGenBuffers(1, &static_vertex_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, static_vertex_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        static_vertex_ssbo_size = 0;

        gl->glGenBuffers(1, &static_index_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_index_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, static_index_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        static_index_ssbo_size = 0;
        
        gl->glGenBuffers(1, &dynamic_vertex_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dynamic_vertex_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
        dynamic_vertex_ssbo_size = 0;

        gl->glGenBuffers(1, &dynamic_index_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_index_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dynamic_index_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
        dynamic_index_ssbo_size = 0;

        gl->glGenBuffers(1, &mesh_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mesh_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
        mesh_ssbo_size = 0;

        gl->glGenBuffers(1, &material_ssbo);
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, material_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STREAM_DRAW);
        material_ssbo_size = 0;

        gl->glGenTextures(1, &material_texture_array);
        gl->glBindTexture(GL_TEXTURE_2D_ARRAY, material_texture_array);
        gl->glTextureParameteri(material_texture_array, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTextureParameteri(material_texture_array, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTextureParameteri(material_texture_array, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTextureParameteri(material_texture_array, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        gl->glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    bool Renderer::update() {
        if (scene) {
            gl->make_current();

            dynamic_vertices.clear();
            dynamic_indices.clear();
            meshes = scene->get_static_meshes();
            traverse_node_tree(scene);

            gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_vertex_ssbo);
            gl->glBufferData(GL_SHADER_STORAGE_BUFFER, dynamic_vertices.size(), dynamic_vertices.data(), GL_STREAM_DRAW);

            gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_index_ssbo);
            gl->glBufferData(GL_SHADER_STORAGE_BUFFER, dynamic_indices.size()*sizeof(Index), dynamic_indices.data(), GL_STREAM_DRAW);

            gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_ssbo);
            gl->glBufferData(GL_SHADER_STORAGE_BUFFER, meshes.size(), meshes.data(), GL_STREAM_DRAW);

            MaterialManager& material_manager = scene->get_material_manager();
            const std::vector<unsigned char>& materials = material_manager.get_materials();

            gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_ssbo);
            gl->glBufferData(GL_SHADER_STORAGE_BUFFER, materials.size(), materials.data(), GL_STREAM_DRAW);

            const std::vector<unsigned char>& mm_texture_array = material_manager.get_material_textures();
            TextureIndex new_nr_material_textures = mm_texture_array.size() / material_manager.bytes_per_image();
            if (nr_material_textures != new_nr_material_textures) {
                nr_material_textures = new_nr_material_textures;
                gl->glBindTexture(GL_TEXTURE_2D_ARRAY, material_texture_array);
                gl->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, material_manager.get_texture_width(), material_manager.get_texture_height(), nr_material_textures, 0, GL_RGBA, GL_UNSIGNED_BYTE, mm_texture_array.data());
            }

            return true;
        }
        return false;
    }

    bool Renderer::render(Texture* render_result, unsigned int width, unsigned int height) {
        if (camera && scene) {
            update();

            gl->make_current();

            if (prev_width != width || prev_height != height) {
                prev_width = width;
                prev_height = height;
                camera->update_perspective(float(width)/height);
            }
            camera->update_view();
            CornerRays eye_rays = camera->get_corner_rays();

            
            gl->glUseProgram(render_shader.get_id());
            render_shader.set_vec3("eye", camera->get_position());
            render_shader.set_vec3("ray00", eye_rays.r00);
            render_shader.set_vec3("ray10", eye_rays.r10);
            render_shader.set_vec3("ray01", eye_rays.r01);
            render_shader.set_vec3("ray11", eye_rays.r11);

            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D_ARRAY, material_texture_array);

            gl->glBindImageTexture(0, render_result->get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            unsigned int worksize_x = round_up_to_pow_2(width);
            unsigned int worksize_y = round_up_to_pow_2(height);
            gl->glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

            // Clean up & make sure the shader has finished writing to the image
            gl->glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return true;
        }
        return false;
    }

    void Renderer::set_camera(AbstractCamera* new_camera) {
        camera = new_camera;
        camera->update_perspective(float(prev_width)/prev_height);
    }

    AbstractCamera* Renderer::get_camera() {
        return camera;
    }

    void Renderer::set_scene(Scene* new_scene) {
        scene = new_scene;

        gl->make_current();

        const std::vector<unsigned char>& static_vertices = scene->get_static_vertices();
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_vertex_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, static_vertices.size(), static_vertices.data(), GL_STATIC_DRAW);
        static_vertex_ssbo_size = static_vertices.size() / vertex_size_in_opengl;

        const std::vector<Index>& static_indices = scene->get_static_indices();
        gl->glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_index_ssbo);
        gl->glBufferData(GL_SHADER_STORAGE_BUFFER, static_indices.size()*sizeof(Index), static_indices.data(), GL_STATIC_DRAW);
        static_index_ssbo_size = static_indices.size();

        nr_material_textures = 0; // Will be updated later in update()
    }

    Scene* Renderer::get_scene() {
        return scene;
    }

    void Renderer::traverse_node_tree(Node* node, glm::mat4 transformation) {
        transformation *= node->get_transformation();

        MaterialManager& material_manager = scene->get_material_manager();
        const std::vector<std::shared_ptr<Mesh>>& node_meshes = node->get_child_meshes();
        MeshIndex mesh_offset = meshes.size();
        meshes.resize(meshes.size() + node_meshes.size()*mesh_size_in_opengl);
        for (auto m : node_meshes) {
            Index vertex_offset = dynamic_vertices.size()/vertex_size_in_opengl;
            Index index_offset = dynamic_indices.size() + scene->get_static_indices().size();
            MaterialIndex material_index = material_manager.get_material_index(m->get_material().get());

            const std::vector<Vertex>& mesh_vertices = m->get_vertices();
            if (vertex_is_opengl_compatible) {
                unsigned char const* mesh_vertex_data = reinterpret_cast<unsigned char const*>(mesh_vertices.data());
                dynamic_vertices.insert(std::end(dynamic_vertices), mesh_vertex_data, mesh_vertex_data+vertex_size_in_opengl*mesh_vertices.size());
            } else {
                dynamic_vertices.resize(dynamic_vertices.size() + mesh_vertices.size()*vertex_size_in_opengl);
                for (Index i=0; i<mesh_vertices.size(); i++) {
                    mesh_vertices[i].as_byte_array(dynamic_vertices.data()+vertex_offset+i*vertex_size_in_opengl);
                }
            }

            const std::vector<Index>& mesh_indices = m->get_indices();
            dynamic_indices.insert(std::end(dynamic_indices), std::begin(mesh_indices), std::end(mesh_indices));

            m->as_byte_array(meshes.data()+mesh_offset, transformation, vertex_offset, index_offset, material_index);
            mesh_offset += mesh_size_in_opengl;
        }

        const std::vector<std::shared_ptr<Node>>& node_child_nodes = node->get_child_nodes();
        for (auto n : node_child_nodes) {
            traverse_node_tree(n.get(), transformation);
        }
    }

    void Renderer::update_material_textures() {
        gl->make_current();

    }

}