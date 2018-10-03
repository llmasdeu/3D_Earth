#version 120

attribute vec2 a_uvs;
attribute vec3 a_normal;
attribute vec3 a_vertexPos;

uniform mat3 u_normal_matrix;
uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

varying vec2 v_uvs;
varying vec3 v_normal;
varying vec3 v_world_vertex_pos;

void main() {
    v_uvs = a_uvs;

	// position of the vertex
    gl_Position = u_projection * u_view * u_model * vec4(a_vertexPos, 1.0);
    vec3 world_vertex_pos = (u_model * vec4(a_vertexPos, 1.0)).xyz;

    v_normal = u_normal_matrix * a_normal;
    v_world_vertex_pos = world_vertex_pos;
}

