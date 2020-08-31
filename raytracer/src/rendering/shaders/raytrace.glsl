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
uniform uint nr_vertices = 0;


layout (std430, binding=1) buffer StaticIndexBuffer {
    // Memory layout should exactly match that of a C++ int array
    uint static_indices[];
};
uniform uint nr_static_indices = 0;


layout (std430, binding=2) buffer DynamicIndexBuffer {
    // Same as StaticIndexBuffer
    // Indices correspond to vertices[dynamic_indices[i] + nr_static_indices]
    uint dynamic_indices[];
};
uniform uint nr_dynamic_indices = 0;


layout (std140, binding=3) buffer StaticVertexBuffer {
    // Same memory layout as VertexBuffer
    Vertex static_vertices[];
};
uniform uint nr_static_vertices = 0;


layout (std140, binding=4) buffer DynamicVertexBuffer {
    Vertex dynamic_vertices[];
};
uniform uint nr_dynamic_vertices = 0;


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
uniform uint nr_meshes = 0;


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
    int normal_ti;         // 4               // 64

    // PADDING:            // 12              // 80

    // Total Size: 80
};

layout(std140, binding=6) buffer MaterialBuffer {
    Material materials[];
};
uniform uint nr_materials = 0;


struct Light {
                              // Base Alignment  // Aligned Offset
    vec3 position;            // 12              // 0
    int type;                 // 4               // 12
    vec3 direction;           // 12              // 16
    int visibility;           // 4               // 28
    vec3 radiance;            // 12              // 32
    float ambient_multiplier; // 4               // 44

    // Total Size: 48
};

layout(std430, binding=7) buffer LightBuffer {
    Light lights[];
};
uniform uint nr_lights = 0;


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


struct LightData {
    vec3 direction;
    float light_distance; // Negative distance (-1) means infinitely far away
    vec3 radiance;
    float ambient_multiplier;
};

// 0: Sunlight
// 1: Pointlight
// 2: Dirlight (Currently unimplemented)
LightData get_light_data(Light light, vec3 at) {
    LightData light_data = {
        vec3(1.0f), -1.0f, vec3(1.0f,0.0f,1.0f), 1.0f
    };

    if (light.type == 0) {
        light_data.direction = normalize(light.direction);
        light_data.light_distance = -1;
        light_data.radiance = light.radiance;
        light_data.ambient_multiplier = light.ambient_multiplier;
    }
    else if (light.type == 1) {
        light_data.direction = normalize(at - light.position);
        light_data.light_distance = distance(light.position, at);
        float falloff = 1.0f / (1.0f + light_data.light_distance*light_data.light_distance);
        light_data.radiance = light.radiance * falloff;
        light_data.ambient_multiplier = light.ambient_multiplier * falloff;
    }

    return light_data;
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
    A ray is described as ray_origin + t * ray_dir
    This function returns t if the ray intersects the plane. Negative output means no intersection
    
    Ray equation: <x,y,z> = ray_origin + t * ray_dir
    Plane equation: dot(plane_normal, <x,y,z>) + D = 0
    We can calculate D = -dot(plane_normal, plane_point)

    dot(plane_normal, ray_origin + t * ray_dir) + D = 0
    dot(plane_normal, ray_origin) + t * dot(plane_normal, ray_dir) + D = 0
    t = - (D + dot(plane_normal, ray_origin)) / dot(plane_normal, ray_dir)
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

float ray_sphere_int(vec3 ray_origin, vec3 ray_dir, vec3 sphere_origin, float sphere_radius, out bool intersected) {
    /*
    math source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
    ray: <x,y,z> = ray_origin + t * ray_dir
    sphere centered @ origin: x^2 + y^2 + z^2 = sphere_radius^2 
    intersection: ||ray_origin + t * ray_dir||^2 - sphere_radius^2 = 0
    intersection (sphere not centered, multiplication between vectors is the dot prduct):
        ||(ray_origin-sphere_origin) + t * ray_dir||^2 - sphere_radius^2 = 0
        ||ray_origin-sphere_origin||^2 + 2*(ray_origin-sphere_origin)(t * ray_dir) + t^2 * ||ray_dir||^2 - sphere_radius^2 = 0
    using quadratic formula to solve for t: x = (-b + sqrt(b^2 - 4ac)) / 2a
        a = ||ray_dir||^2
        b = 2*(ray_origin-sphere_origin)*ray_dir
        c = ||ray_origin-sphere_origin||^2 - sphere_radius^2
    */

    // Assumes ray_dir is normalized
    float a = 1;
    float b = 2*dot(ray_origin-sphere_origin, ray_dir);
    float c = dot(ray_origin-sphere_origin, ray_origin-sphere_origin) - sphere_radius*sphere_radius;

    float inside_sqrt = b*b - 4*a*c;
    if (inside_sqrt <= 0) {
        intersected = false;
        return FAR_PLANE;
    }
    intersected = true;
    // Modified quadradic equation to avoid loss of precision
    return 2*c / (-b + sqrt(inside_sqrt));
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
                    vert.tangent = bc.x*v0.tangent + bc.y*v1.tangent + bc.z*v2.tangent;
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

int cast_ray_for_lights(vec3 ray_origin, vec3 ray_dir, float near_plane, float far_plane, out float depth) {
    int closest_light_index = -1;
    float closest_depth = far_plane;
    for (uint i=0; i<nr_lights; i++) {
        Light current_light = lights[i];
        if (current_light.visibility == 1) {
            bool intersected;
            float current_depth = ray_sphere_int(ray_origin, ray_dir, current_light.position, 0.1f, intersected);
            if (intersected && current_depth > near_plane && current_depth < closest_depth) {
                closest_depth = current_depth;
                closest_light_index = int(i);
            }
        }
    }
    depth = closest_depth;
    return closest_light_index;
}

int cast_ray_for_lights(vec3 ray_origin, vec3 ray_dir, out float depth) {
    return cast_ray_for_lights(ray_origin, ray_dir, NEAR_PLANE, FAR_PLANE, depth);
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

// uniform LightData sunlight = LightData(normalize(vec3(0.4f, -1.0f, -0.4f)), vec3(3.0f), 1.0f);
// uniform LightData sunlight = LightData(normalize(vec3(0.3f, -0.3f, -1.0f)), vec3(3.0f), 0.3f);

#define BIAS 0.0001f
#define SHADOWS 1
vec3 calculate_light(vec3 position, vec3 normal, vec3 ray_dir, MaterialData material, Light light) {
    LightData light_data = get_light_data(light, position);
    #if SHADOWS
        int mesh_index;
        float light_distance = light_data.light_distance;
        if (light_distance < -EPSILON) light_distance = FAR_PLANE;
        cast_ray(position, -light_data.direction, BIAS, light_distance, mesh_index);
        if (mesh_index != -1) {
            return material.albedo.rgb * material.AO * light_data.radiance * light_data.ambient_multiplier;
        }
    #endif
    vec3 color = cook_torrance_BRDF(-ray_dir, normal, -light_data.direction, material);
    color *= light_data.radiance * max(dot(normal, -light_data.direction), 0.0f);
    color += material.albedo.rgb * material.AO * light_data.radiance * light_data.ambient_multiplier;
    return color;
}


// ~=~=~=~=~=~=~= Tracing =~=~=~=~=~=~=~

vec4 trace(vec3 ray_origin, vec3 ray_dir) {
    ray_dir = normalize(ray_dir);
    int mesh_index;
    Vertex vert = cast_ray(ray_origin, ray_dir, mesh_index);

    // Check for ray intersection w/ light (if so, terminate early to avoid unnecessary calculations)
    float vertex_depth = length(vert.position.xyz-ray_origin);
    float lights_depth;
    int light_index = cast_ray_for_lights(ray_origin, ray_dir, lights_depth);
    if (light_index != -1 && (lights_depth <= vertex_depth || mesh_index == -1)) {
        return vec4(lights[light_index].radiance, 1.0f);
    }

    if (mesh_index == -1) return vec4(0.0f,1.0f,0.0f,1.0f);

    vert.normal = vec4(normalize(vert.normal.xyz), 0.0f);
    float normal_sign = sign(dot(vert.normal.xyz, -ray_dir));
    vert.normal *= normal_sign;

    Material material = materials[meshes[mesh_index].material_index];
    if (material.normal_ti != -1) {
        vec3 tex_normal = textureLod(material_textures, vec3(vert.tex_coord, float(material.normal_ti)), 0).xyz * 2.0f - 1.0f;
        vec3 norm = vert.normal.xyz;
        vec3 tang = vert.tangent.xyz;
        tang -= dot(tang, norm)*norm; // No need to divde by magnitude normal squared because the normal vectors should already be normalized
        tang = normalize(tang);
        vec3 bitang = normalize(cross(norm, tang)) * vert.tangent.w * normal_sign;

        vert.normal = normalize(vec4(mat3(tang, bitang, norm) * tex_normal, 0.0f));
    }

    MaterialData mat = get_material_data(material, vert.tex_coord);
    // vec3 color = calculate_light(vert.position.rgb, vert.normal.xyz, ray_dir, mat, Light(vec3(0.0f), 0, vec3(0.4f, -1.0f, -0.4f), 1, vec3(3.0f), 1.0f));
    vec3 color = vec3(0.0f);
    for (uint i=0; i<nr_lights; i++) {
        color += calculate_light(vert.position.rgb, vert.normal.xyz, ray_dir, mat, lights[i]);
    }
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