#version 120

varying vec2 v_uvs;
varying vec3 v_normal;
varying vec3 v_world_vertex_pos;

uniform sampler2D u_texture;
uniform vec3 u_light_dir;
uniform float u_ambient;
uniform vec3 u_light_color;
uniform vec3 u_cam_pos;
uniform float u_shininess;
uniform float u_transparency;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
    
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
			 
    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

// assume N, the interpolated vertex normal and
// V, the view vector (vertex to eye)
vec3 perturbNormal(vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel) {
    normal_pixel = normal_pixel * 2.0 - 1.0;
    mat3 TBN = cotangent_frame(N, V, texcoord);
    return normalize(TBN * normal_pixel);
}

void main(void) {
    vec3 texture_color = texture2D(u_texture, v_uvs).xyz;
    vec3 ambient_color = texture_color * u_ambient;
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_light_dir);
    float NdotL = max(dot(N,L), 0.0);
    vec3 diffuse_color = texture_color * NdotL;
    vec3 R = reflect(L, N);
    vec3 E = normalize(u_cam_pos - v_world_vertex_pos);
    float RdotE = pow(max(dot(R, -E), 0.0), u_shininess);
    vec3 specular_color = u_light_color * RdotE;
    vec3 final_color = texture_color.xyz * NdotL + specular_color + ambient_color;

    gl_FragColor = vec4(final_color, u_transparency);
}


