#include "Vertex.hpp"
#include <QDebug>

#include <algorithm>

namespace Rt {

    Vertex::Vertex(glm::vec4 position, glm::vec4 normal, glm::vec2 tex_coords) : 
        position(position),
        normal(normal),
        tangent(glm::vec4(0.0f)),
        tex_coords(tex_coords)
    {}

    Vertex::Vertex(glm::vec4 position, glm::vec4 normal, glm::vec4 tangent, glm::vec4 bitangent, glm::vec2 tex_coords) :
        position(position),
        normal(normal),
        tangent(tangent),
        tex_coords(tex_coords)
    {
        glm::vec3 calculated_bitangent = glm::cross(glm::vec3(normal), glm::vec3(tangent));
        this->tangent.w = glm::sign(glm::dot(calculated_bitangent, glm::vec3(bitangent)));
    }

    void Vertex::as_byte_array(unsigned char byte_array[vertex_size_in_opengl]) const {
        static_assert(sizeof(glm::vec4) == 16);
        static_assert(sizeof(glm::vec2) == 8);

        unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&position);
        std::copy(tmp, tmp+16, byte_array);

        tmp = reinterpret_cast<unsigned char const*>(&normal);
        std::copy(tmp, tmp+16, byte_array+16);

        tmp = reinterpret_cast<unsigned char const*>(&tangent);
        std::copy(tmp, tmp+16, byte_array+32);

        tmp = reinterpret_cast<unsigned char const*>(&tex_coords);
        std::copy(tmp, tmp+8, byte_array+48);
    }

}