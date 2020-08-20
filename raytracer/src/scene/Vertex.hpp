#ifndef RT_VERTEX_HPP
#define RT_VERTEX_HPP

#include <glm/glm.hpp>

#include "RaytracerGlobals.hpp"

namespace Rt {

    constexpr int vertex_size_in_opengl = 48;

    struct Vertex {
    public:
                                // Base Alignment  // Aligned Offset
        glm::vec4 position;     // 4                  0  (vec4 is treated identical to an array of 4 floats)
                                // 4                  4
                                // 4                  8
                                // 4                  12

        glm::vec4 normal;       // 4                  16
                                // 4                  20
                                // 4                  24
                                // 4                  28

        glm::vec2 tex_coords;   // 4                  32
                                // 4                  36

        char padding[8];        // 8                  40

        // Total Size: 48
        // Alignment : 4

        Vertex(glm::vec4 position, glm::vec4 normal=glm::vec4(0.0f), glm::vec2 tex_coords=glm::vec2(0.0f));
        Vertex() = default;


        void as_byte_array(unsigned char byte_array[vertex_size_in_opengl]) const;
    };

    /*
    If the compiler doesn't add any padding, the memory layout commented above should be correct

    The C standard doesn't specify how/when padding should be used so we have to have make sure the memory
    layout is what we expect

    The memory layout must exactly match the memory layout of the Vertex struct in GLSL
    */
    constexpr bool vertex_is_opengl_compatible = (sizeof(Vertex) == vertex_size_in_opengl) && std::is_standard_layout<Vertex>::value;

    typedef uint32_t Index;

}

#endif