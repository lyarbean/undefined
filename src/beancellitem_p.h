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

#ifndef BEANCELLITEM_PRIVATE_H
#define BEANCELLITEM_PRIVATE_H
#include "records.h"
#include "beancellitem.h"
#include <QtCore/qglobal.h>

#include "renderhelper.h"


class BeanCellItemPrivate : public QOpenGLExtraFunctions
{
public:
    BeanCellItemPrivate(BeanCellItem* q);
    virtual ~BeanCellItemPrivate();
    // The following functions called by render thread
    void initialize();
    void render();
    void fillBuffers();
    void bindObjects();
    void unbindObjects();
private:
    BeanCellItem* const q_ptr;
    Q_DECLARE_PUBLIC(BeanCellItem)
    QSharedPointer<oa::Cell> m_cell;
    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    // type : GL_ARRAY_BUFFER => VertexBuffer
    QOpenGLBuffer* m_vbo;
    // type : GL_ARRAY_BUFFER => VertexBuffer
    QOpenGLBuffer* m_tbo;
    // type : GL_ELEMENT_ARRAY_BUFFER => IndexBuffer
    QOpenGLBuffer* m_ibo;
};

#endif // BEANCELLITEM_PRIVATE_H
