#include "Renderer.hpp"

#include <QDebug>

#include "scene/lights/AbstractLight.hpp"

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

        // Setup the render shader
        ShaderStage comp_shader{GL_COMPUTE_SHADER, ":/src/rendering/shaders/raytrace.glsl"};

        render_shader.initialize(gl);
        render_shader.load_shaders(&comp_shader, 1);
        render_shader.validate();

        vertex_shader.initialize(gl);
        ShaderStage vert_shader{GL_COMPUTE_SHADER, ":/src/rendering/shaders/vertex_shader.glsl"};
        vertex_shader.load_shaders(&vert_shader, 1);

        gl->glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);

        // Set up the SSBOs
        gl->glCreateBuffers(1, &vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_ssbo);
        gl->glNamedBufferData(vertex_ssbo, 0, nullptr, GL_STREAM_DRAW);
        vertex_ssbo_size = 0;

        gl->glCreateBuffers(1, &static_vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, static_vertex_ssbo);
        gl->glNamedBufferData(static_vertex_ssbo, 0, nullptr, GL_STATIC_DRAW);
        static_vertex_ssbo_size = 0;

        gl->glCreateBuffers(1, &static_index_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, static_index_ssbo);
        gl->glNamedBufferData(static_index_ssbo, 0, nullptr, GL_STATIC_DRAW);
        static_index_ssbo_size = 0;
        
        gl->glCreateBuffers(1, &dynamic_vertex_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dynamic_vertex_ssbo);
        gl->glNamedBufferData(dynamic_vertex_ssbo, 0, nullptr, GL_STREAM_DRAW);
        dynamic_vertex_ssbo_size = 0;

        gl->glCreateBuffers(1, &dynamic_index_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dynamic_index_ssbo);
        gl->glNamedBufferData(dynamic_index_ssbo, 0, nullptr, GL_STREAM_DRAW);
        dynamic_index_ssbo_size = 0;

        gl->glCreateBuffers(1, &mesh_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mesh_ssbo);
        gl->glNamedBufferData(mesh_ssbo, 0, nullptr, GL_STREAM_DRAW);
        mesh_ssbo_size = 0;

        gl->glCreateBuffers(1, &material_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, material_ssbo);
        gl->glNamedBufferData(material_ssbo, 0, nullptr, GL_STREAM_DRAW);
        material_ssbo_size = 0;

        gl->glCreateBuffers(1, &light_ssbo);
        gl->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, light_ssbo);
        gl->glNamedBufferData(light_ssbo, 0, nullptr, GL_STREAM_DRAW);
        light_ssbo_size = 0;

        // We need to create the texture here just in case there are no material textures
        gl->glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &material_texture_array);
        nr_material_textures = 0;
    }

    bool Renderer::update() {
        if (scene) {
            gl->make_current();

            dynamic_vertices.clear();
            dynamic_indices.clear();
            lights.clear();
            meshes = scene->get_static_meshes();
            traverse_node_tree(scene);

            gl->glNamedBufferData(dynamic_vertex_ssbo, dynamic_vertices.size(), dynamic_vertices.data(), GL_STREAM_DRAW);
            dynamic_vertex_ssbo_size = dynamic_vertices.size() / vertex_size_in_opengl;
            gl->glNamedBufferData(dynamic_index_ssbo, dynamic_indices.size()*sizeof(Index), dynamic_indices.data(), GL_STREAM_DRAW);
            dynamic_index_ssbo_size = dynamic_indices.size();
            gl->glNamedBufferData(mesh_ssbo, meshes.size(), meshes.data(), GL_STREAM_DRAW);
            mesh_ssbo_size = meshes.size() / mesh_size_in_opengl;
            gl->glNamedBufferData(light_ssbo, lights.size(), lights.data(), GL_STREAM_DRAW);
            light_ssbo_size = lights.size() / light_size_in_opengl;

            // Allocate enough space for the vertex buffer
            vertex_ssbo_size = dynamic_vertex_ssbo_size+static_vertex_ssbo_size;
            gl->glNamedBufferData(vertex_ssbo, vertex_ssbo_size*vertex_size_in_opengl, nullptr, GL_STREAM_DRAW);

            MaterialManager& material_manager = scene->get_material_manager();
            const std::vector<unsigned char>& materials = material_manager.get_materials();

            gl->glNamedBufferData(material_ssbo, materials.size(), materials.data(), GL_STREAM_DRAW);
            material_ssbo_size = materials.size()/material_size_in_opengl;

            const std::vector<unsigned char>& mm_texture_array = material_manager.get_material_textures();
            TextureIndex new_nr_material_textures = mm_texture_array.size() / material_manager.bytes_per_image();
            if (nr_material_textures != new_nr_material_textures) {
                nr_material_textures = new_nr_material_textures;
                gl->glDeleteTextures(1, &material_texture_array);

                gl->glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &material_texture_array);
                gl->glTextureStorage3D(material_texture_array, 1, GL_RGBA32F, material_manager.get_texture_width(), material_manager.get_texture_height(), nr_material_textures);
                gl->glTextureSubImage3D(material_texture_array, 0, 0, 0, 0, material_manager.get_texture_width(), material_manager.get_texture_height(), nr_material_textures, GL_RGBA, GL_UNSIGNED_BYTE, mm_texture_array.data());

                gl->glTextureParameteri(material_texture_array, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                gl->glTextureParameteri(material_texture_array, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                gl->glTextureParameteri(material_texture_array, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                gl->glTextureParameteri(material_texture_array, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            gl->glUseProgram(vertex_shader.get_id());

            vertex_shader.set_uint("nr_vertices", vertex_ssbo_size);
            vertex_shader.set_uint("nr_static_vertices", static_vertex_ssbo_size);
            vertex_shader.set_uint("nr_dynamic_vertices", dynamic_vertex_ssbo_size);
            vertex_shader.set_uint("nr_static_indices", static_index_ssbo_size);
            vertex_shader.set_uint("nr_dynamic_indices", dynamic_index_ssbo_size);
            vertex_shader.set_uint("nr_meshes", mesh_ssbo_size);

            unsigned int worksize_x = round_up_to_pow_2(vertex_ssbo_size) / Y_SIZE + 1;
            unsigned int worksize_y = Y_SIZE;
            gl->glDispatchCompute(worksize_x, worksize_y, 1);

            // Make sure the vertex shader has finished writing
            gl->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            gl->glUseProgram(0);

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

            render_shader.set_uint("nr_vertices", vertex_ssbo_size);
            render_shader.set_uint("nr_static_vertices", static_vertex_ssbo_size);
            render_shader.set_uint("nr_dynamic_vertices", dynamic_vertex_ssbo_size);
            render_shader.set_uint("nr_static_indices", static_index_ssbo_size);
            render_shader.set_uint("nr_dynamic_indices", dynamic_index_ssbo_size);
            render_shader.set_uint("nr_meshes", mesh_ssbo_size);
            render_shader.set_uint("nr_materials", material_ssbo_size);
            render_shader.set_uint("nr_lights", light_ssbo_size);

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
        gl->glNamedBufferData(static_vertex_ssbo, static_vertices.size(), static_vertices.data(), GL_STATIC_DRAW);
        static_vertex_ssbo_size = static_vertices.size() / vertex_size_in_opengl;

        const std::vector<Index>& static_indices = scene->get_static_indices();
        gl->glNamedBufferData(static_index_ssbo, static_indices.size()*sizeof(Index), static_indices.data(), GL_STATIC_DRAW);
        static_index_ssbo_size = static_indices.size();

        nr_material_textures = 0; // Will be updated later in update()
    }

    Scene* Renderer::get_scene() {
        return scene;
    }

    void Renderer::traverse_node_tree(Node* node, glm::mat4 transformation) {
        transformation *= node->get_transformation();

        // Check node type
        if (node->get_node_type() == Node::NodeType::LIGHT) {
            size_t light_offset = lights.size();
            lights.resize(light_offset + light_size_in_opengl);
            AbstractLight* light = reinterpret_cast<AbstractLight*>(node);
            light->as_byte_array(lights.data()+light_offset, transformation);
        }

        // Add mesh data to buffers
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

        // Recursively loop through child nodes
        const std::vector<std::shared_ptr<Node>>& node_child_nodes = node->get_child_nodes();
        for (auto n : node_child_nodes) {
            traverse_node_tree(n.get(), transformation);
        }
    }

    void Renderer::update_material_textures() {
        gl->make_current();

    }

}