#define MAX_OBJ_VERTICES 100000
#define MAX_OBJ_FACES 100000

typedef struct {
    float x;
    float y;
    float z;
} Normal;

typedef struct {
    float x;
    float y;
} TextureCoord;

typedef struct {
    int vertices[3];
    int normals[3];
    int texture_coords[3];
} Face;

typedef struct {
    float x, y, z;
} Vector;

typedef struct {
    Vector *vertices;
    int num_vertices;
    Face *faces;
    int num_faces;
    Normal *normals;
    int num_normals;
    TextureCoord *texture_coords;
    int num_texture_coords;
    GLuint texture_id;
} Model;

typedef enum {
    VERTEX_ONLY,
    VERTEX_NORMAL,
    VERTEX_TEXTURE,
    VERTEX_ALL
} FaceType;

void draw_model(Model model) {
    glBindTexture(GL_TEXTURE_2D, model.texture_id);

    // TODO: stop hardcoding this
    float vertices[4000 * 8] = {0};

    for (int i = 0; i < model.num_faces; i++) { // which face
        Face f = model.faces[i];

        for (int j = 0; j <= 2; j++) { // which vertex

            Vector v = model.vertices[f.vertices[j]];
            vertices[i*3*8 + j*8 + 0] = v.x;
            vertices[i*3*8 + j*8 + 1] = v.y;
            vertices[i*3*8 + j*8 + 2] = v.z;

            if (model.num_normals) {
                Normal n = model.normals[f.normals[j]];
                vertices[i*3*8 + j*8 + 3] = n.x;
                vertices[i*3*8 + j*8 + 4] = n.y;
                vertices[i*3*8 + j*8 + 5] = n.z;
            }

            if (model.num_texture_coords) {
                TextureCoord t = model.texture_coords[f.texture_coords[j]];
                vertices[i*3*8 + j*8 + 6] = t.x;
                vertices[i*3*8 + j*8 + 7] = t.y;
            }
        }
    }

    glBufferData(GL_ARRAY_BUFFER, model.num_faces * 3 * 8 * sizeof(float), vertices, GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, model.num_faces*3);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Model load_wavefront(const char *obj_filename, const char *texture_filename, FaceType face_type, int texture_size) {
    Model model = {};

    // NOTE: Objects bigger than the constants are undefined behavior
    model.vertices = (Vector *) malloc(MAX_OBJ_VERTICES * sizeof(Vector));
    model.faces = (Face *) malloc(MAX_OBJ_FACES * sizeof(Face));
    model.normals = (Normal *) malloc(MAX_OBJ_VERTICES * sizeof(Normal));
    model.texture_coords = (TextureCoord *) malloc(MAX_OBJ_VERTICES * sizeof(TextureCoord));

    if (face_type == VERTEX_ALL || face_type == VERTEX_TEXTURE) {
        // NOTE: this can be freed if it`s not a map model

        // normal texture
        {
            int x,y,n;
            unsigned char *sur = stbi_load(texture_filename, &x, &y, &n, 3);

            assert(sur);

            glGenTextures(1, &model.texture_id);
            glBindTexture(GL_TEXTURE_2D, model.texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, sur);
            glBindTexture(GL_TEXTURE_2D, 0);

            stbi_image_free(sur);
        }
    }

    FILE *f = fopen(obj_filename, "r");
    assert(true);
    char type[40] = {};
    while ((fscanf(f, " %s", type)) != EOF) {
        if (!strcmp(type, "v")) {
            Vector v = {};

            fscanf(f, " %f %f %f", &v.x, &v.y, &v.z);
            model.vertices[++model.num_vertices] = v;
        } else if (!strcmp(type, "vn")) {
            Normal n = {};

            fscanf(f, " %f %f %f", &n.x, &n.y, &n.z);
            model.normals[++model.num_normals] = n;
        } else if (!strcmp(type, "vt")) {
            TextureCoord t = {};

            fscanf(f, " %f %f", &t.x, &t.y);
            t.y = 1 - t.y;
            model.texture_coords[++model.num_texture_coords] = t;
        } else if (!strcmp(type, "f")) {
            Face face = {};

            if (face_type == VERTEX_ONLY) {
                fscanf(f, " %d %d %d", &face.vertices[0], &face.vertices[1], &face.vertices[2]);
            } else if (face_type == VERTEX_NORMAL) {
                fscanf(f, " %d//%d %d//%d %d//%d", &face.vertices[0], &face.normals[0], &face.vertices[1],
                        &face.normals[1], &face.vertices[2], &face.normals[2]);
            } else if (face_type == VERTEX_TEXTURE) {
                // NOTE: not supported
                assert(false);
            } else if (face_type == VERTEX_ALL) {
                fscanf(f, " %d/%d/%d %d/%d/%d %d/%d/%d", &face.vertices[0], &face.texture_coords[0],
                        &face.normals[0], &face.vertices[1],
                        &face.texture_coords[1], &face.normals[1],
                        &face.vertices[2], &face.texture_coords[2],
                        &face.normals[2]);
            } else {
                assert(false);
            }

            model.faces[model.num_faces++] = face;
        }
    }

    return model;
}
