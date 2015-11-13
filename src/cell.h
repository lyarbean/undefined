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

#ifndef OA_CELL_H
#define OA_CELL_H
#include <QString>
#include <QVector>
#include "records.h"
#include "polygon.h"
namespace oa {

struct Cell
{
    Cell() = default;
    QString m_name;// TODO use id
    qint64 m_index;
//     QVector <> m_properties;
//     bool m_XYRelative;

    QVector<Rectangle> m_rectangles;
    QVector<Polygon> m_polygons;
    QVector<Path> m_paths;
    QVector<Trapezoid> m_trapezoids;
    QVector<CTrapezoid> m_ctrapezoids;
    QVector<Circle> m_circles;
    QVector<XGeometry> m_xgeometries;
    QVector<Placement> m_placements;
    QVector<Text> m_texts;
    QVector<XELement> m_xelements;

};
}

#endif // OA_CELL_H
