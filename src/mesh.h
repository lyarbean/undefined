#ifndef MESH_H
#define MESH_H
#include <QVector>
#include <QMap>
#include <QString>
namespace oa {
// m_drawType is
// case Rectangle | Trapezoid | CTrapezoid => GL_TRIANGLES;
// case Polygon | Circle | Path=> GL_TRIANGLE_FAN;
// case Text => null; // ??
// TODO need to generate images for texts with the help QTextLayout??\
// TODO convert Circle to Polygon

struct Mesh {
    quint32 m_layer;
    quint32 m_datatype;
    qint16 m_oasisType;
    qint16 m_drawType; // GL_TRIANGLES
    qint32 m_baseVertex;
    qint32 m_vertexCount;
    qint32 m_baseIndex;
    // Need dynamic bind
    // PointList m_repetition; // put to m_repetitionsBuffer := {{0,0}, ...}
    qint32 m_repetitionOffset;
    qint32 m_repetitionCount;
    qint32 m_instanceCount;   // m_repetition.size()
    qint32 m_materialIndex;
};
struct Text {
    QString m_string;
    quint32 m_textLayer;
    quint32 m_textType;
    qint64 m_x, m_y;
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
    DeltaValue(qint64 x = 0, qint64 y = 0) : m_x(x), m_y(y) {}
    DeltaValue& operator+=(const DeltaValue &that) {
        this->m_x += that.m_x;
        this->m_y += that.m_y;
        return *this;
    }
    qint64 m_x;
    qint64 m_y;
};

struct Delta23 {
    Delta23(quint64 magnitude = 0, bool isTwo = true);
    DeltaValue value;
};

struct DeltaG {
    DeltaG(quint64 magnitude = 0);
    DeltaG(qint64 x = 0, qint64 y = 0);
    DeltaValue value;
};

// TODO not deltas
using PointList = QVector<DeltaValue>;

using IntervalType = QPair<quint64, quint64>;
class Layout {
public:
    Layout() : m_unit(1) {}
    void render(); // TODO pass World matrix
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
    QMap<qint64, NamedCell> m_cells;
    // auto cell = m_cells[m_cellNameToReference[cellname]];
    QMap<QString, qint64> m_cellNameToReference;
    // Buffers
    // Vertex
    PointList m_repetitions;
    PointList m_vertexes;
};

}
#endif // MESH_H
