#ifndef MESH_H
#define MESH_H
#include <QVector>
#include <QMap>
#include <QString>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
//#include <QOpenGLFunctions_3_2_Compatibility> 
#include <QOpenGLFunctions_3_2_Core>
namespace oa {
// m_drawType is
// case Rectangle | Trapezoid | CTrapezoid => GL_TRIANGLES;
// case Polygon | Circle | Path=> GL_TRIANGLE_FAN;
// case Text => null; // ??
// TODO need to generate images for texts with the help QTextLayout??\
// TODO convert Circle to Polygon

struct Mesh {
    quint32 m_layer = 0;
    quint32 m_datatype = 0;
    qint16 m_oasisType = 0;
    qint16 m_drawType = 0; // GL_TRIANGLES
    qint32 m_baseVertex = 0;
    qint32 m_vertexCount = 0;
    qint32 m_baseIndex = -1;
    qint32 m_repetitionOffset = 0;
    qint32 m_repetitionCount = 1;
    qint32 m_instanceCount = 0;   // m_repetition.size()
    qint32 m_materialIndex = -1;
};
struct Text {
    QString m_string;
    quint32 m_textLayer;
    quint32 m_textType;
    qint32 m_x, m_y;
    qint32 m_repetitionOffset;
    qint32 m_repetitionCount;
};
struct Placement {
    QString m_cellName;
    // x00 = cos(t) * mag
    // x01 = sin(t) * mag
    // x10 = -f * x01
    // x11 = f * x00
    // x20 = x + x_r
    // x21 = y + x_r
    // f =  -1 ** (F mod 2)
    struct X {
        qint32 x00, x01, x10, x11, x20, x21;
    };
    QVector<X> m_matrixes;
};
struct Cell {
    Cell() = default;
    QVector<Mesh> m_meshes;
    QVector<Placement> m_placements;
    QVector<Text> m_texts;
};

struct DeltaValue {
    DeltaValue(qint32 x = 0, qint32 y = 0) : m_x(x), m_y(y) {}
    DeltaValue& operator+=(const DeltaValue &that) {
        this->m_x += that.m_x;
        this->m_y += that.m_y;
        return *this;
    }
    qint32 m_x;
    qint32 m_y;
};

struct Delta23 {
    Delta23(quint64 magnitude = 0, bool isTwo = true);
    DeltaValue value;
};

struct DeltaG {
    DeltaG(quint64 magnitude = 0);
    DeltaG(qint32 x = 0, qint32 y = 0);
    DeltaValue value;
};

// TODO not deltas
using PointList = QVector<DeltaValue>;

using IntervalType = QPair<quint32, quint32>;
class Layout  : public QOpenGLExtraFunctions {
public:
    Layout() : m_unit(1) {
        m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_tbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    }
    void initializeRender();
    void render(); // TODO pass World matrix
    void render(Cell *cell, QVector<Placement::X>& matrixes);
    void render(const Placement& placement);
    void render(const Mesh& mesh);
	void bindData();
	bool reportGlError(const QString &tag);
//     void put();
    qreal m_unit;
    QMap<quint32, QString> m_textStrings;
    QMap<quint32, QString> m_propNames;
    QMap<quint32, QString> m_propStrings;
    // QMap<quint32, XName> m_xNames;
    QMap<QString, QVector<QPair<IntervalType, IntervalType>>> m_layerDatatypeNames;
    QMap<QString, QVector<QPair<IntervalType, IntervalType>>> m_textLayerDatatypeNames;
    struct NamedCell {
        QString m_name;
        Cell* m_cell;
    };
    QMap<qint32, NamedCell> m_cells;
    // auto cell = m_cells[m_cellNameToReference[cellname]];
    QMap<QString, qint32> m_cellNameToReference;
    // Buffers
    // Vertex
    PointList m_repetitions;
    PointList m_vertexes;
    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    // type : GL_ARRAY_BUFFER => VertexBuffer
    QOpenGLBuffer* m_vbo;
    // type : GL_ARRAY_BUFFER => VertexBuffer
    QOpenGLBuffer* m_tbo;
    // type : GL_ELEMENT_ARRAY_BUFFER => IndexBuffer
    QOpenGLBuffer* m_ibo;
};

}
#endif // MESH_H
