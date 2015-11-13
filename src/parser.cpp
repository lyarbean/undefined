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

#include "parser.h"

#include <QDataStream>
#include <QFile>

#include "layout.h"

oa::Parser::Parser()
    : m_layout(0)
{
}

bool oa::Parser::open(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    m_dataStream.setDevice(&file);

    static char magicBytes[13];
    if (m_dataStream.readRawData(magicBytes, 13) != 13) {
        qFatal("Not an oasis");
        return false;
    }
    if (strcmp(magicBytes, "%SEMI-OASIS\r\n")) {
        qFatal("Not an oasis");
        return false;
    }
    if (m_layout) {
        // emit m_layout->deleteLater();
    }
    m_layout = new Layout;
    while (!m_dataStream.atEnd() && nextRecord())
        ;
    m_dataStream.setDevice(0);
    file.close();
}

bool oa::Parser::nextRecord()
{
    quint8 type = 0;
    m_dataStream >> type;
    switch (type) {
    case 0:
        return onPad();
    case 1:
        return onStart();
    case 2:
        return onEnd();
    case 3:
    case 4:
        return onCellName(type);
    case 5:
    case 6:
        return onTextString(type);
    case 7:
    case 8:
        return onPropName(type);
    case 9:
    case 10:
        return onPropString(type);
    case 11:
    case 12:
        return onLayerName(type);
    case 13:
    case 14:
        return onCell(type);
    case 15:
        return onXYAbsolute();
    case 16:
        return onXYRelative();
    case 17:
    case 18:
        return onPlacement(type);
    case 19:
        return onText();
    case 20:
        return onRectangle();
    case 21:
        return onPolygon();
    case 22:
        return onPath();
    case 23:
    case 24:
    case 25:
        return onTrapezoid(type);
    case 26:
        return onCTrapezoid();
    case 27:
        return onCircle();
    case 28:
    case 29:
        return onProperty(type);
    case 30:
    case 31:
        return onXName(type);
    case 32:
        return onXElement();
    case 33:
        return onXGeometry();
    case 34:
        return onCBlock();
    default:
        return false;
    }
}
bool oa::Parser::onStart()
{
    static char version[3];
    if (m_dataStream.readRawData(version, 3) != 3) {
        qFatal("Bad version");
        return false;
    }
    if (strcmp(version, "1.0")) {
        qFatal("Bad version");
        return false;
    }
    m_layout->m_unit = onReal(); // validation
    m_layout->m_offsetFlag = onUnsigned();
    if (!m_layout->m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_layout->m_tableOffsets << onUnsigned();
        }
    }
    return true;
}

bool oa::Parser::onPad() { return true; }

bool oa::Parser::onEnd()
{
    if (m_layout->m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_layout->m_tableOffsets << onUnsigned();
        }
    }
    // padding string
    int size = 255 - (m_layout->m_offsetFlag ? 12 : 0);
    char endBytes[256];
    return m_dataStream.readRawData(endBytes, size) == size;
}

bool oa::Parser::onCellName(int type)
{
    QString name = onString(N);
    if (m_cellNameMode == Default) {
        if (type == 3) {
            m_cellNameMode = Implicit;
        }
        else {
            m_cellNameMode = Explicit;
        }
    }
    if ((m_cellNameMode == Implicit && type == 4) || (m_cellNameMode == Explicit && type == 3)) {
        qFatal("Both Implicit and Explicit CellName!");
    }
    quint32 reference;
    if (m_cellNameMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_cellNameReference++;
    }
    // TODO check for reference and name
    m_layout->m_cellNames.insert(reference, name);
    return true;
}

bool oa::Parser::onTextString(int type)
{
    QString name = onString(N);
    if (m_textStringMode == Default) {
        if (type == 5) {
            m_textStringMode = Implicit;
        }
        else {
            m_textStringMode = Explicit;
        }
    }
    if ((m_textStringMode == Implicit && type == 6) || (m_textStringMode == Explicit && type == 5)) {
        qFatal("Both Implicit and Explicit textString!");
    }
    quint32 reference;
    if (m_textStringMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_textStringReference++;
    }
    // TODO check for reference and name
    m_layout->m_textStrings.insert(reference, name);
    return true;
}

bool oa::Parser::onPropName(int type)
{
    QString name = onString(N);
    if (m_propNameMode == Default) {
        if (type == 7) {
            m_propNameMode = Implicit;
        }
        else {
            m_propNameMode = Explicit;
        }
    }
    if ((m_propNameMode == Implicit && type == 8) || (m_propNameMode == Explicit && type == 7)) {
        qFatal("Both Implicit and Explicit PropName!");
    }
    quint32 reference;
    if (m_propNameMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_propNameReference++;
    }
    // TODO check for reference and name
    m_layout->m_propNames.insert(reference, name);
    return true;
}

bool oa::Parser::onPropString(int type)
{
    QString name = onString(N);
    if (m_propStringMode == Default) {
        if (type == 9) {
            m_propStringMode = Implicit;
        }
        else {
            m_propStringMode = Explicit;
        }
    }
    if ((m_propStringMode == Implicit && type == 10) || (m_propStringMode == Explicit && type == 9)) {
        qFatal("Both Implicit and Explicit PropString!");
    }
    quint32 reference;
    if (m_propStringMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_propStringReference++;
    }
    // TODO check for reference and name
    m_layout->m_propStrings.insert(reference, name);
    return true;
}

bool oa::Parser::onLayerName(int type)
{
    Q_UNUSED(type);
    QString name = onString(N);
    IntervalType boundA, boundB;
    boundA = onInterval();
    boundB = onInterval();
    // TODO How to store?
    return true;
}

bool oa::Parser::onCell(int type)
{
    QSharedPointer<Cell> cell(new Cell);
    m_layout->m_cells.push_back(cell);
    m_currentCell = cell;
    if (type == 13) {
        cell->m_index = onUnsigned();
    }
    else {
        cell->m_index = -m_layout->m_localCellNames.count();
        m_layout->m_localCellNames.append(onString(N));
    }
    return true;
}

bool oa::Parser::onXYAbsolute()
{
    m_isXYRelative = false;
    return true;
}

bool oa::Parser::onXYRelative()
{
    m_isXYRelative = true;
    return true;
}

bool oa::Parser::onPlacement(int type)
{
    Placement placement;
    // TODO Wrap these two lines
    quint8 info = 0;
    m_dataStream >> info;
    // CNXYRAAF or CNXYRMAF
    if (info >> 7) { // C
        if ((info >> 6) & 1) { // N
            m_placement.m_index = onUnsigned();
        }
        else {
            m_placement.m_index = -m_layout->m_localCellNames.count();
            m_layout->m_localCellNames.append(onString(N));
        }
    }
    placement.m_index = m_placement.m_index;
    if (type == 18) {
        if ((info >> 2) & 1) { // M
            placement.m_manification = onReal();
        }
        else {
            placement.m_manification = 1.0;
        }
        if ((info >> 1) & 1) { // A
            placement.m_angle = onReal();
        }
        else {
            placement.m_angle = 0.0;
        }
    }
    if ((info >> 5) & 1) { // X
        placement.m_x = onSigned();
        if (m_isXYRelative) {
            placement.m_x += m_placement.m_x;
        }
        m_placement.m_x = placement.m_x;
    }
    else {
        placement.m_x = m_placement.m_x;
    }
    if ((info >> 4) & 1) { // Y
        placement.m_y = onSigned();
        if (m_isXYRelative) {
            placement.m_y += m_placement.m_y;
        }
        m_placement.m_y = placement.m_y;
    }
    else {
        placement.m_y = m_placement.m_y;
    }
    if ((info >> 3) & 1) { // R
        m_repetition = onRepetition();
        placement.m_repetition = m_repetition;
    }
    placement.m_flip = (info & 0b1); // F
    m_currentCell->m_placements.append(placement);
    return true;
}

bool oa::Parser::onText()
{
    Text text;
    quint8 info = 0;
    m_dataStream >> info;
    // 0CNXYRTL
    if (info >> 6) { // C
        if ((info >> 5) & 1) { // N
            text.m_index = onUnsigned();
        }
        else {
            text.m_index = m_layout->m_localTextStrings.count();
            m_layout->m_localTextStrings.append(onString(N));
        }
    }
    else {
        // use modal TEXTSTRING
        text.m_index = m_text.m_index;
    }
    if (info & 1) { // L
        text.m_layer = onUnsigned();
    }
    else {
        text.m_layer = m_layer;
    }
    if ((info >> 1) & 1) { // T
        text.m_datatype = onUnsigned();
    }
    else {
        text.m_datatype = m_datatype;
    }
    if ((info >> 4) & 1) { // X
        text.m_x = onSigned();
        if (m_isXYRelative) {
            text.m_x += m_text.m_x;
        }
    }
    else {
        text.m_x = m_text.m_x;
    }
    if ((info >> 3) & 1) { // Y
        text.m_y = onSigned();
        if (m_isXYRelative) {
            text.m_y += m_text.m_y;
        }
    }
    else {
        text.m_y = m_text.m_y;
    }
    if ((info >> 2) & 1) { // R
        text.m_repetition = onRepetition();
    }
    m_currentCell->m_texts.append(text);
    return true;
}

bool oa::Parser::onRectangle()
{
    Rectangle rectangle;
    quint8 info = 0;
    m_dataStream >> info;
    // SWHXTRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    rectangle.m_layer = m_layer;

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    rectangle.m_datatype = m_datatype;

    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
    }
    rectangle.m_width = m_geometryW;

    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned();
    }
    rectangle.m_height = m_geometryH;

    if ((info >> 4) & 1) { // X
        rectangle.m_x = onUnsigned();
        if (m_isXYRelative) {
            rectangle.m_x += m_geometryX;
        }
    }
    else {
        rectangle.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        rectangle.m_y = onUnsigned();
        if (m_isXYRelative) {
            rectangle.m_y += m_geometryY;
        }
    }
    else {
        rectangle.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        rectangle.m_repetition = m_repetition;
    }
    m_currentCell->m_rectangles.append(rectangle);
    return true;
}

bool oa::Parser::onPolygon()
{
    Polygon polygon;
    quint8 info = 0;
    m_dataStream >> info;
    // 00PXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    polygon.m_layer = m_layer;

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    polygon.m_datatype = m_datatype;

    if ((info >> 5) & 1) { // P
        m_polygonPointList = onPointList();
    }
    polygon.m_pointList = m_polygonPointList;

    if ((info >> 4) & 1) { // X
        polygon.m_x = onUnsigned();
        if (m_isXYRelative) {
            polygon.m_x += m_geometryX;
        }
    }
    else {
        polygon.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        polygon.m_y = onUnsigned();
        if (m_isXYRelative) {
            polygon.m_y += m_geometryY;
        }
    }
    else {
        polygon.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        polygon.m_repetition = m_repetition;
    }
    m_currentCell->m_polygons.append(polygon);
    return true;
}

bool oa::Parser::onPath()
{
    Path path;
    quint8 info = 0;
    m_dataStream >> info;
    // EWPXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    path.m_layer = m_layer;

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    path.m_datatype = m_datatype;

    if ((info >> 5) & 1) { // P
        m_pointList = onPointList();
    }
    path.m_pointList = m_pointList;
    if ((info >> 6) & 1) { // W
        m_path.m_halfWidth = onUnsigned();
    }
    path.m_halfWidth = m_path.m_halfWidth;
    if ((info >> 4) & 1) { // X
        path.m_x = onSigned();
        if (m_isXYRelative) {
            path.m_x += m_geometryX;
        }
    }
    else {
        path.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        path.m_y = onSigned();
        if (m_isXYRelative) {
            path.m_y += m_geometryY;
        }
    }
    else {
        path.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        path.m_repetition = m_repetition;
    }
    m_currentCell->m_paths.append(path);
    return true;
}

bool oa::Parser::onTrapezoid(int type)
{
    Trapezoid trapezoid;
    quint8 info = 0;
    m_dataStream >> info;
    // 0WHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    trapezoid.m_layer = m_layer;
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    trapezoid.m_datatype = m_datatype;
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
    }
    trapezoid.m_width = m_geometryW;
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned();
    }
    trapezoid.m_height = m_geometryH;
    if (type == 23) {
        trapezoid.m_a = onDelta1();
        trapezoid.m_b = onDelta1();
    }
    else if (type == 24) {
        trapezoid.m_a = onDelta1();
    }
    else if (type == 25) {
        trapezoid.m_b = onDelta1();
    }
    if ((info >> 4) & 1) { // X
        trapezoid.m_x = onSigned();
        if (m_isXYRelative) {
            trapezoid.m_x += m_geometryX;
        }
    }
    else {
        trapezoid.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        trapezoid.m_y = onSigned();
        if (m_isXYRelative) {
            trapezoid.m_y += m_geometryY;
        }
    }
    else {
        trapezoid.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        trapezoid.m_repetition = m_repetition;
    }
    m_currentCell->m_trapezoids.append(trapezoid);
}

bool oa::Parser::onCTrapezoid()
{
    CTrapezoid ctrapezoid;
    quint8 info = 0;
    m_dataStream >> info;
    // TWHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    ctrapezoid.m_layer = m_layer;
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    ctrapezoid.m_datatype = m_datatype;
    if ((info >> 7) & 1) { // T
        ctrapezoid.m_type = onUnsigned();
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
    }
    ctrapezoid.m_width = m_geometryW;
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned();
    }
    ctrapezoid.m_height = m_geometryH;
    if ((info >> 4) & 1) { // X
        ctrapezoid.m_x = onSigned();
        if (m_isXYRelative) {
            ctrapezoid.m_x += m_geometryX;
        }
    }
    else {
        ctrapezoid.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        ctrapezoid.m_y = onSigned();
        if (m_isXYRelative) {
            ctrapezoid.m_y += m_geometryY;
        }
    }
    else {
        ctrapezoid.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        ctrapezoid.m_repetition = m_repetition;
    }
    m_currentCell->m_ctrapezoids.append(ctrapezoid);
}

bool oa::Parser::onCircle()
{
    Circle circle;
    quint8 info = 0;
    m_dataStream >> info;
    // 00rXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    circle.m_layer = m_layer;
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    circle.m_datatype = m_datatype;
    if ((info >> 5) & 1) { // r
        m_circleRadius = onUnsigned();
    }
    circle.m_radius = m_circleRadius;
    if ((info >> 4) & 1) { // X
        circle.m_x = onSigned();
        if (m_isXYRelative) {
            circle.m_x += m_geometryX;
        }
    }
    else {
        circle.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        circle.m_y = onSigned();
        if (m_isXYRelative) {
            circle.m_y += m_geometryY;
        }
    }
    else {
        circle.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        circle.m_repetition = m_repetition;
    }
    m_currentCell->m_circles.append(circle);
}

bool oa::Parser::onProperty(int type)
{
    if (type == 29) {
        return false; // m_lastPropertyName
    }
    quint8 info = 0;
    m_dataStream >> info;
    // UUUUVCNS
    if ((info >> 2) & 1) { // C
        if ((info >> 1) & 1) { // N
            m_lastPropertyName = m_layout->m_propNames[onUnsigned()];
        }
        else {
            m_lastPropertyName = onString(N);
        }
    }
    quint32 valueCount = 0;
    if ((info >> 3) & 1) { // V
        quint8 uuuu = info >> 4;
        if (uuuu == 15) {
            valueCount = onUnsigned();
        }
        if (uuuu < 15) {
            valueCount = uuuu;
        }
        for (auto i = 0; i < valueCount; ++i) {
            quint8 kind = 0;
            m_dataStream >> kind;
            switch (kind) {
            case 0 ... 7:
                onReal();
            case 8:
                onUnsigned();
            case 9:
                onSigned();
            case 10:
                onString(A);
            case 11:
                onString(B);
            case 12:
                onString(N);
            case 13 ... 15:
                onUnsigned();
            default:
                break;
            }
        }
    }
    else {
        // Assert info >> 4  == 0;
        // no update
    }
}

bool oa::Parser::onXName(int type)
{
    XName xname;
    xname.m_attribute = onUnsigned();
    xname.m_string = onString(N); // ABN
    if (type == 30) {
        m_layout->m_localXNames.push_back(xname);
    }
    else {
        int referenceNumber = onUnsigned();
        auto pos = m_layout->m_xNames.find(referenceNumber);
        // TODO
        //        if (pos != m_layout->m_xNames.end() && *pos != xname) {
        //            // Fatal
        //        }
        m_layout->m_xNames[onUnsigned()] = xname;
    }
}

bool oa::Parser::onXElement()
{
    // Assert state Element
    XELement xelement;
    xelement.m_attribute = onUnsigned();
    xelement.m_string = onString(B);
    m_currentCell->m_xelements.push_back(xelement);
}

bool oa::Parser::onXGeometry()
{
    XGeometry xgeometry;
    quint8 info = 0;
    m_dataStream >> info;
    // 000XYRDL
    xgeometry.m_attribute = onSigned(); // FIXME integer?
    if (info & 1) { // L
        m_layer = onUnsigned();
    }
    xgeometry.m_layer = m_layer;
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
    }
    xgeometry.m_datatype = m_datatype;
    xgeometry.m_string = onString(B);

    if ((info >> 4) & 1) { // X
        xgeometry.m_x = onSigned();
        if (m_isXYRelative) {
            xgeometry.m_x += m_geometryX;
        }
    }
    else {
        xgeometry.m_x = m_geometryX;
    }
    if ((info >> 3) & 1) { // Y
        xgeometry.m_y = onSigned();
        if (m_isXYRelative) {
            xgeometry.m_y += m_geometryY;
        }
    }
    else {
        xgeometry.m_y = m_geometryY;
    }
    if ((info >> 2) & 1) { // R
        m_repetition = onRepetition();
        xgeometry.m_repetition = m_repetition;
    }
    m_currentCell->m_xgeometries.append(xgeometry);
}

bool oa::Parser::onCBlock() {}

quint32 oa::Parser::onUnsigned() {}

qint64 oa::Parser::onSigned() {}

// kind == -1, means kind is not read yet
qreal oa::Parser::onReal(int kind) {}

oa::Delta1 oa::Parser::onDelta1() {}

oa::Delta23 oa::Parser::onDelta2() {}

oa::Delta23 oa::Parser::onDelta3() {}

oa::DeltaG oa::Parser::onDeltaG() {}

QString oa::Parser::onString(int type) {}

QSharedPointer<oa::Repetition> oa::Parser::onRepetition() {}

QSharedPointer<oa::PointList> oa::Parser::onPointList() {}

bool oa::Parser::onTableOffset() {}

oa::Parser::IntervalType oa::Parser::onInterval() {}
