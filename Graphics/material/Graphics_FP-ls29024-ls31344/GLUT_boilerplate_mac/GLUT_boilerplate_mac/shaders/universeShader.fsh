#version 120

varying vec2 v_uvs;

uniform sampler2D u_texture;

void main(void) {
    vec4 texture_color = texture2D(u_texture, v_uvs);
    vec3 final_color = texture_color.xyz;

    gl_FragColor = vec4(final_color, 1.0);
}
