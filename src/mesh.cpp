#include "mesh.h"
#include <QDebug>
#include <QColor>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
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
        "layout (location = 0) in vec2 vposition;\n"
        "layout (location = 1) in vec2 vorigin;\n"
        "layout (location = 2) in mat3 vplacement;\n"
        "out vec4 fcolor;\n"
        "void main(void) {\n"
        "   vec2 v = vposition + vorigin;\n"
        "   gl_Position = vec4(vplacement * vec3(v, 1), 1000);\n"
        "   fcolor = vec4(0, 0, 1, 0.8);\n"
        "}\n";
    vshader1->compileSourceCode(vsrc1);

    QOpenGLShader *fshader1 = new QOpenGLShader(QOpenGLShader::Fragment, &m_program);
    const char *fsrc1 =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "void main(void) {\n"
        "   gl_FragColor = fcolor;\n"
        "}\n";
    fshader1->compileSourceCode(fsrc1);

    m_program.addShader(vshader1);
    m_program.addShader(fshader1);
    m_program.link();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    bindData();
}

static qint64 rendered = 0;
const qint64 MaxToRender  =  qint64(1 << 30) * qint64(1 << 30);
void oa::Layout::render(const QTransform &map)
{
    //@{
    // prepare
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(0.5f, 0.4f, 0.7f, .1f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_FRONT_AND_BACK);

    glEnable(GL_DEPTH_TEST);

    m_program.bind();
    //@{
//     QSet<quint64> k;
    for (auto &c : m_cells) {
        Cell *cell = c.m_cell;
        // qDebug() << "mesh" << cell->m_meshes.size();
        // qDebug() << "placement" << cell->m_placements.size();
        for (auto &mesh : cell->m_meshes) {
//             k.insert((quint64(mesh.m_layer) << 32) + mesh.m_datatype);
            render(mesh, map);
            //goto Exit;
        }
//         for (auto &p : cell->m_placements) {
//             render(p, map);
//         }
    }
//     for (auto &i : k) {
//         qDebug() << quint32(i >> 32) << quint32(i & 0xFFFFFFFF);
//     }
    //@}
    rendered = 0;
Exit:
    m_program.release();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

}

// void oa::Layout::render(const Placement &placement, const QTransform &map)
// {
//     auto it = m_cellNameToReference.find(placement.m_cellName);
//     if (it == m_cellNameToReference.end()) {
//         qDebug() << "Bug on missing cell";
//         return;
//     }
//     auto it2 = m_cells.find(it.value());
//     if (it2 == m_cells.end()) {
//         qDebug() << "Bug on missing cell";
//         return;
//     }
//     Cell *cell = it2.value().m_cell;
//     for (auto i = 0; i < placement.m_repetitionCount; ++i) {
//         QTransform t(placement.m_x00, placement.m_x01, 0, placement.m_x10, placement.m_x11, 0,
//                      placement.m_x20 + m_repetitions[placement.m_repetitionOffset + i].m_x,
//                      placement.m_x21 + m_repetitions[placement.m_repetitionOffset + i].m_y, 1.0);
//         t  = map * t;
//         for (auto &mesh : cell->m_meshes) {
//             render(mesh, t);
//         }
//         for (auto &p : cell->m_placements) {
//             render(p, t);
//         }
//     }
// }

bool oa::Layout::reportGlError(const QString &tag)
{
    GLenum error = GL_NO_ERROR;
//     error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << tag << error;
        return true;
    }
    return false;
}
void oa::Layout::render(const Mesh &mesh, const QTransform &transform)
{
    if (mesh.m_repetitionOffset == -1) {
        qDebug() << "REP" <<  mesh.m_vertexCount;
        return;
    }
//     if (rendered > MaxToRender)
//     {
//         return;
//     }
// qDebug() << mesh.m_transformInfos.count();
    rendered ++;
//     m_vao.release();
//     int map = m_program.uniformLocation("vmap");
// //   glVertexAttrib2f(map, mesh.m_x, mesh.m_y);
//     GLfloat mapData[9] = {0};
//     mapData[0] = transform.m11();
//     mapData[1] = transform.m12();
//     mapData[2] = transform.m13();
//     mapData[3] = transform.m21();
//     mapData[4] = transform.m22();
//     mapData[5] = transform.m23();
//     mapData[6] = transform.m31();
//     mapData[7] = transform.m32();
//     mapData[8] = transform.m33();
//     glUniformMatrix3fv(map, 1, GL_FALSE, mapData);

    m_vao.bind();

    static QVector<GLuint> indexData;
    indexData.reserve(mesh.m_vertexCount);
    indexData.resize(mesh.m_vertexCount);
    for (int i = 0; i < mesh.m_vertexCount; ++i) {
        indexData[i] = (mesh.m_baseVertex + i);
    }

    m_ibo->create();
    m_ibo->bind();
    m_ibo->allocate(indexData.constData(), mesh.m_vertexCount * sizeof(GLuint));

       m_vao.bind();
    int originAttr = m_program.attributeLocation("vorigin");

    m_program.enableAttributeArray(originAttr);
    glVertexAttrib2f(originAttr, mesh.m_x, mesh.m_y);
    //> TBO
    int placementAttr = m_program.attributeLocation("vplacement");
    qDebug() << "placementAttr" << placementAttr;
    m_tbo->create();
    m_tbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_tbo->bind();

    for (unsigned int i = 0; i < 3 ; i++) {
        glEnableVertexAttribArray(placementAttr + i);
        glVertexAttribPointer(placementAttr + i, 3, GL_FLOAT, GL_FALSE,  sizeof(TransformInfo), (const GLvoid*)(sizeof(GLfloat) * i * 3));
        glVertexAttribDivisor(placementAttr + i, 1);
    }
    m_ibo->allocate(mesh.m_transformInfos.constData(), sizeof(TransformInfo) * mesh.m_transformInfos.count());
//     glBufferData(GL_ARRAY_BUFFER, sizeof(TransformInfo) * mesh.m_transformInfos.count(), mesh.m_transformInfos.constData(), GL_DYNAMIC_DRAW);

//     for (auto l :  mesh.m_transformInfos)
//     {
//         qDebug() << l.a << l.b << l.c << l.d << l.e << l.f << l.g << l.h << l.i;
//         
//     }
    reportGlError("V5");

    //< TBO



    //Draw
//     qDebug() << "X" << transform;

    glDrawElementsInstanced(GL_TRIANGLE_FAN, mesh.m_vertexCount, GL_UNSIGNED_INT, 0, mesh.m_transformInfos.count());
//     glDrawElementsInstanced(GL_LINE_LOOP, mesh.m_vertexCount , GL_UNSIGNED_INT, 0, mesh.m_transformInfos.count());
//   glDrawElementsInstanced(GL_TRIANGLES, mesh.m_vertexCount, GL_UNSIGNED_INT, 0, mesh.m_transformInfos.count());
    reportGlError("V7");
    m_vao.release();
    m_ibo->release();
    m_tbo->release();
    // m_program.disableAttributeArray(0);
//     m_program.disableAttributeArray(offsetAttr);
    // m_program.disableAttributeArray(2);
}


void oa::Layout::bindData()
{
    m_vao.create();
    reportGlError("V1");
    m_vao.bind();
    m_vbo->create();
    reportGlError("V2");
    m_vbo->bind();
    m_vbo->allocate(m_vertexes.constData(), m_vertexes.count() * sizeof(oa::DeltaValue));
    reportGlError("V3");

    int positionAttr = m_program.attributeLocation("vposition");

    m_program.enableAttributeArray(positionAttr);
    glVertexAttribPointer(positionAttr, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    reportGlError("V4");
    m_vao.release();
}

void oa::Layout::prepareMeshes()
{
    for (auto it = m_cells.cbegin(); it != m_cells.cend(); ++it) {
        Cell *cell = it.value().m_cell;
        for (auto &mesh : cell->m_meshes) {
            mesh.m_transforms[it.key()].append(QTransform());
        }
        qDebug() << "cell->m_placements" << cell->m_placements.count();
        for (auto &p : cell->m_placements) {
            applyPlacement(p, it.key());
        }
    }
    /** if mesh has m_repetitionCount greater than one, then also repeate m_transforms **/
    for (auto it = m_cells.cbegin(); it != m_cells.cend(); ++it) {
        Cell *cell = it.value().m_cell;
        for (auto &mesh : cell->m_meshes) {
            for (int i = 0; i < mesh.m_repetitionCount; ++i) {
                QTransform t;
                DeltaValue &v = m_repetitions[mesh.m_repetitionOffset + i];
                t.translate(v.m_x, v.m_y);
                for (auto it2 = mesh.m_transforms.cbegin(); it2 != mesh.m_transforms.cend(); ++it2) {
                    for (auto it3 = it2.value().cbegin(); it3 != it2.value().cend(); ++it3)
                    {
                        QTransform tmp = it3->inverted();
                        tmp *= t;
                        TransformInfo info;
                        info.a = tmp.m11();
                        info.b = tmp.m12();
                        info.c = tmp.m13();
                        info.d = tmp.m21();
                        info.e = tmp.m22();
                        info.f = tmp.m23();
                        info.g = tmp.m31();
                        info.h = tmp.m32();
                        info.i = tmp.m33();
                        mesh.m_transformInfos.append(info);
                    }
                }
            }
        }
    }
}

void oa::Layout::applyPlacement(const Placement &placement, quint32 cellId, const QTransform &transform)
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
    for (auto i = 0; i < placement.m_repetitionCount; ++i) {
        QTransform t(placement.m_x00, placement.m_x01, 0, placement.m_x10, placement.m_x11, 0,
                     placement.m_x20 + m_repetitions[placement.m_repetitionOffset + i].m_x,
                     placement.m_x21 + m_repetitions[placement.m_repetitionOffset + i].m_y, 1.0);
        t  = transform * t;
        for (auto &mesh : cell->m_meshes) {
            mesh.m_transforms[cellId].append(t);
        }
 
        for (auto &p : cell->m_placements) {
            applyPlacement(p, cellId, t);
        }
    }
}

