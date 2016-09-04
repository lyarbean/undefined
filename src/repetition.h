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

// 7.6.2 TYPE 0 indicates that the previous repetition description, stored in modal variable repetition, is to be re-used.
// (See section 10 on page 12.) No additional values are stored with this type.
// 7.6.3 TYPE 1 is an N-column (N > 1) by M-row (M > 1) matrix with uniform horizontal and vertical spacing
// between the elements. x-dimension is N - 2 and y-dimension is M - 2. The (x-offset, y-offset) (cumulative spacing in
// the (horizontal,vertical) direction) of element (i,j) of the repetition (i = 0, ..., N-1 and j = 0, ..., M-1) is
// (i * x-space, j * y-space).
// 7.6.4 TYPE 2 is an N-column (N > 1) by 1-row vector with uniform horizontal spacing between the elements. x-
// dimension is N - 2. The (x-offset, y-offset) (cumulative spacing in the (horizontal,vertical) direction) of element i of
// the repetition (i = 0, ..., N-1) is (i * x-space, 0).
// 7.6.5 TYPE 3 is a 1-column by M-row (M > 1) vector with uniform vertical spacing between the elements. y-dimen-
// sion is M - 2. The (x-offset, y-offset) (cumulative spacing in the (horizontal,vertical) direction) of element j of the
// repetition (j = 0, ..., M-1) is (0, j * y-space).
// 7.6.6 TYPE 4 is an N-column (N > 1) by 1-row vector with (potentially) non-uniform horizontal spacing between the
// elements. x-dimension is N - 2. The (x-offset, y-offset) (cumulative spacing in the (horizontal,vertical) direction) of
// element i of the repetition (i = 0, ..., N-1) is (x-space0 + ... + x-spacei, 0), with x-space0 = 0.
// 7.6.7 TYPE 5 is identical to TYPE 4, except that all offset values must be multiplied by grid during expansion of the
// repetition.
// 7.6.8 TYPE 6 is a 1-column by M-row (M > 1) vector with (potentially) non-uniform vertical spacing between the
// elements. y-dimension is M - 2. The (x-offset, y-offset) (cumulative spacing in the (horizontal,vertical) direction) of
// element j of the repetition (j = 0, ..., M-1) is (0, y-space0 + ... + y-spacej), with y-space0 = 0.
// 7.6.9 TYPE 7 is identical to TYPE 6, except that all offset values must be multiplied by grid during expansion of the
// repetition.
// 7.6.10 TYPE 8 is an N (N > 1) by M (M > 1) repetition with uniform and (potentially) diagonal displacements
// between the elements. n-dimension is N - 2 and m-dimension is M - 2. Defining n-displacement in terms of its
// components nx-space and ny-space (and similarly for m-displacement), the (x-offset, y-offset) (cumulative spacing
// in the (horizontal,vertical) direction) of element (i,j) of the repetition (i = 0, ..., N-1 and j = 0, ..., M-1) is
// (i * nx-space + j * mx-space, i * ny-space + j * my-space).
// 7.6.11 TYPE 9 is a P-element (P > 1) repetition with uniform and (potentially) diagonal displacements between the
// elements. dimension is P - 2. Defining displacement in terms of its components x-space and y-space, the (x-offset,
// y-offset) (cumulative spacing in the (horizontal,vertical) direction) of element k of the repetition (k = 0, ..., P-1) is
// (k * x-space, k * y-space).
// 7.6.12 TYPE 10 is a P-element (P > 1) repetition with (potentially) non-uniform and arbitrary two-dimensional dis-
// placements between the elements. dimension is P - 2. Defining displacementk in terms of its components x-spacek
// and y-spacek, the (x-offset, y-offset) (cumulative spacing in the (horizontal,vertical) direction) of element k of the
// repetition (k = 0, ..., P-1) is (x-space0 + ... + x-spacek, y-space0 + ... + y-spacek) with x-space0 = y-space0 = 0).
// 7.6.13 TYPE 11 is identical to TYPE 10, except that all offset values must be multiplied by grid during expansion of
// the repetition.

namespace oa {
// In oasis, type 0 Repetition enforces last modal Repetition.
// So we do referrence it to the same Repetition
// Dimensions and spaces for X and Y, and grid
// quint32 m_dx, m_dy, m_sx, m_sy, m_g;
// n/m Dimensions and displacement
// quint32 m_dn, m_dm;
// DeltaValue m_pn, m_pm;

    
struct Repetition
{
    enum {
        Type = 0
    };
    virtual int type() const {
        return Type;
    }
    virtual QVector<DeltaValue> values() {} // TODO purize
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
    Repetition2(quint32 dx, quint32 sx) : m_dx(dx), m_sx(sx) {}
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
    quint32 m_d;
    quint32 m_g;
    QVector<DeltaValue> m_pz;
};

}
#endif // OA_REPETITION_H
