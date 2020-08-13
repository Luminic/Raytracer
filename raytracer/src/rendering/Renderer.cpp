#include "Renderer.hpp"
#include "QDebug"

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
    }

    Renderer::~Renderer() {
        glDeleteBuffers(1, &vertex_ssbo);
        glDeleteBuffers(1, &index_ssbo);
    }

    void Renderer::initialize(unsigned int width, unsigned int height) {
        initializeOpenGLFunctions();

        this->width = width;
        this->height = height;

        // Setup the render shader
        ShaderStage comp_shader{GL_COMPUTE_SHADER, ":/src/rendering/shaders/raytrace.glsl"};

        render_shader.load_shaders(&comp_shader, 1);
        render_shader.validate();

        glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
        render_result.create(width, height);

        // Set up the Vertex SSBO
        glGenBuffers(1, &vertex_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_ssbo);

        Vertex verts[] = {
            // Floor
            Vertex(glm::vec4(-1.0f,-1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec4( 1.0f,-1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec4( 1.0f,-1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec4(-1.0f,-1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),

            // Ceiling
            Vertex(glm::vec4(-1.0f, 1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec4( 1.0f, 1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec4( 1.0f, 1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec4(-1.0f, 1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f))
        };

        vertices.insert(vertices.begin(), std::begin(verts), std::end(verts));

        if (vertex_is_opengl_compatible) {
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertices[0])*vertices.size(), vertices.data(), GL_STATIC_DRAW);
        } else {
            std::vector<unsigned char> vertex_data(vertex_struct_size_in_opengl*vertices.size());
            for (unsigned int i=0; i<vertices.size(); i++) {
                vertices[i].as_byte_array(&vertex_data[i*vertex_struct_size_in_opengl]);
            }
            glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);
        }

        // Set up the Index SSBO
        glGenBuffers(1, &index_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, index_ssbo);

        unsigned int inds[] = {
            0, 1, 2,
            2, 3, 0,

            4, 5, 6,
            6, 7, 4
        };

        indices.insert(indices.begin(), std::begin(inds), std::end(inds));
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(indices[0])*indices.size(), indices.data(), GL_STATIC_DRAW);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Not 100% sure if necessary but just in case
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void Renderer::resize(unsigned int width, unsigned int height) {
        this->width = width;
        this->height = height;

        if (camera)
            camera->update_perspective(float(width)/height);
        render_result.resize(width, height);
    }

    Texture* Renderer::render() {
        if (camera) {
            camera->update_view();
            CornerRays eye_rays = camera->get_corner_rays();
            
            glUseProgram(render_shader.get_id());
            render_shader.set_vec3("eye", camera->get_position());
            render_shader.set_vec3("ray00", eye_rays.r00);
            render_shader.set_vec3("ray10", eye_rays.r10);
            render_shader.set_vec3("ray01", eye_rays.r01);
            render_shader.set_vec3("ray11", eye_rays.r11);

            glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            unsigned int worksize_x = round_up_to_pow_2(width);
            unsigned int worksize_y = round_up_to_pow_2(height);
            glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

            // Clean up & make sure the shader has finished writing to the image
            glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            return &render_result;
        }
        return nullptr;
    }

    void Renderer::set_camera(AbstractCamera* new_camera) {
        camera = new_camera;
        qDebug() << "setting camera" << (void*)camera;
        camera->update_perspective(float(width)/height);
    }

    AbstractCamera* Renderer::get_camera() {
        return camera;
    }

}