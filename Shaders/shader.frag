#version 450

layout(location = 0) in vec3 v_FragColor;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    // o_Color = vec4(v_TexCoord, 0.0, 1.0);
    o_Color = texture(texSampler, v_TexCoord);

}