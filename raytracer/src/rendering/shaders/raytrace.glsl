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
   
    vec2 tex_coord; // 4                  32 
                    // 4 (total:8)        36

    // (PADDING)    // 8                  40
    // (8 bytes of padding to pad out struct to a multiple of the size of a vec4)

    // Total Size: 48
};

layout (std140, binding=0) buffer VertexBuffer {
    Vertex vertices[];
    //             // Base Alignment  // Aligned Offset
    // vertex[0]      48                 0
    // vertex[1]      48                 48
    // vertex[2]      48                 96
    // ...
    // Maximum of 2,666,666 Vertices (128 MB / 48 B)
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

layout (binding=0) uniform sampler2DArray material_textures;

struct Material {
    // textures_index corresponds to textured_materials[textures_index] if the
    // material has textures

                        // Base Alignment  // Aligned Offset
    vec4 albedo;        // 16              // 0
    vec4 F0;            // 16              // 16
    float roughness;    // 4               // 32
    float metalness;    // 4               // 36
    float AO;           // 4               // 40

    // The following ints are texture indices

    int albedo_ti;         // 4               // 44
    int F0_ti;             // 4               // 48
    int roughness_ti;      // 4               // 52
    int metalness_ti;      // 4               // 56
    int AO_ti;             // 4               // 60

    // Total Size: 64
};

layout(std140, binding=6) buffer MaterialBuffer {
    Material materials[];
};
uniform uint nr_materials;


// The per-pixel material data once the textures have been read
// and added to the color information
struct MaterialData {
    vec4 albedo;
    vec4 F0;
    float roughness;
    float metalness;
    float AO;
};

MaterialData get_material_data(Material material, vec2 tex_coords) {
    MaterialData material_data = {
        material.albedo,
        material.F0,
        material.roughness,
        material.metalness,
        material.AO
    };
    if (material.albedo_ti != -1) {
        material_data.albedo *= pow(textureLod(material_textures, vec3(tex_coords, float(material.albedo_ti)), 0), 2.2f.xxxx);
    }
    if (material.F0_ti != -1) {
        material_data.F0 *= textureLod(material_textures, vec3(tex_coords, float(material.F0_ti)), 0);
    }
    if (material.roughness_ti != -1) {
        material_data.roughness *= textureLod(material_textures, vec3(tex_coords, float(material.roughness_ti)), 0).r;
    }
    if (material.metalness_ti != -1) {
        material_data.metalness *= textureLod(material_textures, vec3(tex_coords, float(material.metalness_ti)), 0).r;
    }
    if (material.AO_ti != -1) {
        material_data.AO *= textureLod(material_textures, vec3(tex_coords, float(material.AO_ti)), 0).r;
    }
    return material_data;
}

layout (binding = 0, rgba32f) uniform image2D framebuffer;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

#define EPSILON 0.000001f


/* ~=~=~=~=~=~=~= Ray-Triangle Intersection =~=~=~=~=~=~=~ */

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

float ray_plane_int(vec3 ray_origin, vec3 ray_dir, vec3 plane_point, vec3 plane_normal) {
    /*
    A ray is described as ray_orign + t * ray_dir
    This function returns t if the ray intersects the plane. Negative output means no intersection
    
    Ray equation: <x,y,z> = ray_orign + t * ray_dir
    Plane equation: dot(plane_normal, <x,y,z>) + D = 0
    We can calculate D = -dot(plane_normal, plane_point)

    dot(plane_normal, ray_orign + t * ray_dir) + D = 0
    dot(plane_normal, ray_orign) + t * dot(plane_normal, ray_dir) + D = 0
    t = - (D + dot(plane_normal, ray_orign)) / dot(plane_normal, ray_dir)
    */
    float denom = dot(plane_normal, ray_dir);

    if (abs(denom) <= EPSILON) {
        // The ray is parallel to the plane
        return -1;
    }

    float D = -dot(plane_normal, plane_point);
    float numer = -(dot(plane_normal, ray_origin) + D);

    return numer/denom;
}

vec4 barycentric_coordinates(vec3 point, vec3 tri0, vec3 tri1, vec3 tri2) {
    /*
    Returns the barycentric coordinates of point in the triangle tri0-2
    The w value is 1 if the point is inside of the triangle and -1 otherwise
    */
    float double_area_tri = length(cross(tri1-tri0, tri2-tri0));

    float area0 = length(cross(tri1-point, tri2-point)) / double_area_tri;
    float area1 = length(cross(tri0-point, tri2-point)) / double_area_tri;
    float area2 = length(cross(tri0-point, tri1-point)) / double_area_tri;

    if (area0+area1+area2-1 <= EPSILON) {
        // If the combined area of the 3 mini triangles equals the area of the triangle
        // the point is inside of the triangle
        return vec4(area0, area1, area2, 1);
    } else {
        // Otherwise, the point is outside of the triangle
        // Still return the barycentric coordinates because they might still be useful
        return vec4(area0, area1, area2, -1);
    }
}

Vertex cast_ray(vec3 ray_origin, vec3 ray_dir, float near_plane, float far_plane, out int mesh_index) {
    /*
    Returns an interpolated vertex from the intersection between the ray and the
    nearest triangle it collides with

    If there is no triangle, the w component of position will be -1.0f
    otherwise the w component will be 1.0f
    */
    float depth = far_plane;
    Vertex vert = Vertex(
        vec4(0.0f,0.0f,0.0f,-1.0f),
        vec4(0.0f),
        vec2(0.0f)
    );
    mesh_index = -1;

    for (uint mi=0; mi<nr_meshes; mi++) {

        for (uint i=meshes[mi].index_offset; i<meshes[mi].index_offset+meshes[mi].nr_indices; i+=3) {
            Vertex v0;
            Vertex v1;
            Vertex v2;

            // All mesh vertices must be the in same array (static or dynamic)
            if (i < nr_static_indices) {
                v0 = vertices[static_indices[i+0] + meshes[mi].vertex_offset];
                v1 = vertices[static_indices[i+1] + meshes[mi].vertex_offset];
                v2 = vertices[static_indices[i+2] + meshes[mi].vertex_offset];
            } else {
                v0 = vertices[dynamic_indices[i+0-nr_static_indices] + meshes[mi].vertex_offset + nr_static_vertices];
                v1 = vertices[dynamic_indices[i+1-nr_static_indices] + meshes[mi].vertex_offset + nr_static_vertices];
                v2 = vertices[dynamic_indices[i+2-nr_static_indices] + meshes[mi].vertex_offset + nr_static_vertices];
            }

            vec3 normal = cross(vec3(v1.position-v0.position), vec3(v2.position-v0.position));
            float rpi = ray_plane_int(ray_origin, ray_dir, v0.position.xyz, normalize(normal));

            // If the ray intersects the triangle
            float dist = rpi;
            if (dist >= near_plane && dist <= depth) {
                vec3 intersection_point = ray_origin + rpi*ray_dir;
                vec4 bc = barycentric_coordinates(intersection_point, v0.position.xyz, v1.position.xyz, v2.position.xyz);
                // If the point is inside of the triangle
                if (bc.w > 0.0f) {
                    depth = dist;
                    vert.position = vec4(intersection_point, 1.0f);
                    vert.normal = bc.x*v0.normal + bc.y*v1.normal + bc.z*v2.normal;
                    vert.tex_coord = bc.x*v0.tex_coord + bc.y*v1.tex_coord + bc.z*v2.tex_coord;
                    mesh_index = int(mi);
                }
            }
        }
    }
    vert.normal = vec4(normalize(vert.normal.rgb), 0.0f);
    return vert;
}

Vertex cast_ray(vec3 ray_origin, vec3 ray_dir, out int mesh_index) {
    return cast_ray(ray_origin, ray_dir, NEAR_PLANE, FAR_PLANE, mesh_index);
}


/* ~=~=~=~=~=~=~= Lighting Calculations =~=~=~=~=~=~=~ */

#define PI 3.1415926535f
#define TWO_PI 6.28318531f

float NDF_trowbridge_reitz_GGX(vec3 normal, vec3 halfway, float alpha) {
    float a2 = alpha * alpha;

    return a2 / ( PI * pow( pow(max(dot(normal, halfway), 0.0f), 2) * (a2 - 1) + 1, 2) );
}

float GF_schlick_GGX(float n_dot_v, float roughness) {
    // Schlick approximation
    float k = (roughness+1.0f)*(roughness+1.0f) / 8.0f;

    return n_dot_v / ( n_dot_v * (1-k) + k );
}

float GF_smith(vec3 view, vec3 normal, vec3 light, float alpha) {
    float n_dot_v = max(dot(normal, view), 0.0f);
    float n_dot_l = max(dot(normal, light), 0.0f);
    
    return GF_schlick_GGX(n_dot_v, alpha) * GF_schlick_GGX(n_dot_l, alpha);
}

vec3 F_schlick(vec3 v1, vec3 v2, vec3 F0) {
    // F0 is the reflectivity at normal incidence
    return F0 + (1.0f - F0) * pow((1.0f - max(dot(v1, v2), 0.0f)), 5);
}

vec3 cook_torrance_BRDF(vec3 view, vec3 normal, vec3 light, MaterialData material) {
    vec3 lambertian_diffuse = material.albedo.rgb / PI;

    float alpha = material.roughness * material.roughness;
    vec3 F0 = material.F0.rgb;
    F0 = mix(F0, material.albedo.rgb, material.metalness);
    vec3 halfway = normalize(view + light);

    float NDF = NDF_trowbridge_reitz_GGX(normal, halfway, alpha);
    float GF = GF_smith(view, normal, light, material.roughness);
    vec3 F = F_schlick(light, halfway, F0);

    vec3 kD = (1.0f.xxx - F_schlick(normal, light, F0))*(1.0f.xxx - F_schlick(normal, view, F0));
    kD *= (1.0f - material.metalness);

    vec3 numer = NDF * GF * F;
    float denom = 4.0f * max(dot(normal, view), 0.0f) * max(dot(normal, light), 0.0f);

    return kD*lambertian_diffuse + numer/max(denom, 0.001f);
}

// Lights

struct Light {
    vec3 direction;
    vec3 radiance;
    float ambient_multiplier;
};

uniform Light sunlight = Light(normalize(vec3(0.4f, -1.0f, -0.4f)), vec3(3.0f), 0.3f);

#define BIAS 0.0001f
#define SHADOWS 1
vec3 calculate_light(vec3 position, vec3 normal, vec3 ray_dir, MaterialData material, Light light) {
    #if SHADOWS
        int mesh_index;
        cast_ray(position, -light.direction, BIAS, FAR_PLANE, mesh_index);
        if (mesh_index != -1) {
            return material.albedo.rgb * material.AO * light.ambient_multiplier;
        }
    #endif
    vec3 color = cook_torrance_BRDF(-ray_dir, normal, -light.direction, material);
    color *= light.radiance * max(dot(normal, -light.direction), 0.0f);
    color += material.albedo.rgb * material.AO * light.ambient_multiplier;
    return color;
}


// ~=~=~=~=~=~=~= Tracing =~=~=~=~=~=~=~

vec4 trace(vec3 ray_origin, vec3 ray_dir) {
    ray_dir = normalize(ray_dir);
    int mesh_index;
    Vertex vert = cast_ray(ray_origin, ray_dir, mesh_index);
    vert.normal *= sign(dot(vert.normal.xyz, -ray_dir));

    // return vec4(vert.tex_coord.xy,0.0f,1.0f);
    // return vec4(vert.position.xyz/2.0f, 1.0f);

    if (mesh_index == -1) return vec4(0.0f,1.0f,0.0f,1.0f);

    MaterialData mat = get_material_data(materials[meshes[mesh_index].material_index], vert.tex_coord);
    vec3 color = calculate_light(vert.position.rgb, vert.normal.xyz, ray_dir, mat, sunlight);
    return vec4(color, 1.0f);
}

layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    vec2 tex_coords = vec2(pix)/size;

    vec3 ray = mix(mix(ray00, ray10, tex_coords.x), mix(ray01, ray11, tex_coords.x), tex_coords.y);

    vec4 col = trace(eye, ray);
    imageStore(framebuffer, pix, col);
}