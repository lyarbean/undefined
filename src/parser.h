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

#ifndef OA_PARSER_H
#define OA_PARSER_H
#include <QString>
#include "cell.h"

#include <QDataStream>
#include <QVariant>
namespace oa {

class Layout;
class Parser
{
    Parser();
    bool open(const QString& filename);
private:
    bool nextRecord();
    // Records
    bool onPad();
    bool onStart();
    bool onEnd();
    bool onCellName(int type); // 3 or 4
    bool onTextString(int type); // 5 or 6
    bool onPropName(int type);
    bool onPropString(int type);
    bool onLayerName(int type);
    bool onCell(int type);
    bool onXYAbsolute();
    bool onXYRelative();
    bool onPlacement(int type);
    bool onText();
    bool onRectangle();
    bool onPolygon();
    bool onPath();
    bool onTrapezoid(int type);
    bool onCTrapezoid();
    bool onCircle();
    bool onProperty(int type);
    bool onXName(int type);
    bool onXElement();
    bool onXGeometry();
    bool onCBlock();
    // Data
    quint64 onUnsigned();
    qint64 onSigned();
    double onReal();
    Delta1 onDelta1();
    Delta23 onDelta2();
    Delta23 onDelta3();
    DeltaG onDeltaG();
    enum StringType {
        A,
        B,
        N
    };
    QString onString(StringType type = A); // TODO Validation

    // Composite
    QSharedPointer<Repetition> onRepetition();
    QSharedPointer<PointList> onPointList(bool isPolygon);
    bool onTableOffset();
    using IntervalType = QPair<quint32, quint32>;
    IntervalType onInterval();
    void undefineModalVariables();
private:
    QDataStream m_dataStream;
    Layout* m_layout;
    //     QVector<QSharedPointer<Repetition>> m_repetitions;
    //     QVector<QSharedPointer<PointList>> m_polygonPointlists;
    bool m_offsetFlag;
        enum Mode {
        Default = 0,
        Explicit,
        Implicit
    };
    Mode m_cellNameMode;
    Mode m_textStringMode;
    Mode m_propNameMode;
    Mode m_propStringMode;
    quint32 m_cellLocalNameReference;
    quint32 m_cellNameReference;
    quint32 m_textStringReference;
    quint32 m_propNameReference;
    quint32 m_propStringReference;
    QSharedPointer<Cell> m_currentCell;
    //////////////////////////////
    //      Modal variables     //
    //////////////////////////////
    bool m_isXYRelative; // xy-mode
    QSharedPointer<Repetition> m_repetition;
    QSharedPointer<PointList> m_polygonPointList;
    QSharedPointer<PointList> m_pointList;
    QSharedPointer<Placement> m_placementCell;
    quint32 m_layer, m_datatype;
    quint32 m_textLayer, m_textType;
    qint64 m_placementX;
    qint64 m_placementY;
    QString m_textString;
    qint64 m_textX, m_textY;
    qint64 m_geometryX, m_geometryY;
    quint32 m_geometryW, m_geometryH;
    quint32 m_halfWidth;
    qint32 m_startExtension, m_endExtension;
    quint8 m_ctrapezoidType;
    quint32 m_circleRadius;
    QString m_lastPropertyName;
    QVector<QVariant> m_lastValuesList; // qreal or QString
    union {
        struct {
            uint32_t repetition: 1;
            uint32_t placementCell: 1;
            uint32_t layer: 1;
            uint32_t datatype: 1;
            uint32_t textlayer: 1;
            uint32_t textdatatype: 1;
            uint32_t textString: 1;
            uint32_t xyMode: 1;
            uint32_t geometryW: 1;
            uint32_t geometryH: 1;
            uint32_t polygonPointList: 1;
            uint32_t pathHalfwidth: 1;
            uint32_t pathPointList: 1;
            uint32_t pathStartExtention: 1;
            uint32_t pathEndExtention: 1;
            uint32_t ctrapezoidType: 1;
            uint32_t circleRadius: 1;
            uint32_t lastPropertyName: 1;
            uint32_t lastValueList: 1;
        } m_d;
        uint32_t m_dummy;
    } m_modalVariableSetStatus;
    
    
    
};
}

#endif // OA_PARSER_H
