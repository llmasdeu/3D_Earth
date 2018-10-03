#version 120

varying vec2 v_uvs;

uniform sampler2D u_texture;
uniform vec3 u_cam_pos;
uniform float u_transparency;

void main(void) {
    vec3 texture_color = texture2D(u_texture, v_uvs).xyz;
    vec3 final_color = texture_color.xyz;

    gl_FragColor = vec4(final_color, u_transparency);
}


