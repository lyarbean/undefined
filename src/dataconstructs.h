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

#ifndef OA_REAL_H
#define OA_REAL_H
// NOTE UNUSED
#include <QtCore/QByteArray>
namespace oa {

struct UnsignedInteger {
    static quint64 decode (const QByteArray& data);
    enum {
        Type = 0
    };
    static int type()  {
        return Type;
    }
};

struct UnsignedInteger8 : public UnsignedInteger {
    enum {
        Type = 8
    };
    quint8 m_data;
};

struct UnsignedInteger16 {
    enum {
        Type = 16
    };
    quint16 m_data;
};


struct UnsignedInteger32 {
    enum {
        Type = 32
    };
    quint32 m_data;
};

struct UnsignedInteger64 {
    enum {
        Type = 64
    };
    quint64 m_data;
};


struct Real
{
    operator double () const;
    // Store type at last byte
    QByteArray m_data;
};
}

#endif // OA_REAL_H


