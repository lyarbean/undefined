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

#include "dataconstructs.h"
#include <QDebug>
#include <QVector>
using namespace oa;

quint64 UnsignedInteger::decode(const QByteArray& data) {
    if (data.isEmpty()) {
        return 0;
    }
    qDebug() << sizeof(UnsignedInteger);
    qDebug() << sizeof(UnsignedInteger8);
    qDebug() << sizeof(UnsignedInteger16);
    qDebug() << sizeof(UnsignedInteger32);
    qDebug() << sizeof(UnsignedInteger64);
    
    qDebug() << UnsignedInteger8::type();
      
    static quint64 threshold = std::numeric_limits <quint64>::max () >> 7;
    quint64 r = 0;
    for (int i = 0; i < data.length(); ++i) {
        quint8 b = *reinterpret_cast<const quint8*>(data.constData() + i);
        if (r > threshold &&
                (quint64) (b & 0x7F) > (std::numeric_limits <quint64>::max () / r)) {
            qFatal("Unsigned integer value overflow");
        }
        r += ((quint64(b & 127)) << (7 * i));
        if (0 == (b & 128)) break;
    }
    return r;
}

Real::operator double () const  {
    if (m_data.isEmpty()) {
        return 0;
    }
    int type = m_data.at(m_data.length() - 1);
    switch (type) {
    case 0:
        return UnsignedInteger::decode(m_data);
    case 1:
        return - UnsignedInteger::decode(m_data);;
    case 2:
        return 1 / UnsignedInteger::decode(m_data);
    case 3:
        return - 1 / UnsignedInteger::decode(m_data);
    case 4:
        return 0;
    case 5:
        return 0;
    case 6: {
        Q_ASSERT(m_data.length() == 5);
        return  *(reinterpret_cast<const float*> (m_data.data()));
    }
    case 7: {
        Q_ASSERT(m_data.length() == 9);
        return *(reinterpret_cast<const double*> (m_data.data()));
    }
    default:
        return 0;
    }
}
