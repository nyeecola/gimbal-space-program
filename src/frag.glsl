#version 330

smooth in vec3 color;
smooth in vec2 texCoord;

uniform sampler2D tex;

void main() {
    //gl_FragColor = vec4(color, 1.0);
    gl_FragColor = texture(tex, texCoord);
};
