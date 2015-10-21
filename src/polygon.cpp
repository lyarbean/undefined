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

#include "polygon.h"
#include "delta.h"




QVector<QPair<qint64, qint64>> oa::Polygon::value()
{
    switch (m_pointList->type()) {
    case 0: {
        auto pointList = static_cast<PointList1*>(m_pointList.data());
        return pointList->value(m_x, m_y);
    }
    case 1: {
        auto pointList = static_cast<PointList1*>(m_pointList.data());
        return pointList->value(m_x, m_y);
    }

    case 2:
    case 3: {
        auto pointList = static_cast<PointList23*>(m_pointList.data());
        return pointList->value(m_x, m_y);
    }
    case 4:
    case 5: {
        auto pointList = static_cast<PointList23*>(m_pointList.data());
        return pointList->value(m_x, m_y);
    }
    default:
        return {};
    }
}



QVector<QPair<qint64, qint64> > oa::Path::value()
{
    switch (m_pointList->type()) {
    case 0: {
        auto pointList = static_cast<PointList1*>(m_pointList.data());
        return pointList->value(m_x, m_y, false);
    }

    case 1: {
        auto pointList = static_cast<PointList1*>(m_pointList.data());
        return pointList->value(m_x, m_y, false);
    }

    case 2:
    case 3: {
        auto pointList = static_cast<PointList23*>(m_pointList.data());
        return pointList->value(m_x, m_y, false);
    }

    case 4:
    case 5: {
        auto pointList = static_cast<PointList23*>(m_pointList.data());
        return pointList->value(m_x, m_y, false);
    }
    default:
        return {};
    }
}

