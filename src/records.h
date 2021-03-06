#ifndef OA_RECORDS_H
#define OA_RECORDS_H

#include <QSharedPointer>
#include <QVector>
#include <QMap>
// NOTE
// 1. Repetition has absolute coords,
// 2. PointList's is relative to the shape it belong to.
// 3. X and Y are absolute coord

namespace oa {
using IntervalType = QPair<quint64, quint64>;

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

QString image(const DeltaValue&  value);
QString image(PointList* pointList);



// TODO A repetition holds positions to repeate shape.
// To improve rendering, we may just give some positions that fit a given boundry.

struct Repetition
{
    enum {
        Type = 0
    };
    virtual int type() const {
        return Type;
    }
    virtual QVector<DeltaValue> values() {}
};

struct Repetition1 : public Repetition {
    enum {
        Type = 1
    };
    virtual int type() const {
        return Type;
    }
    Repetition1(quint32 dx, quint32 dy, quint32 sx, quint32 sy)
        : m_dx(dx), m_dy(dy), m_sx(sx), m_sy(sy) {}
    virtual QVector<DeltaValue> values() override;

    quint32 m_dx, m_dy, m_sx, m_sy;
};

struct Repetition2 : public Repetition {
    enum {
        Type = 2
    };
    virtual int type() const {
        return Type;
    }
    Repetition2(quint32 dx, quint32 sx) : m_dx(dx), m_sx(sx) {}
    virtual QVector<DeltaValue> values() override;
    // (m_dx+2) length array with space m_sx
    // TODO
    // QVector<QPair<quint32, quint32>> offset;
    // for(int i=0; i < m_dx + 1; ++i)
    //      offset.append({m_sx * i, 0})
    // bounding {0, 0, m_sx * (m_dx + 1), 0}
    //
    quint32 m_dx, m_sx;
};

struct Repetition3 : public Repetition {
    enum {
        Type = 3
    };
    virtual int type() const {
        return Type;
    }
    Repetition3(quint32 dy, quint32 sy) : m_dy(dy), m_sy(sy) {}
    virtual QVector<DeltaValue> values() override;
    // (m_dy+2) length array with space m_sy
    // TODO
    // QVector<QPair<quint32, quint32>> offset;
    // for(int i=0; i < m_dy + 1; ++i)
    //      offset.append({m_sy * i, 0})
    // bounding {0, 0, 0, m_sy * (m_dy + 1)}
    //
    quint32 m_dy, m_sy;
};

struct Repetition4 : public Repetition {
    enum {
        Type = 4
    };
    virtual int type() const {
        return Type;
    }
    Repetition4(quint32 dx, const QVector<quint32>& sxz) : m_dx(dx), m_sxz(sxz) {}
    virtual QVector<DeltaValue> values() override;
    // (m_dy+2) length array with spaces m_syz of size (m_dy + 1), the first element has no space
    quint32 m_dx;
    QVector<quint32> m_sxz;
};

struct Repetition5 : public Repetition {
    enum {
        Type = 5
    };
    virtual int type() const {
        return Type;
    }
    Repetition5(quint32 dx, quint32 g, const QVector<quint32>& sxz) : m_dx(dx), m_g(g), m_sxz(sxz) {}
    virtual QVector<DeltaValue> values() override;
    // Similar to Repetition4, but with m_g multiplied by.
    quint32 m_dx;
    quint32 m_g;
    QVector<quint32> m_sxz;
};

struct Repetition6 : public Repetition {
    enum {
        Type = 6
    };
    virtual int type() const {
        return Type;
    }
    Repetition6(quint32 dy, const QVector<quint32>& syz) : m_dy(dy), m_syz(syz) {}
    virtual QVector<DeltaValue> values() override;
    // Similar to Repetition4, but row and column exchanged.
    quint32 m_dy;
    QVector<quint32> m_syz;
};

struct Repetition7 : public Repetition {
    enum {
        Type = 7
    };
    virtual int type() const {
        return Type;
    }
    Repetition7(quint32 dy, quint32 g, const QVector<quint32>& syz) : m_dy(dy), m_g(g), m_syz(syz) {}
    virtual QVector<DeltaValue> values() override;
    // Similar to Repetition5, but row and column exchanged.
    quint32 m_dy;
    quint32 m_g;
    QVector<quint32> m_syz;
};

struct Repetition8 : public Repetition {
    enum {
        Type = 8
    };
    virtual int type() const {
        return Type;
    }
    Repetition8(quint32 dn, quint32 dm, const DeltaValue& pn, const DeltaValue& pm)
        : m_dn(dn), m_dm(dm), m_pn(pn), m_pm(pm) {}
    virtual QVector<DeltaValue> values() override;
    quint32 m_dn, m_dm;
    DeltaValue m_pn, m_pm;
};

struct Repetition9 : public Repetition {
    enum {
        Type = 9
    };
    virtual int type() const {
        return Type;
    }
    Repetition9(quint32 d, const DeltaValue& p) : m_d(d), m_p(p) {}
    virtual QVector<DeltaValue> values() override;
    quint32 m_d;
    DeltaValue m_p;
};

struct Repetition10 : public Repetition {
    enum {
        Type = 10
    };
    virtual int type() const {
        return Type;
    }
    Repetition10(quint32 d, const QVector<DeltaValue>& pz) : m_d(d), m_pz(pz) {}
    virtual QVector<DeltaValue> values() override;
    quint32 m_d;
    QVector<DeltaValue> m_pz;
};

struct Repetition11 : public Repetition {
    enum {
        Type = 11
    };
    virtual int type() const {
        return Type;
    }
    Repetition11(quint32 d, quint32 g, const QVector<DeltaValue>& pz) : m_d(d), m_g(g), m_pz(pz) {}
    virtual QVector<DeltaValue> values() override;
    quint32 m_d;
    quint32 m_g;
    QVector<DeltaValue> m_pz;
};

QString image(Repetition* repetition);

struct Placement
{
    Placement(): m_manification(0), m_angle(0), m_x(0), m_y(0), m_flip(false), m_repetition(nullptr) {}
    QString m_cellName;
    qreal m_manification;
    qreal m_angle;
    qint64 m_x, m_y;
    bool m_flip;
    QSharedPointer<Repetition> m_repetition;
};

QString image(const Placement& p);

struct Text {
    Text() : m_x(0), m_y(0), m_textLayer(0), m_textType(0), m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_textLayer;
    quint32 m_textType;
    QString m_string;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const Text& t);

struct Rectangle {
    Rectangle() : m_x(0), m_y(0), m_height(0), m_width(0), m_layer(0), m_datatype(0), m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const Rectangle& r);

struct Polygon
{
    Polygon() : m_x(0), m_y(0), m_layer(0), m_datatype(0), m_pointList(nullptr) , m_repetition(nullptr) {}
    qint64 m_x, m_y;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Repetition> m_repetition;

};
QString image(const Polygon& p);

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
QString image(const Path& p);

struct Trapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    bool m_isHorizontal;
    // PointList m_points;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const Trapezoid& t);

struct CTrapezoid {
    qint64 m_x, m_y;
    quint32 m_height, m_width;
    quint32 m_layer;
    quint32 m_datatype;
    quint32 m_type;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const CTrapezoid& t);

struct Circle {
    qint64 m_x, m_y;
    quint32 m_radius;
    quint32 m_layer;
    quint32 m_datatype;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const Circle& c);


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
    qint32 m_attribute;
    QString m_string;
    QSharedPointer<Repetition> m_repetition;
};
QString image(const XGeometry& x);

struct Cell {
    Cell() = default;
    QVector<Rectangle> m_rectangles;
    QVector<Polygon> m_polygons;
    QVector<Path> m_paths;
    QVector<Trapezoid> m_trapezoids;
    QVector<CTrapezoid> m_ctrapezoids;
    QVector<Circle> m_circles;
    QVector<XGeometry> m_xgeometries;
    QVector<Placement> m_placements;
    QVector<Text> m_texts;
    QVector<XELement> m_xelements;
    QVector<QSharedPointer<PointList> > m_pointLists;
    QVector<QSharedPointer<Repetition> > m_repetition;
};

class Layout {
public:
    Layout() : m_unit(1) {}
    void put();
    qreal m_unit;
    QMap<quint32, QString> m_textStrings;
    QMap<quint32, QString> m_propNames;
    QMap<quint32, QString> m_propStrings;
    QMap<quint32, XName> m_xNames;
    QMap<QString, QVector<QPair<IntervalType, IntervalType>>> m_layerDatatypeNames;
    QMap<QString, QVector<QPair<IntervalType, IntervalType>>> m_textLayerDatatypeNames;
    struct NamedCell {
        QString m_name;
        QSharedPointer<Cell> m_cell;
    };
    QMap<qint64, NamedCell> m_cells;
    // auto cell = m_cells[m_cellNameToReference[cellname]];
    QMap<QString, qint64> m_cellNameToReference;
};
}
#endif // OA_RECORDS_H
