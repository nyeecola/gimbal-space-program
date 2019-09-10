#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define CGLM_ALL_UNALIGNED
#include <cglm/cglm.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define POLL_GL_ERROR poll_gl_error(__FILE__, __LINE__)
void poll_gl_error(char *file, long long line) {
    int err = glGetError();
    if (err) {
        printf("%s: line %lld: error %d, ", file, line, err);
        printf("%s\n", gluErrorString(err));
        exit(1);
    }
}

#include "model.c"

static const struct {
    float x, y, z;
    float r, g, b;
} vertices[6] =
{
    { -0.6f, -0.4f, 0.0f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.0f, 1.f, 0.f, 0.f },
    {   0.f,  0.6f, 0.0f, 1.f, 0.f, 0.f },
    { -0.6f, -0.4f, 0.4f, 0.f, 0.f, 1.f },
    {  0.6f, -0.4f, 0.4f, 0.f, 0.f, 1.f },
    {   0.f,  0.6f, 0.4f, 0.f, 0.f, 1.f }
};

// TODO: caller must free buffer
char* load_file(char const* path) {
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb");

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*)malloc ((length+1)*sizeof(char));
      if (buffer)
      {
        fread (buffer, sizeof(char), length, f);
      }
      fclose (f);
    }
    buffer[length] = '\0';

#if 0
    for (int i = 0; i < length; i++) {
        printf("buffer[%d] == %c\n", i, buffer[i]);
    }
    printf("buffer = %s\n", buffer);
#endif

    return buffer;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
    GLFWwindow* window;

    GLuint vertex_buffer, vertex_shader, fragment_shader, program;

    GLint mvp_location, vpos_location, vcol_location, vtexc_location;

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Engine Propria", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    glewExperimental = 1;
    if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);

    nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas *atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();

    // display OpenGL context version
    {
        const GLubyte *version_str = glGetString(GL_VERSION);
        printf("%s\n", version_str);
    }

    // currently binding but not using for anything
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glfwSwapInterval(1); // TODO: check
    // NOTE: OpenGL error checks have been omitted for brevity
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    ///glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // TODO: fix the size
    GLchar shader_info_buffer[200];
    GLint shader_info_len;

    // TODO: free
    char* vertex_shader_text = load_file("src/vertex.glsl");
    char* fragment_shader_text = load_file("src/frag.glsl");

    // vertex shader
    {
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, (const char * const *) &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        glGetShaderInfoLog(vertex_shader, 200, &shader_info_len, shader_info_buffer);
        if (shader_info_len) printf("Vertex Shader: %s\n", shader_info_buffer);
    }

    // frag shader
    {
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, (const char * const *) &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        glGetShaderInfoLog(fragment_shader, 200, &shader_info_len, shader_info_buffer);
        if (shader_info_len) printf("Fragment Shader: %s\n", shader_info_buffer);
    }

    // shader program
    {
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        glGetProgramInfoLog(program, 200, &shader_info_len, shader_info_buffer);
        if (shader_info_len) printf("Shader Program: %s\n", shader_info_buffer);
    }

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
    vtexc_location = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*) 0);
    //glEnableVertexAttribArray(vcol_location);
    //glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*) (sizeof(float) * 3));
    glEnableVertexAttribArray(vtexc_location);
    glVertexAttribPointer(vtexc_location, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*) (sizeof(float) * 6));
    glBindVertexArray(0);

    versor orientation;
    glm_quat(orientation, 0, 0, 0, 1);

    Model model = load_wavefront("assets/spaceship.obj", "assets/spaceship.png", VERTEX_ALL, 1024);

    while (!glfwWindowShouldClose(window)) {
        POLL_GL_ERROR;
        
        float ratio;
        int width, height;
        mat4 p, mvp;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, width, height);
        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            mat4 model_mat;
            glm_mat4_identity(model_mat);
            glm_translate_z(model_mat, -30);

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                versor update_yaw;
                glm_quat(update_yaw, 0.02, 0, 1, 0);
                
                versor tmp;
                //glm_quat_mul(update_yaw, orientation, tmp); // GLOBAL
                glm_quat_mul(orientation, update_yaw, tmp); // LOCAL

                glm_quat_copy(tmp, orientation);
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                versor update_yaw;
                glm_quat(update_yaw, -0.02, 0, 1, 0);
                
                versor tmp;
                glm_quat_mul(orientation, update_yaw, tmp);

                glm_quat_copy(tmp, orientation);
            }

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                versor update_pitch;
                glm_quat(update_pitch, 0.02, 1, 0, 0);
                
                versor tmp;
                glm_quat_mul(orientation, update_pitch, tmp); // LOCAL

                glm_quat_copy(tmp, orientation);
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                versor update_pitch;
                glm_quat(update_pitch, -0.02, 1, 0, 0);
                
                versor tmp;
                glm_quat_mul(orientation, update_pitch, tmp);

                glm_quat_copy(tmp, orientation);
            }

            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                versor update_roll;
                glm_quat(update_roll, 0.02, 0, 0, -1);
                
                versor tmp;
                glm_quat_mul(orientation, update_roll, tmp);

                glm_quat_copy(tmp, orientation);
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                versor update_roll;
                glm_quat(update_roll, -0.02, 0, 0, -1);
                
                versor tmp;
                glm_quat_mul(orientation, update_roll, tmp);

                glm_quat_copy(tmp, orientation);
            }

            mat4 tmp_model;
            glm_quat_rotate(model_mat, orientation, tmp_model);
            glm_mat4_copy(tmp_model, model_mat);

            float left = -width / 2.0;
            float right = width / 2.0;
            float top = height / 2.0;
            float bottom = -height / 2.0;
            //glm_ortho(left, right, top, bottom, 200.f, -200.f, p);
            glm_perspective(GLM_PI_4f, ratio, 0.01f, 300.0f, p);
            glm_mat4_mul(p, model_mat, mvp);

            glUseProgram(program);

            glBindVertexArray(vao);
                glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp[0]);
                glActiveTexture(GL_TEXTURE0); // TODO: ????
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer); // TODO: ????
                draw_model(model);
            glBindVertexArray(0);
        }

        {
            nk_glfw3_new_frame();

            nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

