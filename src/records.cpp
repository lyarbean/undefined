#include "records.h"

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

