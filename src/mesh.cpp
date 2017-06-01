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
		"#version 120\n"
        "attribute highp ivec2 vposition;\n"
        "attribute highp ivec2 voffset;\n"
        "varying mediump vec4 fcolor;\n"
        "void main(void) {\n"
        "   gl_Position = vec4((vposition + voffset) - 1000, 1, 4000);\n"
        "   fcolor = vec4((vposition / 100)*0.1, 0, 1);\n"
        "}\n";
    vshader1->compileSourceCode(vsrc1);

    QOpenGLShader *fshader1 = new QOpenGLShader(QOpenGLShader::Fragment, &m_program);
    const char *fsrc1 =
		"#version 120\n"
        "varying mediump vec4 fcolor;\n"
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

void oa::Layout::render()
{
    //@{
    // prepare
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glClearColor(0.5f, 0.4f, 0.7f, .1f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glDisable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
    //glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);

    m_program.bind();
    //@{
    for (auto &c : m_cells) {
        Cell *cell = c.m_cell;
        for (auto &mesh : cell->m_meshes) {
            render(mesh);
			//goto Exit;
        }
        for (auto &p : cell->m_placements) {
            render(p);
        }
    }
    //@}
	Exit:
    m_program.release();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

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
			return;
        }
        for (auto &p : cell->m_placements) {
            render(p);
        }
    }
}

bool oa::Layout::reportGlError(const QString &tag) {
    GLenum error = GL_NO_ERROR;
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << tag << error;
		return true;
    }
	return false;
}
void oa::Layout::render(const Mesh &mesh)
{
    if (mesh.m_repetitionOffset == -1) {
		qDebug() << "REP" <<  mesh.m_vertexCount;
        return;
    }
	m_vao.bind();
	QVector<GLuint> indexData;
	int slice = mesh.m_vertexCount / 4;
	if (mesh.m_vertexCount % 4)
	{
		qDebug() << mesh.m_vertexCount;
	}
	for (int i = 0; i < slice ; ++i)
	{
		indexData.append(i*4);
		indexData.append(i*4+1);
		indexData.append(i*4+2);
		indexData.append(i*4);
		indexData.append(i*4+2);
		indexData.append(i*4+3);
	}

    m_ibo->create();
    m_ibo->bind();
    m_ibo->allocate(indexData.constData(), indexData.size() * sizeof(GLuint));

    m_tbo->create();
    if (mesh.m_repetitionOffset > -1) {
        m_tbo->bind();
        m_tbo->allocate(m_repetitions.constData() + mesh.m_repetitionOffset, mesh.m_repetitionCount * sizeof(oa::DeltaValue));
		int offsetAttr = m_program.attributeLocation("vposition");
        m_program.enableAttributeArray(offsetAttr);

		glVertexAttribPointer(offsetAttr, 2, GL_INT, GL_FALSE, 0, nullptr);
		reportGlError("V5");
        //glVertexAttribDivisor(1, 1);
    }
    
    //Draw
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, mesh.m_vertexCount * 2 , GL_UNSIGNED_INT, nullptr, mesh.m_repetitionCount, mesh.m_baseVertex);
	reportGlError("V7");
	m_vao.release();

    //m_vbo->release();
    m_ibo->release();
    m_tbo->release();
   // m_program.disableAttributeArray(0);
    m_program.disableAttributeArray(1);
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
    m_vbo->allocate(m_vertexes.constData(), m_vertexes.size() * sizeof(oa::DeltaValue));
	reportGlError("V3");

	int positionAttr = m_program.attributeLocation("vposition");
	
    m_program.enableAttributeArray(positionAttr);
	glVertexAttribPointer(positionAttr, 2, GL_INT, GL_FALSE, 0, nullptr); 
	reportGlError("V4");
	m_vao.release();
}




