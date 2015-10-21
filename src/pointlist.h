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

#ifndef POINTLIST_H
#define POINTLIST_H
#include "delta.h"

namespace oa {
// TODO [Optimization] The direction for each delta is range of 1 to 12, so 4 bits is enough.

struct PointList {
    enum {
        Type = -1
    };
    virtual int type() const {
        return Type;
    }
};

struct PointList0 : public PointList {
    enum {
        Type = 0
    };
    virtual int type() const {
        return Type;
    }
    // 0 Horizontal first
    // 1 Vertical first
    virtual QVector<QPair<qint64, qint64>> value(qint64 x, qint64 y, bool isPolygon = true) const;
    QVector<quint8> m_flags; // flags must be NorthEast or SouthWest
    QVector<Delta1> m_points;
};

struct PointList1 : public PointList0 {
    enum {
        Type = 1
    };
    virtual int type() const {
        return Type;
    }
};


struct PointList23 : public PointList {
    enum {
        Type = 2
    };
    virtual int type() const {
        return Type;
    }
    // 2 manhattan
    // 3 octangular
    QVector<QPair<qint64, qint64>> value(qint64 x, qint64 y, bool isPolygon = true) const;
    QVector<quint8> m_flags;
    QVector<Delta23> m_points;
};

struct PointList4 : public PointList {
    enum {
        Type = 4
    };
    virtual int type() const {
        return Type;
    }
    // 4 all angle delta
    // 5 all angle double
    QVector<QPair<qint64, qint64>> value(qint64 x, qint64 y, bool isPolygon = true) const;
    QVector<quint8> m_flags;
    QVector<DeltaG> m_points;
};

struct PointList5 : public PointList {
    enum {
        Type = 8
    };
    virtual int type() const {
        return Type;
    }
    // 4 all angle delta
    // 5 all angle double
    QVector<QPair<qint64, qint64>> value(qint64 x, qint64 y, bool isPolygon = true) const;
    QVector<quint8> m_flags;
    QVector<DeltaG> m_points;
};

}

#endif // POINTLIST_H
