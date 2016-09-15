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
    QVector<quint64> m_tableOffsets;
    // TODO put together, and move the following two to parse
    QVector<QString> m_localCellNames;
    QMap<quint32, QString> m_cellNames;
    QMap<quint32, QString> m_textStrings;
    QVector<QString> m_localLayerNames;
    QMap<quint32, QString> m_layerNames;
    QVector<QString> m_localPropNames;
    QMap<quint32, QString> m_propNames;
    QVector<QString> m_localPropStrings;
    QMap<quint32, QString> m_propStrings;
    QMap<quint32, XName> m_xNames;
    struct NamedCell {
        QString m_name;
        QSharedPointer<Cell> m_cell;
    };
    // {cellReference ==> {cellName ==> cellptr}}, if use this form, a cellptr could be null like klayout does
    QMap<qint64, NamedCell> m_cells;
    QMap<QString, QVector<QPair<quint32, quint32>>> m_layerMap; // cellname to data-layer

    
};
}

#endif // OA_LAYOUT_H
