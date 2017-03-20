/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2017  颜烈彬 <slbyan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "beancellitem_p.h"

BeanCellItemPrivate::BeanCellItemPrivate(BeanCellItem *q) : q_ptr(q)
{
    m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_tbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
}

BeanCellItemPrivate::~BeanCellItemPrivate()
{

}

void BeanCellItemPrivate::initialize()
{
    initializeOpenGLFunctions();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Setup Shader
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, &m_program);
    const char *vsrc =
        "#version 330\n"
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec3 voffset;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "   fcolor = vec4(.9,.6,.1, 1.0);\n"
        "   gl_Position = vposition + vec4(voffset, 0);\n"
        "}\n";
    vshader->compileSourceCode(vsrc1);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, &m_program);
    const char *fsrc =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = fcolor;\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    m_program.addShader(vshader);
    m_program.addShader(fshader);
    m_program.link();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void BeanCellItemPrivate::render()
{
    // render rectangles
    // render polygons
    // render paths
    // render trapezoids
    // render circles
    // render xgeometry
    // render texts
    // render placements
    
}


void BeanCellItemPrivate::fillBuffers()
 {}
void BeanCellItemPrivate::bindObjects()
{
  GLenum error = GL_NO_ERROR;
    m_vao.create();
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "v1 " << error;
    }
    m_vao.bind();
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "v2 " << error;
    }
	m_vbo->create();
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "v3 " << error;
    }
    m_vbo->bind();
    m_vbo->allocate(m_vertices.constData(), m_vertices.size()*sizeof(QVector3D));
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "v4 " << error;
    }

    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "1 " << error;
    }
//	int vertexAttrLocation = 0;
//	int offsetAttrLocation = 1;
    m_vbo->bind();
	m_program.enableAttributeArray(0);
    m_program.setAttributeArray(0, 0, 3, sizeof(QVector3D));
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "2 " << error;
    }
//    m_program.setAttributeArray(0, m_vertices.constData());
	GLuint indexData[] =
	{
        0, 1 ,2,
        2, 3, 0,
	};
//    m_program.enableAttributeArray(2);
//    QVector4D c(1.f, 1.f, 0.7f, 1.f);
//    m_program.setAttributeValue(2, c);
    m_ibo->create();
    m_ibo->bind();
    m_ibo->allocate(indexData, 6 * sizeof (GLuint));
//    m_ibo->bind();
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "3 " << error;
    }

	m_tbo->create();
    m_tbo->bind();
    m_tbo->allocate(m_offsets.constData(), m_offsets.size()*sizeof(QVector3D));

    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "4 " << error;
    }
    m_tbo->bind();
    m_program.enableAttributeArray(1);
    m_program.setAttributeArray(1, 0, 3,  sizeof(QVector3D));
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "5 " << error;
    }

      glVertexAttribDivisor(1, 1);
//    glVertexAttribDivisor(2, 1);
    error = glGetError();
    if(error != GL_NO_ERROR) {
       qDebug() << "6 " << error;
    }
    m_vao.bind();
}
void BeanCellItemPrivate::unbindObjects()
{
    m_vao.release();
    m_vao.destroy();
    m_vbo->release();
    m_ibo->release();
    m_tbo->release();
    m_program.disableAttributeArray(0);
    m_program.disableAttributeArray(1);
    m_program.disableAttributeArray(2);
}


BeanCellItem::BeanCellItem() : d_ptr(new BeanCellItemPrivate(this))
{

}
void BeanCellItem::setCell(QSharedPointer<oa::Cell> &cell)
{

}
