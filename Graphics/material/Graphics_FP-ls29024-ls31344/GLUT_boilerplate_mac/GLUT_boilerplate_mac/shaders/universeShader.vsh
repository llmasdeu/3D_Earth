#version 120

attribute vec3 a_vertexPos;
attribute vec2 a_uvs;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

varying vec2 v_uvs;


void main() {
    v_uvs = a_uvs;

    // position of the vertex
    gl_Position = u_projection * u_view * u_model * vec4(a_vertexPos, 1.0);
}

