#version 450

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;

layout(location = 0) out vec3 v_FragColor;

void main() {
    gl_Position = vec4(a_Position, 0.0, 1.0);
    v_FragColor = a_Color;
}