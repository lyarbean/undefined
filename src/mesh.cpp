#include "mesh.h"
#include <QDebug>
#include <QColor>
// 7.5.3 A 2-delta is stored as an unsigned-integer and represents a horizontal or vertical displacement. Bits 0-1 encode
// direction: 0 for east, 1 for north, 2 for west, and 3 for south. The remaining bits are the magnitude.

// 7.5.4 A 3-delta is stored as an unsigned-integer and represents a horizontal, vertical, or 45-degree diagonal displace-
// ment. Bits 0-2 encode direction: 0 for east, 1 for north, 2 for west, 3 for south, 4 for northeast, 5 for northwest, 6 for
// southwest, and 7 for southeast. The remaining bits are the magnitude (for horizontal and vertical deltas) or the magni-
// tude of the projection onto the x- or y-axis (for 45-degree deltas).

oa::Delta23::Delta23(quint64 magnitude, bool isTwo)
{
    if (isTwo) {
        qint32 m = magnitude >> 2;
        switch (magnitude & 3) {
        case 0:
            value = {m, 0};
            break;
        case 1:
            value = {0, m};
            break;
        case 2:
            value = { -m, 0};
            break;
        case 3:

        default:
            value = {0, - m};
            break;
        }
    } else {
        qint32 m = magnitude >> 3;
        switch (magnitude & 7) {
        case 0:
            value = {m, 0};
            break;
        case 1:
            value = {0, m};
            break;
        case 2:
            value = { -m, 0};
            break;
        case 3:
            value = {0, -m};
            break;
        case 4:
            value = {m, m};
            break;
        case 5:
            value = { -m, m};
            break;
        case 6:
            value = { -m, -m};
            break;
        case 7:
        default:
            value = {m, -m};
            break;
        }
    }
}

// 7.5.5 A g-delta has two alternative forms and is stored either as a single unsigned-integer or as a pair of unsigned-
// integers. The first form is indicated when bit 0 is zero, and represents a horizontal, vertical, or 45-degree diagonal dis-
// placement, with bits 1-3 encoding direction, and the remaining bits storing the magnitude, in the same fashion as a 3-
// delta. The second form represents a general (x,y) displacement and is a pair of unsigned-integers. Bit 0 of the first
// integer is 1. Bit 1 of the first integer is the x-direction (0 for east, 1 for west). The remaining bits of the first integer
// represent the magnitude in the x-direction. Bit 0 of the second integer is the y-direction (0 for north, 1 for south). The
// remaining bits of the second integer represent the magnitude in the y-direction. Both forms may appear in a list of g-
// deltas.

oa::DeltaG::DeltaG(quint64 magnitude)
{
    qint32 m = static_cast<qint32>(magnitude >> 4);
    switch ((magnitude >> 1) & 7) {
    case 0:
        value = {m, 0};
        break;
    case 1:
        value = {0, m};
        break;
    case 2:
        value = { -m, 0};
        break;
    case 3:
        value = {0, -m};
        break;
    case 4:
        value = {m, m};
        break;
    case 5:
        value = { -m, m};
        break;
    case 6:
        value = { -m, -m};
        break;
    case 7:
    default:
        value = {m, -m};
        break;
    }
}

oa::DeltaG::DeltaG(qint32 x, qint32 y)
{
    value = {x, y};
}

void oa::Layout::initializeRender()
{
    initializeOpenGLFunctions();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup Shader
    QOpenGLShader *vshader1 = new QOpenGLShader(QOpenGLShader::Vertex, &m_program);
    const char *vsrc1 =
        "#version 330\n"
        "layout(location = 0) in vec2 vposition;\n"
        "layout(location = 1) in vec2 voffset;\n"
        "layout(location = 2) in vec4 vcolor;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "   fcolor = vec4(1, 0, 0, .1);\n"
        "   gl_Position = vec4((vposition + voffset), 0, 1);\n"
        "}\n";
    vshader1->compileSourceCode(vsrc1);

    QOpenGLShader *fshader1 = new QOpenGLShader(QOpenGLShader::Fragment, &m_program);
    const char *fsrc1 =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = fcolor;\n"
        "}\n";
    fshader1->compileSourceCode(fsrc1);

    m_program.addShader(vshader1);
    m_program.addShader(fshader1);
    m_program.link();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void oa::Layout::render()
{
    //@{
    // prepare
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.5f, 0.4f, 0.7f, .1f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFrontFace(GL_CW);
   glCullFace(GL_FRONT);
   glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    m_program.bind();
    //@{
    for (auto &c : m_cells) {
        Cell *cell = c.m_cell;
        for (auto &mesh : cell->m_meshes) {
            render(mesh);
        }
        for (auto &p : cell->m_placements) {
            render(p);
        }
    }
    //@}
    m_program.release();
    glDisable(GL_DEPTH_TEST);

}

void oa::Layout::render(const Placement &placement)
{
    auto it = m_cellNameToReference.find(placement.m_cellName);
    if (it == m_cellNameToReference.end()) {
        qDebug() << "Bug on missing cell";
        return;
    }
    auto it2 = m_cells.find(it.value());
    if (it2 == m_cells.end()) {
        qDebug() << "Bug on missing cell";
        return;
    }
    Cell *cell = it2.value().m_cell;
    for (auto &matrix : placement.m_matrixes) {
        for (auto &mesh : cell->m_meshes) {
            render(mesh);
        }
        for (auto &p : cell->m_placements) {
            render(p);
        }
    }
}

void oa::Layout::render(const Mesh &mesh)
{

    // bindObject
    GLenum error = GL_NO_ERROR;
    m_vao.create();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v1 " << error;
    }
    m_vao.bind();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v2 " << error;
    }
    m_vbo->create();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v3 " << error;
    }
//     for (int i = 0; i < mesh.m_vertexCount; ++i)
//     {
//        auto& p = m_vertexes[mesh.m_baseVertex +i];
//        qDebug() << p.m_x << p.m_y;
//     }
    for (int i = 0; i < mesh.m_repetitionCount; ++i)
    {
       auto& p = m_repetitions[mesh.m_repetitionOffset +i];
       qDebug() << p.m_x << p.m_y;
    }
//     qDebug() << "yy" << mesh.m_baseVertex <<  mesh.m_vertexCount ;
//     m_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vbo->bind();
    m_vbo->allocate(m_vertexes.constData() + mesh.m_baseVertex, mesh.m_vertexCount * sizeof(oa::DeltaValue));
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v4 " << error;
    }

    m_program.enableAttributeArray(0);
    m_program.setAttributeArray(0, 0, 2, sizeof(oa::DeltaValue));
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v5 " << error;
    }

    // TODO indices
    static GLuint indexData[] = {
        0, 1, 2,
        2, 3, 0,
    };


    m_ibo->create();
    m_ibo->bind();
    m_ibo->allocate(indexData, 6 * sizeof(GLuint));

    m_tbo->create();
    if (mesh.m_repetitionOffset > -1) {
//         qDebug() << "xx" << mesh.m_repetitionOffset << mesh.m_repetitionCount << m_repetitions.size();

//         m_tbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
        m_tbo->bind();
        m_tbo->allocate(m_repetitions.constData() +  mesh.m_repetitionOffset, mesh.m_repetitionCount * sizeof(oa::DeltaValue));
        error = glGetError();
        if (error != GL_NO_ERROR) {
            qDebug() << "v6 " << error;
        }
        m_program.enableAttributeArray(1);
        m_program.setAttributeArray(1, 0, 2, sizeof(oa::DeltaValue));


        error = glGetError();
        if (error != GL_NO_ERROR) {
            qDebug() << "v7 " << error;
        }
        glVertexAttribDivisor(1, 1);
        //    glVertexAttribDivisor(2, 1);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            qDebug() << "v8 " << error;
        }
    }
    //m_program.setAttributeValue(2, QColor(Qt::red));
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v8 " << error;
    }
//     m_vao.bind();
    //Draw
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v9 " << error ;
    }
        m_vbo->bind();
    if (mesh.m_repetitionOffset > -1) {
//         qDebug() << ". " ;
        glDrawElementsInstanced(GL_LINE_LOOP, mesh.m_vertexCount *  sizeof(oa::DeltaValue) , GL_UNSIGNED_INT, nullptr, mesh.m_repetitionCount);
    }
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "v10 " << error;
    }
    
    m_vao.release();

    m_vbo->release();
    m_ibo->release();
    m_tbo->release();
        m_vao.destroy();
    m_program.disableAttributeArray(0);
    m_program.disableAttributeArray(1);
    m_program.disableAttributeArray(2);
}






