#version 330

uniform mat4 MVP;

layout (location = 0) in vec3 vCol;
layout (location = 1) in vec3 vPos;
layout (location = 2) in vec2 vTexCoord;

smooth out vec3 color;
smooth out vec2 texCoord;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vCol;
    texCoord = vTexCoord;
};
