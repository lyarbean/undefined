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

#ifndef OA_REPETITION_H
#define OA_REPETITION_H

#include <QVector>
#include "delta.h"
namespace oa {
// In oasis, type 0 Repetition enforces last modal Repetition.
// So we do referrence it to the same Repetition
// Dimensions and spaces for X and Y, and grid
// quint32 m_dx, m_dy, m_sx, m_sy, m_g;
// n/m Dimensions and displacement
// quint32 m_dn, m_dm;
// DeltaG m_pn, m_pm;

struct Repetition
{
    enum {
        Type = 0
    };
    virtual int type() const {
        return Type;
    }
};

struct Repetition1 : public Repetition {
    enum {
        Type = 1
    };
    virtual int type() const {
        return Type;
    }
    // (m_dx+2) columns by (m_dy+2) rows matrix with spaces m_sx and m_sy for columns and rows
    // TODO
    // QVector<QPair<quint32, quint32>> offset;
    // for(int i=0; i < m_dx + 1; ++i)
    //     for(int j=0; j < m_dy + 1; ++j)
    //          offset.append({m_sx * i, m_y * j})
    // bounding {0, 0, m_sx * (m_dx + 1), m_sy * (m_dy + 1)}
    //
    quint32 m_dx, m_dy, m_sx, m_sy;
};

struct Repetition2 : public Repetition {
    enum {
        Type = 2
    };
    virtual int type() const {
        return Type;
    }
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
    // Similar to Repetition4, but with m_g multiplied by.
    quint32 m_g;
    quint32 m_dx;
    QVector<quint32> m_sxz;
};

struct Repetition6 : public Repetition {
    enum {
        Type = 6
    };
    virtual int type() const {
        return Type;
    }
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
    // Similar to Repetition5, but row and column exchanged.
    quint32 m_g;
    quint32 m_dy;
    QVector<quint32> m_syz;
};

struct Repetition8 : public Repetition {
    enum {
        Type = 8
    };
    virtual int type() const {
        return Type;
    }
    quint32 m_dn, m_dm, m_pn, m_pm;
};

struct Repetition9 : public Repetition {
    enum {
        Type = 9
    };
    virtual int type() const {
        return Type;
    }
    quint32 m_d;
    DeltaG m_p;
};

struct Repetition10 : public Repetition {
    enum {
        Type = 10
    };
    virtual int type() const {
        return Type;
    }
    quint32 m_d;
    QVector<DeltaG> m_pz;
};

struct Repetition11 : public Repetition {
    enum {
        Type = 11
    };
    virtual int type() const {
        return Type;
    }
    quint32 m_g;
    quint32 m_d; // g-delta
    QVector<DeltaG> m_pz;
};

// TODO store type at container, use it to detect Repetition type

}
#endif // OA_REPETITION_H
