/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright 2015  颜烈彬 <slbyan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "pointlist.h"
#include <QtGlobal>

QVector<QPair<qint64, qint64> > oa::PointList0::value(qint64 x, qint64 y, bool isPolygon) const
{
    int count = m_flags.count();
    if (isPolygon && count < 2) {
        qFatal("Requires more (>=2) points");
    }
    QPair<qint64, qint64> current(x,y);
    QVector<QPair<qint64, qint64>> result;
    result.append(current);
    int orientation = type() == 0 ? DeltaDirection::Horizontal : DeltaDirection::Vertical;
    for (int i = 0; i < count; ++i) {
        auto delta = m_points[i].value(orientation & m_flags.at(i));
        orientation = ~orientation;
        current.first += delta.first;
        current.second += delta.second;
        result.append(current);
    }
    if (isPolygon) {
        // Now, add the last point.
        if (orientation & DeltaDirection::Horizontal) {
            current.first = x;
        } else {
            current.second = y;
        }
        result.append(current);
    }
    return result;
}

QVector<QPair<qint64, qint64> > oa::PointList23::value(qint64 x, qint64 y, bool isPolygon) const
{
    int count = m_flags.count();
    if (isPolygon && count < 2) {
        qFatal("Requires more (>=2) points");
    }
    QPair<qint64, qint64> current(x,y);
    QVector<QPair<qint64, qint64>> result;
    result.append(current);
    for (int i = 0; i < count; ++i) {
        auto delta = m_points[i].value(m_flags.at(i));
        current.first += delta.first;
        current.second += delta.second;
        result.append(current);
    }
    // TODO For type 2
//     Q_ASSERT((current.first == x || current.second == y ||
//               (qAbs(current.first - x) == qAbs(current.second - y)));             // For type 3
    return result;
}

QVector<QPair<qint64, qint64> > oa::PointList4::value(qint64 x, qint64 y, bool isPolygon) const
{
    int count = m_flags.count();
    if (isPolygon && count < 2) {
        qFatal("Requires more (>=2) points");
    }
    QPair<qint64, qint64> current(x,y);
    QVector<QPair<qint64, qint64>> result;
    result.append(current);
    for (int i = 0; i < count; ++i) {
        auto delta = m_points[i].value(m_flags.at(i));
        current.first += delta.first;
        current.second += delta.second;
        result.append(current);
    }
    return result;
}

// TODO Merge to PointList4, by pre-calculating
QVector<QPair<qint64, qint64> > oa::PointList5::value(qint64 x, qint64 y, bool isPolygon) const
{
    int count = m_flags.count();
    if (isPolygon && count < 2) {
        qFatal("Requires more (>=2) points");
    }
    QPair<qint64, qint64> current(x,y);
    QVector<QPair<qint64, qint64>> result;
    QPair<qint64, qint64> deltaAll(0,0);
    result.append(current);
    for (int i = 0; i < count; ++i) {
        auto delta = m_points[i].value(m_flags.at(i));
        deltaAll.first += delta.first;
        deltaAll.second += delta.second;
        current.first += deltaAll.first;
        current.second += deltaAll.second;
        result.append(current);
    }
    return result;
}

