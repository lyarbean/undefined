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

#ifndef OA_DELTA_H
#define OA_DELTA_H
#include <QtGlobal>

namespace oa {
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
    // For delta g1, direction is one of east, north, west, south, northeast, northwest, southwest, and southeast.
    // and m_y is 2^64 - 1, m_x is the magnitude.
    // For delta g2, direction is one of northeast, northwest, southwest, and southeast.
    DeltaG(quint64 magnitude = 0);
    DeltaG(qint64 x = 0, qint64 y = 0);
    DeltaValue value;
};
}

#endif // OA_DELTA_H

