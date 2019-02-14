#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

typedef struct
{
    GLuint program;
    GLuint position_location;
    GLuint normal_location;
    GLuint tex_id_location;
    GLuint tex_coord_location;
    GLuint model_location;
    GLuint view_location;
    GLuint color_location;
    GLuint projection_location;
    GLuint texture_location;
} shader;

#endif
