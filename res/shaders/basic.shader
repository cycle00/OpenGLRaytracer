#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

void main() {
    gl_Position = position;
}

#shader fragment
#version 330 core

layout(pixel_center_image) in vec4 gl_FragCoord;
layout(location = 0) out vec4 color;

void main() {
    vec2 screenpos = vec2(gl_FragCoord.xy);
    color = vec4(screenpos.x, 0.3, screenpos.y, 1.0);
}