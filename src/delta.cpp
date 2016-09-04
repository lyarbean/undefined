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

// 7.5.2 A 1-delta is stored as a signed-integer and represents a horizontal or vertical displacement. Bit 0 encodes direc-
// tion: 0 for east or north, 1 for west or south. The remaining bits are the magnitude. Horizontal or vertical alignment is
// implied by context.
oa::Delta1::Delta1(qint64 magnitude) :  m_magnitude(magnitude)
{
}

// FIXME
oa::DeltaValue oa::Delta1::value(int direction) const
{
    switch (direction) {
    case 0:
        return {m_magnitude, 0};
        return {0, m_magnitude};
    case 1:
        return {0, - m_magnitude};
    default:
        // qFatal
        return {0, 0};
    }
}

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
            value = {-m, 0};
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
            value = {-m, 0};
            break;
        case 3:
            value = {0, -m};
            break;
        case 4:
            value = {m, m};
            break;
        case 5:
            value = {-m, m};
            break;
        case 6:
            value = {-m, -m};
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
        value = {-m, 0};
        break;
    case 3:
        value = {0, -m};
        break;
    case 4:
        value = {m, m};
        break;
    case 5:
        value = {-m, m};
        break;
    case 6:
        value = {-m, -m};
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

// oa::DeltaG & oa::DeltaG::operator+=(const oa::DeltaG that)
// {
//     
// }

