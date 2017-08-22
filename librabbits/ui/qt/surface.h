/*
 *  This file is part of Rabbits
 *  Copyright (C) 2016  Clement Deschamps and Luc Michel
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SURFACE_H
#define SURFACE_H

#include "rabbits/ui/view/framebuffer.h"

#include <QOpenGLWidget>

class QOpenGLContext;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;
class QOpenGLFunctions;

class Surface : public QOpenGLWidget
{
private:
    Q_OBJECT

    struct TextureUnpackInfo {
        GLenum format;
        GLenum type;
        bool soft_unpack;
        std::vector<uint8_t> unpacked_buffer;
        GLvoid *data;
    };

    FramebufferInfo m_info;
    TextureUnpackInfo m_tex_info;

    GLuint m_texture = 0, m_blit_prog = 0;
    QOpenGLVertexArrayObject *m_vao = nullptr;
    QOpenGLBuffer *m_vbo = nullptr;

    QOpenGLFunctions *m_func = nullptr;

    std::vector<uint32_t> m_palette;

    void report_shader_compile_error(GLenum type, GLuint shader);
    void report_prog_link_error(GLuint prog);
    GLuint compile_shader(GLenum type, const GLchar *src);
    GLuint link_program(GLuint vert, GLuint frag);
    void compile_shaders();

    void create_vao();
    void create_texture();

    void update_texture_info();
    void unpack_texture();
    void unpack_texture_indexed();
    void update_texture();

    bool gl_inited() const { return m_func != nullptr; }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

public:
    Surface(QWidget *parent = 0);

    void set_info(const FramebufferInfo & info);
    void set_palette(const std::vector<uint32_t> &palette);
};

#endif // SURFACE_H
