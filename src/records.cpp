#include "records.h"
#include <QDebug>
// 7.5.3 A 2-delta is stored as an unsigned-integer and represents a horizontal or vertical displacement. Bits 0-1 encode
// direction: 0 for east, 1 for north, 2 for west, and 3 for south. The remaining bits are the magnitude.

// 7.5.4 A 3-delta is stored as an unsigned-integer and represents a horizontal, vertical, or 45-degree diagonal displace-
// ment. Bits 0-2 encode direction: 0 for east, 1 for north, 2 for west, 3 for south, 4 for northeast, 5 for northwest, 6 for
// southwest, and 7 for southeast. The remaining bits are the magnitude (for horizontal and vertical deltas) or the magni-
// tude of the projection onto the x- or y-axis (for 45-degree deltas).

oa::Delta23::Delta23(quint64 magnitude, bool isTwo)
{
    if (isTwo) {
        qint64 m = magnitude >> 2;
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
        qint64 m = magnitude >> 3;
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
    qint64 m = static_cast<qint64>(magnitude >> 4);
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

oa::DeltaG::DeltaG(qint64 x, qint64 y)
{
    value = {x, y};
}

QString oa::image(const DeltaValue &value)
{
    return QString("(%1, %2)").arg(value.m_x).arg(value.m_y);
}
QString oa::image(PointList *pointList)
{
    QString s("PointList{");
    if (pointList) {
        for (auto &p : *pointList) {
            s += image(p);
        }
    }
    s += "}";
    return s;
}

// TODO optimize
// m_dx := N-2, m_dy := M-2, N x M
QVector<oa::DeltaValue> oa::Repetition1::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve((m_dx + 2) * (m_dy + 2));
    for (int i = 0; i < m_dx + 2; ++i) {
        for (int j = 0; j < m_dy + 2; ++j) {
            offsets.append(oa::DeltaValue(m_sx * i, m_sy * j));
        }
    }
    return offsets;
}

// m_dx := N-2,  N x 1
QVector<oa::DeltaValue> oa::Repetition2::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dx + 2);
    for (int i = 0; i < m_dx + 2; ++i) {
        offsets.append(oa::DeltaValue(m_sx * i, 0));
    }
    return offsets;
}

// m_dy := M-2,  1 x M
QVector<oa::DeltaValue> oa::Repetition3::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dy + 2);
    for (int i = 0; i < m_dy + 2; ++i) {
        offsets.append(oa::DeltaValue(0, m_sy * i));
    }
    return offsets;
}

// m_dx := N-2,  N x 1
QVector<oa::DeltaValue> oa::Repetition4::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dx + 2);
    qint64 space = 0;
    offsets.append(oa::DeltaValue(space, 0));
    for (int i = 0; i < m_dx + 1; ++i) {
        space += m_sxz[i];
        offsets.append(oa::DeltaValue(space, 0));
    }
    return offsets;
}

// m_dx := N-2,  N x 1
QVector<oa::DeltaValue> oa::Repetition5::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dx + 2);
    qint64 space = 0;
    offsets.append(oa::DeltaValue(space, 0));
    for (int i = 0; i < m_dx + 1; ++i) {
        space += m_sxz[i];
        offsets.append(oa::DeltaValue(space * m_g, 0));
    }
    return offsets;
}

QVector<oa::DeltaValue> oa::Repetition6::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dy + 2);
    qint64 space = 0;
    offsets.append(oa::DeltaValue(0, space));
    for (int i = 0; i < m_dy + 1; ++i) {
        space += m_syz[i];
        offsets.append(oa::DeltaValue(0, space));
    }
    return offsets;
}

QVector<oa::DeltaValue> oa::Repetition7::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_dy + 2);
    qint64 space = 0;
    offsets.append(oa::DeltaValue(0, space));
    for (int i = 0; i < m_dy + 1; ++i) {
        space += m_syz[i];
        offsets.append(oa::DeltaValue(space * m_g, 0));
    }
    return offsets;
}

// m_dn = N-2, m_dm = M-2; N x M
QVector<oa::DeltaValue> oa::Repetition8::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve((m_dn + 2) * (m_dm + 2));
    for (int i = 0; i < m_dn + 2; ++i) {
        for (int j = 0; j < m_dm + 2; ++j) {
            offsets.append(oa::DeltaValue(i * m_pn.m_x + j * m_pm.m_x, i * m_pn.m_y + j * m_pm.m_y));
        }
    }
    return offsets;
}
// m_d = P-2, P x 1
QVector<oa::DeltaValue> oa::Repetition9::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_d + 2);
    for (int i = 0; i < m_d + 2; ++i) {
        offsets.append(oa::DeltaValue(i * m_p.m_x, i * m_p.m_y));
    }
    return offsets;
}
QVector<oa::DeltaValue> oa::Repetition10::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_d + 2);
    qint64 spaceX = 0;
    qint64 spaceY = 0;
    offsets.append(oa::DeltaValue(spaceX, spaceY));
    for (int i = 0; i < m_d + 1; ++i) {
        spaceX += m_pz[i].m_x;
        spaceY += m_pz[i].m_y;
        offsets.append(oa::DeltaValue(spaceX, spaceY));
    }
    return offsets;
}

QVector<oa::DeltaValue> oa::Repetition11::values()
{
    QVector<oa::DeltaValue> offsets;
    offsets.reserve(m_d + 2);
    qint64 spaceX = 0;
    qint64 spaceY = 0;
    offsets.append(oa::DeltaValue(spaceX, spaceY));
    for (int i = 0; i < m_d + 1; ++i) {
        spaceX += m_pz[i].m_x;
        spaceY += m_pz[i].m_y;
        offsets.append(oa::DeltaValue(spaceX * m_g, spaceY * m_g));
    }
    return offsets;
}

QString oa::image(Repetition *repetition)
{
    if (!repetition) {
        return QString("No repetition");
    }
    switch (repetition->type()) {
    case 1: {
        Repetition1 *r = static_cast<Repetition1 *>(repetition);
        return QString("Repetition1{%1, %2, %3, %4}").arg(r->m_dx).arg(r->m_dy).arg(r->m_sx).arg(r->m_sy);
    }
    case 2: {
        Repetition2 *r = static_cast<Repetition2 *>(repetition);
        return QString("Repetition2{%1, %2}").arg(r->m_dx).arg(r->m_sx);
    }
    case 3: {
        Repetition3 *r = static_cast<Repetition3 *>(repetition);
        return QString("Repetition3{%1, %2}").arg(r->m_dy).arg(r->m_sy);
    }
    case 4: {
        Repetition4 *r = static_cast<Repetition4 *>(repetition);
        QString s("Repetition4{");
        s += QString::number(r->m_dx);
        s += ", (";
        for (auto z : r->m_sxz) {
            s += QString::number(z);
            s += " ";
        }
        s += ")}";
        return s;
    }
    case 5: {
        Repetition5 *r = static_cast<Repetition5 *>(repetition);
        QString s("Repetition5{");
        s += QString::number(r->m_dx);
        s += ", ";
        s += QString::number(r->m_g);
        s += ", (";
        for (auto z : r->m_sxz) {
            s += QString::number(z);
            s += ", ";
        }
        s += ")}";
        return s;
    }
    case 6: {
        Repetition6 *r = static_cast<Repetition6 *>(repetition);
        QString s("Repetition4{");
        s += QString::number(r->m_dy);
        s += ", (";
        for (auto z : r->m_syz) {
            s += QString::number(z);
            s += ", ";
        }
        s += ")}";
        return s;
    }
    case 7: {
        Repetition7 *r = static_cast<Repetition7 *>(repetition);
        QString s("Repetition7{");
        s += QString::number(r->m_dy);
        s += ", ";
        s += QString::number(r->m_g);
        s += ", (";
        for (auto z : r->m_syz) {
            s += QString::number(z);
            s += ", ";
        }
        s += ")}";
        return s;
    }
    case 8: {
        Repetition8 *r = static_cast<Repetition8 *>(repetition);
        return QString("Repetition8{(%1, %2), (%3, %4), (%5, %6)}").arg(r->m_dn).arg(r->m_dm).arg(r->m_pn.m_x).arg(r->m_pn.m_x).arg(r->m_pm.m_x).arg(r->m_pm.m_y);
    }
    case 9: {
        Repetition9 *r = static_cast<Repetition9 *>(repetition);
        return QString("Repetition9{%1, (%2, %3)}").arg(r->m_d).arg(r->m_p.m_x).arg(r->m_p.m_y);
    }
    case 10: {
        Repetition10 *r = static_cast<Repetition10 *>(repetition);
        QString s("Repetition10{");
        s += QString::number(r->m_d);
        s += ", (";
        for (auto z : r->m_pz) {
            s += image(z);
            s += ", ";
        }
        s += ")}";
        return s;
    }
    case 11: {
        Repetition11 *r = static_cast<Repetition11 *>(repetition);
        QString s("Repetition10{");
        s += QString::number(r->m_d);
        s += ", ";
        s += QString::number(r->m_g);
        s += ", (";
        for (auto &z : r->m_pz) {
            s += image(z);
            s += ", ";
        }
        s += ")}";
        return s;
    }
    default:
        return "";
    }
}

QString oa::image(const Placement &p)
{
    return QString("Placement{N%1, M%2, A%3, X%4, Y%5, F%6, %7}")
           .arg(p.m_cellName)
           .arg(p.m_manification)
           .arg(p.m_angle)
           .arg(p.m_x)
           .arg(p.m_y)
           .arg(p.m_flip)
           .arg(image(p.m_repetition.data()));
}

QString oa::image(const Text &t)
{
    return QString("Text{X%1, Y%2, L%3, D%4, S[%5], %6}")
           .arg(t.m_x)
           .arg(t.m_y)
           .arg(t.m_textLayer)
           .arg(t.m_textType)
           .arg(t.m_string)
           .arg(image(t.m_repetition.data()));
}

QString oa::image(const Rectangle &r)
{
    return QString("Rectangle{X%1, Y%2, H%3, W%4, L%5, D%6, %7}")
           .arg(r.m_x)
           .arg(r.m_y)
           .arg(r.m_height)
           .arg(r.m_width)
           .arg(r.m_layer)
           .arg(r.m_datatype)
           .arg(image(r.m_repetition.data()));
}

QString oa::image(const Polygon &p)
{
    return QString("Polygon{X%1, Y%2, L%3, D%4, %5, %6}")
           .arg(p.m_x)
           .arg(p.m_y)
           .arg(p.m_layer)
           .arg(p.m_datatype)
           .arg(image(p.m_pointList.data()))
           .arg(image(p.m_repetition.data()));
}


QString oa::image(const Path &p)
{
    return QString("Path{X%1, Y%2, L%3, D%4, HW%5, S%6, E%7, %8, %9}")
           .arg(p.m_x)
           .arg(p.m_y)
           .arg(p.m_layer)
           .arg(p.m_datatype)
           .arg(p.m_halfWidth)
           .arg(p.m_startExtension)
           .arg(p.m_endExtension)
           .arg(image(p.m_pointList.data()))
           .arg(image(p.m_repetition.data()));
}

QString oa::image(const Trapezoid &t)
{
    return QString("Trapezoid{X%1, Y%2, H%3, W%4, L%5, D%6, %7}")
           .arg(t.m_x)
           .arg(t.m_y)
           .arg(t.m_height)
           .arg(t.m_width)
           .arg(t.m_layer)
           .arg(t.m_datatype)
           // .arg(image(&t.m_points))
           .arg(image(t.m_repetition.data()));
}

QString oa::image(const CTrapezoid &t)
{
    return QString("CTrapezoid{X%1, Y%2, H%3, W%4, L%5, D%6, %7}")
           .arg(t.m_x)
           .arg(t.m_y)
           .arg(t.m_height)
           .arg(t.m_width)
           .arg(t.m_layer)
           .arg(t.m_datatype)
           // .arg(image(&t.m_points))
           .arg(image(t.m_repetition.data()));
}

QString oa::image(const Circle &c)
{
    return QString("Circle{X%1, Y%2, R%3, L%4, D%5, %6}")
           .arg(c.m_x)
           .arg(c.m_y)
           .arg(c.m_radius)
           .arg(c.m_layer)
           .arg(c.m_datatype)
           .arg(image(c.m_repetition.data()));
}

QString oa::image(const XGeometry &x)
{
    return QString("XGeometry{X%1, Y%2, L%3, D%4, A%5, S[%6], %7}")
           .arg(x.m_x)
           .arg(x.m_y)
           .arg(x.m_layer)
           .arg(x.m_datatype)
           .arg(x.m_attribute)
           .arg(x.m_string)
           .arg(image(x.m_repetition.data()));
}

void oa::Layout::put()
{
    for (auto &c : m_cells) {
        qDebug() << "Cell of " << c.m_name;
        if (!c.m_cell) {
            qDebug() << "Empty Cell";
        } else {
            if (!c.m_cell->m_rectangles.empty()) {
                qDebug() << "Rectangles";
                for (auto &r : c.m_cell->m_rectangles) {
                    qDebug() << image(r);
                }
            }
            if (!c.m_cell->m_polygons.empty()) {
                qDebug() << "Polygons";
                for (auto &p : c.m_cell->m_polygons) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_paths.empty()) {
                qDebug() << "Paths";
                for (auto &p : c.m_cell->m_paths) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_trapezoids.empty()) {
                qDebug() << "Trapezoids";
                for (auto &p : c.m_cell->m_trapezoids) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_ctrapezoids.empty()) {
                qDebug() << "CTrapezoids";
                for (auto &p : c.m_cell->m_ctrapezoids) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_circles.empty()) {
                qDebug() << "Circles";
                for (auto &p : c.m_cell->m_circles) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_xgeometries.empty()) {
                qDebug() << "XGeometries";
                for (auto &p : c.m_cell->m_xgeometries) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_placements.empty()) {
                qDebug() << "Placements";
                for (auto &p : c.m_cell->m_placements) {
                    qDebug() << image(p);
                }
            }
            if (!c.m_cell->m_texts.empty()) {
                qDebug() << "Texts";
                for (auto &t : c.m_cell->m_texts) {
                    qDebug() << image(t);
                }
            }
        }
    }
}

