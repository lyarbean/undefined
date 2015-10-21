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
#include <QVector>
#include <QPair>
namespace oa {

// FIXME Check if 32 bit is enough in practice

using DeltaValue = QPair<qint64, qint64>;

enum DeltaDirection {
    East = 1,
    North = 2,
    West = 4,
    South = 8,
    NorthEast = 3,
    NorthWest = 6,
    SouthEast = 9,
    SouthWest = 12,
    Horizontal = 5, // East and West
    Vertical = 10, // North and South
    // Diagonal = 15, // Horizontal and Vertical
};


struct Delta1 {
    // Direction is either east/north or west/south, i.e., positive or negative, with value , 1, 2, 4, 8 exclusively;
    Delta1(quint32 magnitude = 0);
    DeltaValue value(int direction) const;
    quint32 m_magnitude;
};

struct Delta23 {
    // For delta 2, direction is one of east, north, west and south.
    // For delta 3, direction is one of east, north, west, south, northeast, northwest, southwest, and southeast.
    Delta23(quint32 magnitude = 0);
    DeltaValue value(int direction) const;
    quint32 m_magnitude;
};


struct DeltaG {
    // For delta g1, direction is one of east, north, west, south, northeast, northwest, southwest, and southeast.
    // and m_y is 2^64 - 1, m_x is the magnitude.
    // For delta g2, direction is one of northeast, northwest, southwest, and southeast.
    DeltaG(quint32 xMagnitude = 0, quint32 yMagnitude = -1);
    DeltaValue value(int direction) const;
    DeltaG& operator +=(const DeltaG that);
    quint32 m_x;
    quint32 m_y;
};

}

#endif // OA_DELTA_H

