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
#include <QVariant>

#include "records.h"

class QIODevice;
namespace oa {

class Layout;
class Parser
{
public:
    Parser(Layout& layout);
    bool open(const QString& filename);
protected:
    bool parse(QIODevice& dataStream);
    void beginStart(QIODevice& dataStream);
    void readTableOffsets(QIODevice& dataStream);
private:
    enum RecordType {
        PAD = 0,
        START = 1,
        END = 2,
        CELLNAME = 3,
        CELLNAMEX = 4,
        TEXTSTRING =5,
        TEXTSTRINGX = 6,
        PROPNAME = 7,
        PROPNAMEX = 8,
        PROPSTRING = 9,
        PROPSTRINGX = 10,
        LAYERNAME = 11,
        LAYERNAMEX = 12,
        CELL = 13,
        CELLX =14,
        XYABSOLUTE = 15,
        XYRELATIVE = 16,
        PLACEMENT = 17,
        PLACEMENTX = 18,
        TEXT = 19,
        RECTANGLE = 20,
        POLYGON = 21,
        PATH = 22,
        TRAPEZOID = 23,
        TRAPEZOIDX = 24,
        TRAPEZOIDY = 25,
        CTRAPEZOID =26,
        CIRCLE =27,
        PROPERTY = 28,
        PROPERTYX = 29,
        XNAME = 30,
        XNAMEX = 31,
        XELEMENT = 32,
        XGEOMETRY = 33,
        CBLOCK = 34
    };
    void nextRecord(QIODevice& dataStream);
    // Records
    void onPad(QIODevice& dataStream);
    void onStart(QIODevice& dataStream);
    void onEnd(QIODevice& dataStream);
    void onCellName(QIODevice& dataStream, int type); // 3 or 4
    void onTextString(QIODevice& dataStream, int type); // 5 or 6
    void onPropName(QIODevice& dataStream, int type);
    void onPropString(QIODevice& dataStream, int type);
    void onLayerName(QIODevice& dataStream, int type);
    void onCell(QIODevice& dataStream, int type);
    void onXYAbsolute();
    void onXYRelative();
    void onPlacement(QIODevice& dataStream, int type);
    void onText(QIODevice& dataStream);
    void onRectangle(QIODevice& dataStream);
    void onPolygon(QIODevice& dataStream);
    void onPath(QIODevice& dataStream);
    void onTrapezoid(QIODevice& dataStream, int type);
    void onCTrapezoid(QIODevice& dataStream);
    void onCircle(QIODevice& dataStream);
    void onProperty(QIODevice& dataStream, int type);
    void onXName(QIODevice& dataStream, int type);
    void onXElement(QIODevice& dataStream);
    void onXGeometry(QIODevice& dataStream);
    void onCBlock(QIODevice& dataStream);
    // Data
    quint64 onUnsigned(QIODevice& dataStream);
    qint64 onSigned(QIODevice& dataStream);
    double onReal(QIODevice& dataStream);
    Delta23 onDelta2(QIODevice& dataStream);
    Delta23 onDelta3(QIODevice& dataStream);
    DeltaG onDeltaG(QIODevice& dataStream);
    enum StringType {
        A,
        B,
        N
    };
    QString onString(QIODevice& dataStream, StringType type = A); // TODO Validation

    // Composite
    void onRepetition(QIODevice& dataStream);
    void onPointList(QIODevice& dataStream, bool isPolygon);

    IntervalType onInterval(QIODevice& dataStream);
    void undefineModalVariables();
private:
    Layout& m_layout;
    QSharedPointer<Cell> m_currentCell;
    QMap<qint64, QString> m_unresolvedCellName;
    QMap<qint64, QString> m_unresolvedPropName;
    QMap<qint64, QString> m_unresolvedTextString;
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
    Mode m_xNameMode;
    quint32 m_cellLocalNameReference;
    quint32 m_cellNameReference;
    quint32 m_textStringReference;
    quint32 m_propNameReference;
    quint32 m_propStringReference;
    quint32 m_xNameReference;
    QVector<quint64> m_tableOffsets;
    //////////////////////////////
    //      Modal variables     //
    //////////////////////////////
    bool m_isXYRelative; // xy-mode
    QSharedPointer<Repetition> m_repetition;
    QSharedPointer<PointList> m_polygonPointList;
    QSharedPointer<PointList> m_pointList;
    QString m_placementCell;
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
            uint32_t texttype: 1;
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
