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

#ifndef OA_LAYOUT_H
#define OA_LAYOUT_H
#include <QVector>
#include <QMap>
#include "cell.h"
namespace oa {

class Layout
{
public:
    Layout();
/*private:*/
    qreal m_unit;
    quint8 m_offsetFlag;
    QVector<quint32> m_tableOffsets;
    QMap<quint32, QString> m_cellNames;
    QMap<quint32, QString> m_textStrings;
    QMap<quint32, QString> m_layerNames;
    QMap<quint32, QString> m_PropNames;
    QMap<quint32, QString> m_PropStrings;
    QMap<quint32, QString> m_XNames;
    QVector<QSharedPointer<Cell>> m_cells;

};
}

#endif // OA_LAYOUT_H
