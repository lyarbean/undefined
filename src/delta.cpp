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

#include "delta.h"
#include <QDebug>


oa::Delta1::Delta1(quint32 magnitude) :  m_magnitude(magnitude)
{
}


oa::DeltaValue oa::Delta1::value(int direction) const
{
    qint64 m = static_cast<qint64>(m_magnitude);
    switch (direction) {
    case East:
        return {m, 0};
    case West:
        return {- m, 0};
    case North:
        return {0, m};
    case South:
        return {0, - m};
    default:
        // qFatal
        return {0, 0};
    }
}
oa::Delta23::Delta23(quint32 magnitude)
    :  m_magnitude(magnitude)
{
}

oa::DeltaValue oa::Delta23::value(int direction) const
{
    qint64 m = static_cast<qint64>(m_magnitude);
    switch (direction) {
    case East:
        return {m, 0};
    case West:
        return {- m, 0};
    case North:
        return {0, m};
    case South:
        return {0, - m};
    case NorthEast:
        return {m, m};
    case NorthWest:
        return {-m, m};
    case SouthEast:
        return {m, - m};
    case SouthWest:
        return {- m, - m};
    default:
        // qFatal
        return {0, 0};
    }
}

oa::DeltaG::DeltaG(quint32 xMagnitude, quint32 yMagnitude) : m_x (xMagnitude), m_y (yMagnitude)
{
}


oa::DeltaValue oa::DeltaG::value(int direction) const
{
    qint64 x = static_cast<qint64>(m_x);
    qint64 y = static_cast<qint64>(m_y);
    if (m_y == -1) {
        switch (direction) {
        case East:
            return {x, 0};
        case West:
            return {- x, 0};
        case North:
            return {0, x};
        case South:
            return {0, - x};
        case NorthEast:
            return {x, x};
        case NorthWest:
            return {- x, x};
        case SouthEast:
            return {x, - x};
        case SouthWest:
            return {- x, - x};
        default:
            return {0, 0};
        }
    } else {
        switch (direction) {
        case NorthEast:
            return {x, y};
        case NorthWest:
            return {- x, y};
        case SouthEast:
            return {x, - y};
        case SouthWest:
            return {- x, - y};
        default:
            return {0, 0};
        }
    }
}

oa::DeltaG & oa::DeltaG::operator+=(const oa::DeltaG that)
{
    
}

