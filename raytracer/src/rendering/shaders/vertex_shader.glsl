#version 450 core

struct Vertex {
                    // Base Alignment  // Aligned Offset
    vec4 position;  // 4                  0
                    // 4                  4
                    // 4                  8
                    // 4 (total:16)       12

    vec4 normal;    // 4                  16
                    // 4                  20
                    // 4                  24
                    // 4 (total:16)       28
    
    vec4 tangent;   // 4                  32
                    // 4                  36
                    // 4                  40
                    // 4 (total:16)       44
   
    vec2 tex_coord; // 4                  48 
                    // 4 (total:8)        52

    // (PADDING)    // 8                  56
    // (8 bytes of padding to pad out struct to a multiple of the size of a vec4)

    // Total Size: 64
};

layout (std140, binding=0) buffer VertexBuffer {
    Vertex vertices[];
    //             // Base Alignment  // Aligned Offset
    // vertex[0]      64                 0
    // vertex[1]      64                 64
    // vertex[2]      64                 128
    // ...
    // Potential maximum of 2,000,000 Vertices (128 MB / 64 B)
};
uniform uint nr_vertices;

layout (std140, binding=3) buffer StaticVertexBuffer {
    // Same memory layout as VertexBuffer
    Vertex static_vertices[];
};
uniform uint nr_static_vertices;

layout (std140, binding=4) buffer DynamicVertexBuffer {
    Vertex dynamic_vertices[];
};
uniform uint nr_dynamic_vertices;

layout (std430, binding=1) buffer StaticIndexBuffer {
    // Memory layout should exactly match that of a C++ int array
    uint static_indices[];
};
uniform uint nr_static_indices;

layout (std430, binding=2) buffer DynamicIndexBuffer {
    // Same as StaticIndexBuffer
    // Indices correspond to vertices[dynamic_indices[i] + nr_static_indices]
    uint dynamic_indices[];
};
uniform uint nr_dynamic_indices;

struct Mesh {
                          // Base Alignment  // Aligned Offset
    mat4 transformation;  // 16              // 0
                          // 16              // 16
                          // 16              // 32
                          // 16 (total: 64)  // 48

    int vertex_offset;    // 4               // 64

    int index_offset;     // 4               // 68
    int nr_indices;       // 4               // 72

    int material_index;   // 4               // 76

    // Total Size: 80
};

layout (std140, binding=5) buffer MeshBuffer {
    Mesh meshes[];
    //          // Base Alignment  // Aligned Offset
    // mesh[0]  // 80              // 0
    // mesh[1]  // 80              // 80
    // mesh[3]  // 80              // 160
    // ...
};
uniform uint nr_meshes;

#define Y_SIZE 64
layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    // The OGL spec says the minimum possible invocations for x, y, and z is 65535
    // The maximum number of elements in VertexBuffer far exceeds this number
    // so a second axis must be used to augment the size of index

    // The corresponding ogl code calling dispatch compute MUST know this Y_SIZE value and
    // take it into account when dispatching compute
    uint index = gl_GlobalInvocationID.x*Y_SIZE + gl_GlobalInvocationID.y;
    if (index >= nr_vertices || gl_GlobalInvocationID.y >= Y_SIZE) {
        return;
    }

    Vertex vert;
    if (index < nr_static_vertices) {
        vert = static_vertices[index];
    } else {
        vert = dynamic_vertices[index - nr_static_vertices];
    }

    // Honestly not a great solution but I have no idea how to get the mesh index another way
    uint mesh_index = 0;
    for (uint mi=0; mi<nr_meshes; mi++) {
        uint mesh_mi_vertex_offset = meshes[mi].vertex_offset;
        if (meshes[mi].index_offset >= nr_static_indices)
            mesh_mi_vertex_offset += nr_static_vertices;

        if (mesh_mi_vertex_offset > index)
            break;

        mesh_index = mi;
    }

    mat4 model = meshes[mesh_index].transformation;

    vert.position = model * vec4(vert.position.xyz, 1.0f);
    mat3 ti_model = transpose(inverse(mat3(model)));
    vert.normal = vec4(ti_model * vert.normal.xyz, 0.0f);
    vert.tangent = vec4(ti_model * vert.tangent.xyz, vert.tangent.w);

    vertices[index] = vert;
}