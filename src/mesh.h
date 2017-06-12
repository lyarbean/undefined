#ifndef MESH_H
#define MESH_H
#include <QVector>
#include <QMap>
#include <QString>
#include <QTransform> 
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
namespace oa {
// m_drawType is
// case Rectangle | Trapezoid | CTrapezoid => GL_TRIANGLES;
// case Polygon | Circle | Path=> GL_TRIANGLE_FAN;
// case Text => null; // ??
// TODO need to generate images for texts with the help QTextLayout??\
// TODO convert Circle to Polygon

struct Transform {
    float m_x00, m_x01, m_x10, m_x11, m_x20, m_x21;
};

struct TransformInfo {
    float a, b, c, d, e, f, g, h, i; // matrix
};

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
    float m_x = 0;
    float m_y = 0;
    // key is the cell
    QMap<qint32, QVector<QTransform>> m_transforms;
    QVector<TransformInfo> m_transformInfos;
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
    float m_x00, m_x01, m_x10, m_x11, m_x20, m_x21;
    qint32 m_repetitionOffset = 0;
    qint32 m_repetitionCount = 1;
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
	float m_x;
	float m_y;
    //qint32 m_x;
    //qint32 m_y;
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
class Layout : public QOpenGLExtraFunctions {
public:
    Layout() : m_unit(1) {
        m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        m_tbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

        m_ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    }
    void initializeRender();
    void render(const QTransform& map = QTransform()); // TODO pass World matrix
//     void render(const Placement& placement, const QTransform& map = QTransform());
    void render(const Mesh& mesh, const QTransform& map = QTransform());
    void prepareMeshes();
    void applyPlacement(const Placement& placement, quint32 cellId, const QTransform& transform = QTransform());
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
