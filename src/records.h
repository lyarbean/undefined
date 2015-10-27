#ifndef OA_RECORDS_H
#define OA_RECORDS_H

#include "repetition.h"
#include "pointlist.h"
// #include "cell.h"
#include <QSharedPointer>

namespace oa {
struct Placement
{
    //     quint32 m_referenceNumber;
    //     QString m_cellnameString;
    qint64 m_index;
    qreal m_manification;
    qreal m_angle;
    qint64 m_x, m_y;
    bool m_flip;
    QSharedPointer<Repetition> m_repetition;
};

struct Text {
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
//     QString m_string; // TODO if string to refer is not read yet
    qint64 m_index;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};

struct Rectangle {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};

// ByteInfo 00PXYRDL
// P -> point list
// R -> Repetition
// XYDL -> x, y, datatype and layer. If not specified, use modal variable instead.


struct Polygon
{
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();

};

struct Path {
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_halfWidth;
    quint32 m_startExtension;
    quint32 m_endExtension;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();

};

struct Trapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    Delta1 m_a, m_b;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};

struct CTrapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};

struct Circle {
    qint64 m_x, m_y;
    quint32 m_radius;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};


struct XELement {
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_attribute;
    QString m_string;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();
};

struct XGeometry {
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_attribute;
    QString m_string;
    QSharedPointer<Repetition> m_repetition;
    QVector<QPair<qint64, qint64>> value();

};
}
#endif // OA_RECORDS_H
