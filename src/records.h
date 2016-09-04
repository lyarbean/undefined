#ifndef OA_RECORDS_H
#define OA_RECORDS_H

#include "repetition.h"
#include "pointlist.h"

#include <QSharedPointer>


// NOTE
// 1. Repetition has absolute coords,
// 2. PointList's is relative to the shape it belong to.
// 3. X and Y are absolute coord

namespace oa {
struct Placement
{
    Placement(): m_index(0), m_manification(0), m_angle(0), m_x(0), m_y(0), m_flip(0), m_repetition(nullptr) {}
    quint32 m_index;
    qreal m_manification;
    qreal m_angle;
    qint64 m_x, m_y;
    bool m_flip;
    QSharedPointer<Repetition> m_repetition;
};

struct Text {
    Text() : m_x(0), m_y(0), m_layer(0), m_datatype(0), m_index(0), m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_index; // Layout::m_localTextStrings or Layout::m_textStrings
    QSharedPointer<Repetition> m_repetition;
};

struct Rectangle {
    Rectangle() : m_x(0), m_y(0), m_height(0), m_width(0), m_layer(0), m_datatype(0), m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
};

// ByteInfo 00PXYRDL
// P -> point list
// R -> Repetition
// XYDL -> x, y, datatype and layer. If not specified, use modal variable instead.


struct Polygon
{
    Polygon() : m_x(0), m_y(0), m_layer(0), m_datatype(0), m_pointList(nullptr) , m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Repetition> m_repetition;

};

struct Path {
    Path()
    : m_x(0), m_y(0), m_layer(0), m_datatype(0), m_halfWidth(0)
    , m_startExtension(0), m_endExtension(0), m_pointList(nullptr), m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_halfWidth;
    qint32 m_startExtension;
    qint32 m_endExtension;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Repetition> m_repetition;

};

struct Trapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    Delta1 m_a, m_b;
    QSharedPointer<Repetition> m_repetition;
};

struct CTrapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_type;
    QSharedPointer<Repetition> m_repetition;
};

struct Circle {
    qint64 m_x, m_y;
    quint32 m_radius;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
};

struct XName {
    quint32 m_attribute;
    QString m_string;
};

struct XELement {
    quint32 m_attribute;
    QString m_string;
};

struct XGeometry {
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_attribute;
    QString m_string;
    QSharedPointer<Repetition> m_repetition;
};
}
#endif // OA_RECORDS_H
