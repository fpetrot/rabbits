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

#include <cassert>
#include <unordered_map>

#include <QOpenGLContext>
#include <QTimer>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QSizePolicy>

#include <QtDebug>

#include "rabbits/logger.h"

#include "surface.h"


Surface::Surface(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(update()));
    timer->setInterval(1000/30);
    timer->setSingleShot(false);
    timer->start();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

struct OpenGLFormatMapping {
    GLenum format;
    GLenum type;
};

typedef std::unordered_map<uint32_t, const OpenGLFormatMapping> RabbitsToOpenGLMapping;

static const RabbitsToOpenGLMapping RABBITS_TO_GL_MAP {
    { FramebufferInfo::RGBA_8888, { GL_RGBA, GL_UNSIGNED_BYTE } },
    { FramebufferInfo::BGRA_8888, { GL_BGRA, GL_UNSIGNED_BYTE } },

    { FramebufferInfo::RGB_888, { GL_RGB, GL_UNSIGNED_BYTE } },
    { FramebufferInfo::BGR_888, { GL_BGR, GL_UNSIGNED_BYTE } },

    { FramebufferInfo::RGB_565, { GL_RGB, GL_UNSIGNED_SHORT_5_6_5 } },
};

void Surface::update_texture_info()
{
    const uint32_t id = m_info.pixel_info.get_id();
    const bool has_native = (RABBITS_TO_GL_MAP.find(id) != RABBITS_TO_GL_MAP.end());

    LOG(APP, DBG) << "qt fb: Update texture info for pixel format " << m_info.pixel_info << "\n";

    if ((!m_info.post_processor) && has_native) {
        const OpenGLFormatMapping &map = RABBITS_TO_GL_MAP.at(id);
        m_tex_info.format = map.format;
        m_tex_info.type = map.type;
        m_tex_info.soft_unpack = false;
        m_tex_info.data = m_info.data;
        LOG(APP, DBG) << "qt fb: Using native OpenGL unpacker\n";
        return;
    }

    /* Soft unpack */
    m_tex_info.soft_unpack = true;

    m_tex_info.type = GL_UNSIGNED_BYTE; /* Unpack to 32bpp RGBA texture */
    m_tex_info.format = GL_RGBA;

    m_tex_info.data = nullptr; /* Will be set by the software unpacker */

    LOG(APP, DBG) << "qt fb: Using software unpacker\n";
}

void Surface::set_info(const FramebufferInfo & info)
{
    qDebug("Surface::set_info: w=%d, h=%d, buf=%p\n",
          info.w, info.h, info.data);

    m_info = info;
    update_texture_info();
}

void Surface::set_palette(const std::vector<uint32_t> &palette)
{
    m_palette = palette;
}

static GLchar vert_shader_src[] = R"(
    #version 330 core

    in vec2 vert_pos;
    out vec2 tex_coord;

    void main(void) {
        gl_Position = vec4(vert_pos, 0.0, 1.0);
        tex_coord = vec2(vert_pos.x + 1.0, 1.0 - vert_pos.y) * 0.5;
    }
)";

static GLchar frag_shader_src[] = R"(
    #version 330 core

    uniform sampler2D surface;

    in vec2 tex_coord;
    out vec3 color;

    void main(void) {
        color = texture(surface, tex_coord).rgb;
    }
)";

void Surface::report_shader_compile_error(GLenum type, GLuint shader)
{
    char * msg = nullptr;

    GLint length;
    m_func->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    msg = new char[length];
    m_func->glGetShaderInfoLog(shader, length, &length, msg);

    qCritical() << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
        << "shader compile error:" << msg << "\n";

    delete [] msg;
}

void Surface::report_prog_link_error(GLuint prog)
{
    char * msg = nullptr;

    GLint length;
    m_func->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &length);

    msg = new char[length];
    m_func->glGetProgramInfoLog(prog, length, &length, msg);

    qCritical() << "shader program link error:" << msg << "\n";

    delete [] msg;
}

GLuint Surface::compile_shader(GLenum type, const GLchar *src)
{

    GLuint shader = m_func->glCreateShader(type);
    m_func->glShaderSource(shader, 1, &src, NULL);
    m_func->glCompileShader(shader);

    GLint status;
    m_func->glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        report_shader_compile_error(type, shader);
        return 0;
    }

    return shader;
}

GLuint Surface::link_program(GLuint vert, GLuint frag)
{

    GLuint prog = m_func->glCreateProgram();
    m_func->glAttachShader(prog, vert);
    m_func->glAttachShader(prog, frag);
    m_func->glLinkProgram(prog);

    GLint status;
    m_func->glGetProgramiv(prog, GL_LINK_STATUS, &status);

    if (!status) {
        report_prog_link_error(prog);
        return 0;
    }

    return prog;
}

void Surface::compile_shaders()
{

    GLuint vert = compile_shader(GL_VERTEX_SHADER, vert_shader_src);

    if (!vert) {
        return;
    }

    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, frag_shader_src);

    if (!frag) {
        m_func->glDeleteShader(vert);
        return;
    }

    m_blit_prog = link_program(vert, frag);

    m_func->glDeleteShader(vert);
    m_func->glDeleteShader(frag);
}

static const GLfloat vertices[] = {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f,
};

void Surface::create_vao()
{

    m_vao = new QOpenGLVertexArrayObject(this);
    m_vao->create();
    m_vao->bind();

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo->bind();

    m_vbo->allocate(vertices, sizeof(vertices));

    GLint attr = m_func->glGetAttribLocation(m_blit_prog, "vert_pos");
    m_func->glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, 0);
    m_func->glEnableVertexAttribArray(attr);
}

void Surface::create_texture()
{
    m_func->glGenTextures(1, &m_texture);
    m_func->glBindTexture(GL_TEXTURE_2D, m_texture);

    m_func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_func->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

class BitsStream {
private:
    uint8_t *m_buf;
    uint8_t m_cur;
    int m_left;

public:
    BitsStream(uint8_t *buffer) : m_buf(buffer)
    {
        m_cur = *m_buf++;
        m_left = 8;
    }

    uint8_t next_bits(int bits)
    {
        uint8_t ret = 0;
        int sh = 0;

        assert(bits <= 8);

        if (bits > m_left) {
            ret = m_cur;
            m_cur = *(m_buf++);
            sh = m_left;
            bits -= m_left;
            m_left = 8;
        }

        ret |= ((m_cur & ((1 << bits) - 1)) << sh);
        m_left -= bits;
        m_cur >>= bits;

        return ret;
    }

};

static uint8_t normalize(uint8_t pix, int orig_s)
{
    uint32_t p = pix;

    p = p * 255 / ((1 << orig_s) - 1);

    return (p > 255) ? 255 : p;
}

void Surface::unpack_texture_indexed()
{
    int size = m_info.pixel_info.get_bpp();

    const size_t buf_size = m_info.w * m_info.h * 4;
    if (m_tex_info.unpacked_buffer.capacity() < buf_size) {
        m_tex_info.unpacked_buffer.reserve(buf_size);
    }

    if (m_palette.size() < (1u << size)) {
        qWarning() << "Not enough entry in palette\n";
        m_palette.resize(1 << size);
    }

    BitsStream pix_in(reinterpret_cast<uint8_t*>(m_info.data));
    uint32_t *pix_out = reinterpret_cast<uint32_t*>(&(m_tex_info.unpacked_buffer[0]));

    for (int i = 0; i < m_info.w * m_info.h; i++) {
        uint8_t idx = pix_in.next_bits(size);
        *(pix_out++) = m_palette[idx] | 0xff000000;
    }

    m_tex_info.data = &(m_tex_info.unpacked_buffer[0]);
}

void Surface::unpack_texture()
{
    const size_t buf_size = m_info.w * m_info.h * 4;
    const PixelInfo &info = m_info.pixel_info;

    if (m_tex_info.unpacked_buffer.capacity() < buf_size) {
        m_tex_info.unpacked_buffer.reserve(buf_size);
    }

    int c_idx[4];

    c_idx[info.get_r().idx] = 0;
    c_idx[info.get_g().idx] = 1;
    c_idx[info.get_b().idx] = 2;

    BitsStream pix_in(reinterpret_cast<uint8_t*>(m_info.data));
    uint8_t *pix_out = &(m_tex_info.unpacked_buffer[0]);

    for (int i = 0; i < m_info.w * m_info.h; i++) {
        for (int j = 0; j < 4; j++) {
            const PixelInfo::ComponentInfo &comp = info.get_comp(j);

            uint8_t c = pix_in.next_bits(info.get_comp(j).size);

            if (comp.sym == 'A') {
                /* Ignore alpha channel */
                continue;
            }

            pix_out[c_idx[j]] = normalize(c, info.get_comp(j).size);
        }

        pix_out += 4;
    }

    m_tex_info.data = &(m_tex_info.unpacked_buffer[0]);
}

void Surface::update_texture()
{
    if (!m_info.enabled) {
        return;
    }

    if (m_tex_info.soft_unpack) {
        if (m_info.pixel_info.is_indexed()) {
            unpack_texture_indexed();
        } else {
            unpack_texture();
        }

        if (m_info.post_processor) {
            const uint8_t *src_data = reinterpret_cast<const uint8_t*>(m_info.data);
            uint8_t *dst_data = reinterpret_cast<uint8_t*>(m_tex_info.data);
            PixelInfo dst_fmt(FramebufferInfo::RGBA_8888);
            size_t count = m_info.w * m_info.h;

            m_info.post_processor->fb_post_process(m_info.pixel_info, src_data,
                                                      dst_fmt, dst_data, count);
        }
    }

    m_func->glBindTexture(GL_TEXTURE_2D, m_texture);
    m_func->glPixelStorei(GL_UNPACK_ROW_LENGTH, m_info.w);
    m_func->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                    m_info.w, m_info.h, 0,
                    m_tex_info.format, m_tex_info.type, m_tex_info.data);
}


void Surface::initializeGL()
{
    m_func = context()->functions();

    compile_shaders();
    create_vao();
    create_texture();

    m_func->glClearColor(0, 0, 0, 1);
}

void Surface::paintGL()
{
    if (!m_info.enabled) {
        return;
    }

    update_texture();

    m_func->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_func->glUseProgram(m_blit_prog);
    m_vao->bind();

    m_func->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Surface::resizeGL(int w, int h)
{
    qDebug("Surface::resizeGL: %dx%d\n", w, h);

    if (!m_info.enabled) {
        m_func->glViewport(0, 0, w, h);
        return;
    }

    float wr = float(w) / m_info.w;
    float hr = float(h) / m_info.h;

    int final_w, final_h;

    if (wr < hr) {
        int stripe = h - (h * wr / hr);
        final_w = w;
        final_h = h - stripe;

        m_func->glViewport(0, stripe / 2, final_w, final_h);
    } else {
        int stripe = w - (w * hr / wr);
        final_w = w - stripe;
        final_h = h;

        m_func->glViewport(stripe / 2, 0, final_w, final_h);
    }
}
