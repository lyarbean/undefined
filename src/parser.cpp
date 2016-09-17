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
#include <stdexcept>

oa::Parser::Parser(Layout& layout)
    : m_layout(layout), m_offsetFlag(true),
      m_cellNameMode(Default),
      m_textStringMode(Default),
      m_propNameMode(Default),
      m_propStringMode(Default),
      m_xNameMode(Default),
      m_cellLocalNameReference(0),
      m_cellNameReference(0),
      m_textStringReference(0), 
      m_propNameReference(0),
      m_propStringReference(0),
      m_xNameReference(0)
{
    qWarning("Initialize Parser");
    undefineModalVariables();
}

bool oa::Parser::open(const QString& filename) {
    m_dataStream.reset(new QFile(filename));
    if (!m_dataStream->open(QIODevice::ReadOnly)) {
        return false;
    }
    bool r = false;
    try {
        static char magicBytes[13];
        if (m_dataStream->read(magicBytes, 13) != 13) {
            throw std::domain_error("open: Read error");
        }
        if (strcmp(magicBytes, "%SEMI-OASIS\r\n")) {
            throw std::domain_error("No magicBytes found");

        }
        r = parse();
    } catch (std::exception& e) {
        qWarning(e.what());
    }
    m_dataStream.reset();
    return r;
}

bool oa::Parser::parse() {
    // Handle START and END first
    if (!beginStart()) {
        throw std::domain_error("Fail to parse beginStart");
    }
    // A2-1.1 Any file-level standard properties must appear immediately after the START record in an OASIS file. Use of
    // file-level standard properties is optional—OASIS processors may omit/ignore any or all of them.

    while (!m_dataStream->atEnd()) {
        quint8 type = m_dataStream->peek(1)[0];
        switch(type) {
        case CBLOCK:
        case PAD:
        case PROPERTY:
        case PROPERTYX:
        case CELL:
        case CELLX:
        case CELLNAME ... LAYERNAMEX:
            if (!nextRecord()) {
                throw std::domain_error("parse: Read error");
            }
            break;
        case END:
            if (m_dataStream->pos() != m_dataStream->size() - 256) {
                throw std::domain_error("parse End Read error");
            }
            return true;
        case START: // START again
            throw std::domain_error("parse: START again? Parse Error");
            break;
        default:
            throw std::domain_error("parse: Parse Error");
            break;
        }
    }
}

bool oa::Parser::beginStart() {
    quint8 type = onUnsigned();
    if (type != 1) {
        throw std::domain_error("beginStart: No START?");
    }
    return onStart();
}


bool oa::Parser::nextRecord()
{
    quint64 type = onUnsigned();
    switch (type) {
    case PAD:
        return onPad();
    case CELLNAME:
    case CELLNAMEX:
        return onCellName(type);
    case TEXTSTRING:
    case TEXTSTRINGX:
        return onTextString(type);
    case PROPNAME:
    case PROPNAMEX:
        return onPropName(type);
    case PROPSTRING:
    case PROPSTRINGX:
        return onPropString(type);
    case LAYERNAME:
    case LAYERNAMEX:
        return onLayerName(type);
    case CELL:
    case CELLX:
        return onCell(type);
    case XYABSOLUTE:
        return onXYAbsolute();
    case XYRELATIVE:
        return onXYRelative();
    case PLACEMENT:
    case PLACEMENTX:
        return onPlacement(type);
    case TEXT:
        return onText();
    case RECTANGLE:
        return onRectangle();
    case POLYGON:
        return onPolygon();
    case PATH:
        return onPath();
    case TRAPEZOID:
    case TRAPEZOIDX:
    case TRAPEZOIDY:
        return onTrapezoid(type);
    case CTRAPEZOID:
        return onCTrapezoid();
    case CIRCLE:
        return onCircle();
    case PROPERTY:
    case PROPERTYX:
        return onProperty(type);
    case XNAME:
    case XNAMEX:
        return onXName(type);
    case XELEMENT:
        return onXElement();
    case XGEOMETRY:
        return onXGeometry();
    case CBLOCK:
        return onCBlock();
    default:
        throw std::domain_error(QString("nextRecord: unintended type:%1").arg(type).toStdString());
    }
}
bool oa::Parser::onStart()
{
    QString version = onString(N);
    if (version != "1.0") {
        throw std::domain_error(QString("onStart: invalid VERSION [%1][%2]").arg(version).arg(version.count()).toStdString());
    }
    m_layout.m_unit = onReal(); // validation
    qint64 currentPos;
    m_offsetFlag = onUnsigned();

    if (!m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_tableOffsets << onUnsigned(); // TODO rename
        }
        currentPos = m_dataStream->pos();
    } else { // onEndEx()
        currentPos = m_dataStream->pos();
        qint64 endOffset = m_dataStream->size() - 255; // END at -256
        m_dataStream->seek(endOffset);
        if (!onEnd()) {
            throw std::domain_error("fail to parse onEnd");
        }
    }
    if (!readTableOffsets()) {
        throw std::domain_error("fail to parse readTableOffsets");
    }
    m_dataStream->seek(currentPos);
    return true;
}


bool oa::Parser:: readTableOffsets() {
    quint32 cellnameOffset, textstringOffset, propnameOffset, propstringOffset, layernameOffset, xnameOffset;
    cellnameOffset = m_tableOffsets.at(1);
    textstringOffset = m_tableOffsets.at(3);
    propnameOffset = m_tableOffsets.at(5);
    propstringOffset = m_tableOffsets.at(7);
    layernameOffset = m_tableOffsets.at(9);
    xnameOffset = m_tableOffsets.at(11);

    if (cellnameOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(cellnameOffset);
        while (true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 3 || type == 4) {
                m_dataStream->read(1);
                if(!onCellName(type)) {
                    throw std::domain_error("readTableOffsets: onCellName");
                }
            } else {
                break;
            }
        }
    }
    if (textstringOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(textstringOffset);
        while (true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 5 || type == 6) {
                m_dataStream->read(1);
                if(!onTextString(type)) {
                    throw std::domain_error("readTableOffsets: onTextString");
                }
            } else {
                break;
            }
        }
    }
    if (propnameOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(propnameOffset);
        while (true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 7 || type == 8) {
                m_dataStream->read(1);
                if(!onPropName(type)) {
                    throw std::domain_error("readTableOffsets: onPropName");
                }
            } else {
                break;
            }
        }
    }
    if (propstringOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(propstringOffset);
        while (true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 9 || type == 10) {
                m_dataStream->read(1);
                if(!onPropString(type)) {
                    throw std::domain_error("readTableOffsets: onPropString");
                }
            } else {
                break;
            }
        }
    }
    if (layernameOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(layernameOffset);
        while (true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 11 || type == 12) {
                m_dataStream->read(1);
                if(!onLayerName(type)) {
                    throw std::domain_error("readTableOffsets: onLayerName");
                }
            } else {
                break;
            }
        }
    }
    if (xnameOffset) {
        // peek first since we're doing read ahead
        m_dataStream->seek(xnameOffset);
        while(true) {
            quint8 type = m_dataStream->peek(1)[0];
            if (type == 30 || type == 31) {
                m_dataStream->read(1);
                if(!onXName(type)) {
                    throw std::domain_error("readTableOffsets: onXName");
                }
            } else {
                break;
            }
        }
    }
    return true;
}

bool oa::Parser::onPad() {
    return true;
}

// TODO CRC
bool oa::Parser::onEnd()
{
    if (m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_tableOffsets << onUnsigned();
        }
    }
    return true;
    // TODO validation
    // padding string
//     int size = 255 - (m_offsetFlag ? 12 : 0);
//     char endBytes[256];
//     return m_dataStream->read(endBytes, size) == size;
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
        throw std::domain_error("Both Implicit and Explicit CellName!");
    }
    quint32 reference;
    if (m_cellNameMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_cellNameReference++;
    }

    auto namedCell = m_layout.m_cells.find(reference);
    if (namedCell != m_layout.m_cells.end()) {
        if (!namedCell->m_name.isEmpty()) {
            throw std::domain_error("Duplicated CELLNAME reference-number");
        }
    }
    // update or create
    m_layout.m_cells[reference].m_name = name;
    m_layout.m_cellNameToReference[name] = reference;
    // A2-2.1 Any cell-level standard properties must appear immediately after the corresponding CELLNAME record in
    // an OASIS file. Use of cell-level standard properties is optional—OASIS processors may omit/ignore any or all of
    // them.
    // TODO try to handle cell properties immediately if we care
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
        throw std::domain_error("Both Implicit and Explicit textString!");
    }
    quint32 reference;
    if (m_textStringMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_textStringReference;
        m_textStringReference++;
    }
    if (m_layout.m_textStrings.find(reference) != m_layout.m_textStrings.end()) {
        throw std::domain_error("Duplicated TEXTSTRING reference-number");
    }
    m_layout.m_textStrings.insert(reference, name);
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
        throw std::domain_error("Both Implicit and Explicit PropName!");
    }
    quint32 reference;
    if (m_propNameMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_propNameReference++;
    }
    if (m_layout.m_propNames.find(reference) != m_layout.m_propNames.end()) {
        throw std::domain_error("Duplicated PROPNAME reference-number");
    }
    m_layout.m_propNames.insert(reference, name);
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
        throw std::domain_error("Both Implicit and Explicit PropString!");
    }
    quint32 reference;
    if (m_propStringMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_propStringReference++;
    }
    if (m_layout.m_propStrings.find(reference) != m_layout.m_propStrings.end()) {
        throw std::domain_error("Duplicated PROPSTRING reference-number");
    }
    m_layout.m_propStrings.insert(reference, name);
    return true;
}

bool oa::Parser::onLayerName(int type)
{
    Q_UNUSED(type);
//     qWarning("onLayerName %d %d", m_dataStream->pos(), m_dataStream->size());
    QString name = onString(N);
//     qWarning("onLayerName %d %d", m_dataStream->pos(), m_dataStream->size());
    IntervalType interval = onInterval();
    // TODO Merge
    QVector<QPair<quint32, quint32>>& dataLayers = m_layout.m_layerNames[name];
    dataLayers.append(interval);
    return true;
}

bool oa::Parser::onCell(int type)
{
    QSharedPointer<Cell> cell(new Cell);
    m_currentCell = cell;
    if (type == 13) {
        quint32 cellReference = onUnsigned();
        auto c = m_layout.m_cells.find(cellReference);
        if (c != m_layout.m_cells.end()) {
            if (c->m_cell) {
                qWarning("Duplicated  cell reference-number");
            }
            c->m_cell = cell;
        } else {
            // name is not read yet, shall be read in a CellName record
            m_layout.m_cells[cellReference].m_cell = cell;
        }
    } else {
        // TODO name duplication check
        QString name = onString(N);
        m_cellLocalNameReference ++;
        m_layout.m_cells[- static_cast<qint64>(m_cellLocalNameReference)] = {name, cell};
        m_layout.m_cellNameToReference[name] = -static_cast<qint64>(m_cellLocalNameReference);
    }
    // reset modal variables
    undefineModalVariables();
    while (!m_dataStream->atEnd()) {
        quint8 type = m_dataStream->peek(1)[0];
        switch(type) {
        case CBLOCK:
        case PAD:
        case XYABSOLUTE ... PROPERTYX:
        case XELEMENT:
        case XGEOMETRY:
            if (!nextRecord()) {
                throw std::domain_error("onCell: parse error");
            }
            break;
        case START: // START again
            throw std::domain_error("onCell: START again");
            break;
        default:
            return true; // Handle on parse()
            break;
        }
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
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onPlacement: read error");
    }
    Placement placement;
    // CNXYRAAF or CNXYRMAF
    if (info >> 7) { // C
        if ((info >> 6) & 1) { // N
            m_placementCell = onUnsigned();
            if (m_layout.m_cells.find(m_placementCell) == m_layout.m_cells.end()) {
                // FIXME CellName not read yet
//                 return false;
            }
            placement.m_cellName = m_layout.m_cells[m_placementCell].m_name;
        } else {
            placement.m_cellName = onString(N);
        }
        m_modalVariableSetStatus.m_d.placementCell = 1;
    } else if (!m_modalVariableSetStatus.m_d.placementCell) {
        throw std::domain_error("Modal variable placementCell is undefined");
    }  else {
        placement.m_cellName = m_layout.m_cells[m_placementCell].m_name;
    }

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
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_placementX += x;
        } else {
            m_placementX = x;
        }
    }

    placement.m_x = m_placementX;

    if ((info >> 4) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_placementY += y;
        } else {
            m_placementY = y;
        }
    }
    placement.m_y = m_placementY;

    if ((info >> 3) & 1) { // R
        if(!onRepetition()) {
             throw std::domain_error("onPlacement: no Repetition");
        }
        placement.m_repetition = m_repetition;
    }
    placement.m_flip = (info & 0b1); // F
    m_currentCell->m_placements.append(placement);
    // TODO onProperty
    return true;
}

bool oa::Parser::onText()
{
    Text text;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onText: read error");
    }
    // 0CNXYRTL
    if (info >> 6) { // C
        if ((info >> 5) & 1) { // N
            // t3.4.oas
            // FIXME could be ;empty now?
            quint32 reference = onUnsigned();
            if (m_layout.m_textStrings.find(reference) == m_layout.m_textStrings.end()) {
                throw std::domain_error("onText: TEXTSTRING not found");
            }
            m_textString = m_layout.m_textStrings[reference]; 
        }
        else {
            m_textString = onString(N);
        }
        qWarning(m_textString.toStdString().c_str());
        m_modalVariableSetStatus.m_d.textString = 1;
    }

    if (info & 1) { // L
        m_textLayer = onUnsigned();
        m_modalVariableSetStatus.m_d.textlayer = 1;
    }

    if ((info >> 1) & 1) { // T
        m_textType = onUnsigned();
        m_modalVariableSetStatus.m_d.texttype = 1;
    }

    if (!(m_modalVariableSetStatus.m_d.texttype & m_modalVariableSetStatus.m_d.textlayer
            & m_modalVariableSetStatus.m_d.textString)) {
        throw std::domain_error("onText: required modal variables not set");
    }

    text.m_string = m_textString;
    text.m_textLayer = m_textLayer;
    text.m_textType = m_textType;

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_textX += x;
        } else {
            m_textX = x;
        }
    }
    text.m_x = m_textX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_textY += y;
        } else {
            m_textY = y;
        }
    }
    text.m_y = m_textY;

    if ((info >> 2) & 1) { // R
        if (!onRepetition()) {
            throw std::domain_error("No Repetition");
        }
        text.m_repetition = m_repetition;
    }
    m_currentCell->m_texts.append(text);
    return true;
}

bool oa::Parser::onRectangle()
{
    Rectangle rectangle;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onRectangle: read error");
    }
    // SWHXTRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        if ((info >> 7) & 1) { // S
            qWarning("A squred rectangle??");
        }
        m_geometryH = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryH = 1;
    } else if ((info >> 7) & 1) { // S
        m_geometryH = m_geometryW;
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
    rectangle.m_layer = m_layer;
    rectangle.m_datatype = m_datatype;
    rectangle.m_width = m_geometryW;
    rectangle.m_height = m_geometryH;
    if (!(m_modalVariableSetStatus.m_d.layer & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.geometryW  & m_modalVariableSetStatus.m_d.geometryH)) {
        throw std::domain_error("onRectangle: required modal variables not set");
    }
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    rectangle.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    rectangle.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        rectangle.m_repetition = m_repetition;
    }
    m_currentCell->m_rectangles.append(rectangle);
    return true;
}

bool oa::Parser::onPolygon()
{
    Polygon polygon;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onPolygon: read error");
    }
    // 00PXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }

    if ((info >> 5) & 1) { // P
        if(!onPointList(true)) {
            return false;
        }
        m_modalVariableSetStatus.m_d.polygonPointList = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.polygonPointList)) {
        throw std::domain_error("onPolygon: required modal variables not set");
    }
    polygon.m_layer = m_layer;
    polygon.m_datatype = m_datatype;
    polygon.m_pointList = m_polygonPointList;

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    polygon.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    polygon.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        polygon.m_repetition = m_repetition;
    }
    m_currentCell->m_polygons.append(polygon);
    return true;
}


bool oa::Parser::onPath()
{
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onPath: read error");
    }
    // EWPXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }

    if ((info >> 6) & 1) { // W
        m_halfWidth = onUnsigned();
        m_modalVariableSetStatus.m_d.pathHalfwidth = 1;
    }


    if ((info >> 7) & 1) { // E
        quint8 ext = onUnsigned();
        quint8 ss = (ext >> 2) & 3;
        if (ss &  3) { // SS
            m_startExtension = onSigned();
        } else if (ss & 1) { // 0S
            m_startExtension = 0;
        } else if (ss & 2) { // S0
            m_startExtension = m_halfWidth;
        }
        if (ss) {
            m_modalVariableSetStatus.m_d.pathStartExtention = 1;
        }
        quint8 ee = ext & 3;
        if (ee & 3) { // EE
            m_endExtension = onSigned();
        } else if (ee & 1) { // 0E
            m_endExtension = 0;
        } else if (ee & 2) { // E0
            m_endExtension = m_halfWidth;
        }
        if (ee) {
            m_modalVariableSetStatus.m_d.pathEndExtention = 1;
        }
    }


    if ((info >> 5) & 1) { // P
        if(!onPointList(false)) {
            throw std::domain_error("onPath: onPointList");
        }
        m_modalVariableSetStatus.m_d.pathPointList = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.pathHalfwidth
            & m_modalVariableSetStatus.m_d.pathStartExtention
            & m_modalVariableSetStatus.m_d.pathEndExtention
            & m_modalVariableSetStatus.m_d.pathPointList)) {
        throw std::domain_error("onPath: required modal variables not set");
    }

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    Path path;
    path.m_layer = m_layer;
    path.m_datatype = m_datatype;
    path.m_halfWidth = m_halfWidth;
    path.m_startExtension = m_startExtension;
    path.m_endExtension = m_endExtension;
    path.m_pointList = m_pointList;
    path.m_x = m_geometryX;
    path.m_y = m_geometryY;
    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        path.m_repetition = m_repetition;
    }
    m_currentCell->m_paths.append(path);
    return true;
}

bool oa::Parser::onTrapezoid(int type)
{
    Trapezoid trapezoid;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onTrapezoid: read error");
    }
    // 0WHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.geometryH
            & m_modalVariableSetStatus.m_d.geometryW)) {
        throw std::domain_error("onTrapezoid: required modal variables not set");
    }
    trapezoid.m_layer = m_layer;
    trapezoid.m_datatype = m_datatype;
    trapezoid.m_width = m_geometryW;
    trapezoid.m_height = m_geometryH;
    qint64 a = 0, b = 0;
    if (type == 23) {
        a = onSigned();
        b = onSigned();
    } else if (type == 24) {
        a = onSigned();
    } else if (type == 25) {
        b = onSigned();
    }
    // FIXME
    // construct points
    if ((info >> 7) & 1) { // O
        trapezoid.m_points << DeltaValue {0, qMax<qint64>(a, 0)};
        trapezoid.m_points << DeltaValue {0, m_geometryH + qMin<qint64>(b, 0)};
        trapezoid.m_points << DeltaValue {m_geometryW, m_geometryH - qMax<qint64>(b, 0)};
        trapezoid.m_points << DeltaValue {m_geometryW, -qMin<qint64>(a, 0)};
    } else {
        trapezoid.m_points << DeltaValue {qMax<qint64>(a, 0), m_geometryH};
        trapezoid.m_points << DeltaValue {m_geometryW + qMin<qint64>(b, 0), m_geometryH};
        trapezoid.m_points << DeltaValue {m_geometryW - qMax<qint64>(b, 0), 0};
        trapezoid.m_points << DeltaValue {-qMin<qint64>(a, 0), 0};
    }
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    trapezoid.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    trapezoid.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        trapezoid.m_repetition = m_repetition;
    }
    m_currentCell->m_trapezoids.append(trapezoid);
}

bool oa::Parser::onCTrapezoid()
{
    CTrapezoid ctrapezoid;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onCTrapezoid: read error");
    }
    // TWHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 7) & 1) { // T
        m_ctrapezoidType = onUnsigned();
        m_modalVariableSetStatus.m_d.ctrapezoidType = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned();
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.ctrapezoidType
            & m_modalVariableSetStatus.m_d.geometryH
            & m_modalVariableSetStatus.m_d.geometryW)) {
        throw std::domain_error("onCTrapezoid: required modal variables not set");
    }
    ctrapezoid.m_layer = m_layer;
    ctrapezoid.m_datatype = m_datatype;
    ctrapezoid.m_type = m_ctrapezoidType;
    ctrapezoid.m_width = m_geometryW;
    ctrapezoid.m_height = m_geometryH;
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    ctrapezoid.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    ctrapezoid.m_y = m_geometryY;
    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        ctrapezoid.m_repetition = m_repetition;
    }
    m_currentCell->m_ctrapezoids.append(ctrapezoid);
}

bool oa::Parser::onCircle()
{
    Circle circle;
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onCircle: read error");
    }
    // 00rXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 5) & 1) { // r
        m_circleRadius = onUnsigned();
        m_modalVariableSetStatus.m_d.circleRadius = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.circleRadius)) {
        throw std::domain_error("onCircle: required modal variables not set");
    }
    circle.m_layer = m_layer;
    circle.m_datatype = m_datatype;
    circle.m_radius = m_circleRadius;
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    circle.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    circle.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        circle.m_repetition = m_repetition;
    }
    m_currentCell->m_circles.append(circle);
}

// TODO How to associate properties to elements
// If no m_currentCell set, then this property applies to the whole layout
bool oa::Parser::onProperty(int type)
{
    if (type == 29) {
        if (!m_modalVariableSetStatus.m_d.lastPropertyName) {
            throw std::domain_error("onProperty: type 29, but lastPropertyName is not set");
        }
        return true; // m_lastPropertyName
    }
    quint8 info = 0;
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onProperty: read error");
    }
    // UUUUVCNS
    if ((info >> 2) & 1) { // C
        if ((info >> 1) & 1) { // N
            quint64 i = onUnsigned();
            if (m_layout.m_propNames.find(i) == m_layout.m_propNames.end()) {
                // TODO Klayout introduces a forward  name table to delay reference
                // we assume we are in strict mode
                throw std::domain_error("PropertyName not found");
            }
            m_lastPropertyName = m_layout.m_propNames[i];
        }
        else {
            m_lastPropertyName = onString(N);
        }
        m_modalVariableSetStatus.m_d.lastPropertyName = 1;
    }
    quint32 valueCount = 0;
    if ((info >> 3) & 1) { // V
        if (info >> 4) {
            throw std::domain_error("onProperty: V=1 but UUUU != 0");
        }
        if (!m_modalVariableSetStatus.m_d.lastValueList) {
            throw std::domain_error("onProperty: lastValueList not set");
        }
        return true;
    } else {
        quint8 uuuu = info >> 4;
        if (uuuu == 15) {
            valueCount = onUnsigned();
        }
        if (uuuu < 15) {
            valueCount = uuuu;
        }
        for (auto i = 0; i < valueCount; ++i) {
            quint8 kind = 0;
            if (m_dataStream->read((char*)&kind, 1) != 1) {
                return false;
            }
            switch (kind) {
            case 0 ... 7:
                m_lastValuesList << onReal();
                break;
            case 8:
                m_lastValuesList << onUnsigned();
                break;
            case 9:
                m_lastValuesList << onSigned();
                break;
            case 10:
                m_lastValuesList << onString(A);
                break;
            case 11:
                m_lastValuesList << onString(B);
                break;
            case 12:
                m_lastValuesList << onString(N);
                break;
            case 13 ... 15:
                m_lastValuesList << onUnsigned();
                break;
            default:
                // parse error
                throw std::domain_error("onProperty: invalid lastValueList kind");
                break;
            }
            if (kind < 16) {
                m_modalVariableSetStatus.m_d.lastValueList = 1;
            }
        }
        return true;
    }
}

bool oa::Parser::onXName(int type)
{
    if (m_xNameMode == Default) {
        if (type == 30) {
            m_xNameMode = Implicit;
        }
        else {
            m_xNameMode = Explicit;
        }
    }
    if ((m_xNameMode == Implicit && type == 31) || (m_xNameMode == Explicit && type == 30)) {
        throw std::domain_error("Both Implicit and Explicit XName!");
    }

    XName xname;
    xname.m_attribute = onUnsigned();
    xname.m_string = onString(N); // ABN
    quint32 reference;
    if (m_xNameMode == Explicit) {
        reference = onUnsigned();
    }
    else {
        reference = m_xNameReference++;
    }
    m_layout.m_xNames[reference] = xname;

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
    if(m_dataStream->read((char*) &info, 1) != 1) {
        throw std::domain_error("onXGeometry: read error");
    }
    // 000XYRDL
    xgeometry.m_attribute = onSigned();
    if (info & 1) { // L
        m_layer = onUnsigned();
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned();
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype)) {
        return false;
    }
    xgeometry.m_layer = m_layer;
    xgeometry.m_datatype = m_datatype;
    xgeometry.m_string = onString(B);

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned();
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
    xgeometry.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned();
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    xgeometry.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        if(!onRepetition()) {
            return false;
        }
        xgeometry.m_repetition = m_repetition;
    }
    m_currentCell->m_xgeometries.append(xgeometry);
}

bool oa::Parser::onCBlock() {
    quint64 comType = onUnsigned();
    quint64 uncomByteCount = onUnsigned();
    quint64 comByteCount = onUnsigned();
    QVarLengthArray<char, 1024> cblockData(comByteCount);
    if (m_dataStream->read(cblockData.data(), comByteCount) != comByteCount)
    {
        throw std::domain_error("onCBlock: read error");
    }
    // TODO handle cblockData
    return true;
}

quint64 oa::Parser::onUnsigned() {
    quint64 v = 0;
    quint64 p = 1;
    char c = 0;
    static quint64 threshold = std::numeric_limits <quint64>::max () >> 7;

    for (;;) {
        if (m_dataStream->atEnd()) {
            /*abort();*/
            throw std::domain_error("Fail atEnd");
        }
        if (m_dataStream->read(&c, 1) != 1) {
            throw std::domain_error("fail to read ");
        }
        if (p > threshold &&
                (quint64) (c & 0x7F) > (std::numeric_limits <quint64>::max () / p)) {
            throw std::domain_error("onUnsigned: overflow");
        }

        v += static_cast<quint64> (c & 0x7F) * p;
        p <<= 7;
        if (!(c & 0x80)) {
            break;
        }
    };
    return v;
}

qint64 oa::Parser::onSigned() {
    quint64 us = onUnsigned();
    if (us & 1)
    {
        return - static_cast<qint64> (us >> 1);
    }
    return static_cast<qint64> (us >> 1);
}

double oa::Parser::onReal() {
    quint64 k = onUnsigned();
    switch(k) {
    case 0:
        return onUnsigned();
    case 1:
        return -onUnsigned();
    case 2:
        return 1.0 / onUnsigned();
    case 3:
        return - 1.0 / onUnsigned();
    case 4:
        return ((double) onUnsigned()) / ((double) onUnsigned());
    case 5:
        return - ((double) onUnsigned()) / ((double) onUnsigned());
    case 6:
    {
        float v;
        if(m_dataStream->read((char*) &v, sizeof(float)) != sizeof(float)) {

            throw std::domain_error("onReal: fail to read float");
        }
        return v;
    }
    case 7:
    {
        double v;
        if(m_dataStream->read((char*) &v, sizeof(double)) != sizeof(double)) {
            throw std::domain_error("onReal: fail to read double");
        }
        return v;
    }
    default:
        throw std::domain_error("onReal: fail to read real");
    }
}


oa::Delta23 oa::Parser::onDelta2() {
    return Delta23(onUnsigned(), true);
}

oa::Delta23 oa::Parser::onDelta3() {
    return Delta23(onUnsigned(), false);
}


oa::DeltaG oa::Parser::onDeltaG() {
    quint64 v = onUnsigned();
    if (v & 1) {
        return DeltaG(v);
    }
    qint64 x = (v & 2) ? - static_cast<qint64>(v >> 2) : static_cast<qint64>(v >> 2);
    return DeltaG(x, onSigned());
}

QString oa::Parser::onString(StringType type) {
    quint32 len = onUnsigned();
    if (len > 1024) {
        qWarning("A long string");
    }
//     qWarning("onString %d %d", len, m_dataStream->pos());
    QVarLengthArray<char, 1024> s(len);
    if (m_dataStream->read(s.data(), len) != len)
    {
//         abort();
        throw std::domain_error("onString: read error");
    }
    qWarning(QString::fromLatin1(s.constData(), len).toStdString().c_str());
    return QString::fromLatin1(s.constData(), len);
}

bool oa::Parser::onRepetition() {
    quint8 type = onUnsigned();
    if (type && type < 12) {
        m_modalVariableSetStatus.m_d.repetition = 1;
    }
    switch (type)
    {
    case 0:
        return m_modalVariableSetStatus.m_d.repetition == 1;
    case 1: {
        m_repetition.reset(new Repetition1(onUnsigned(), onUnsigned(), onUnsigned(), onUnsigned()));
        return true;
    }
    case 2: {
        m_repetition.reset(new Repetition2(onUnsigned(), onUnsigned()));
        return true;
    }
    case 3: {
        m_repetition.reset(new Repetition3(onUnsigned(), onUnsigned()));
        return true;
    }
    case 4: {
        quint64 dx = onUnsigned(); // n - 2
        QVector<quint32> sxz;
        for (quint64 i = 0; i < dx + 2; ++i) {
            sxz << onUnsigned();
        }
        m_repetition.reset(new Repetition4(dx, sxz));
        return true;
    }
    case 5: {
        quint64 dx = onUnsigned(); // n - 2
        quint64 g = onUnsigned();
        QVector<quint32> sxz;
        for (quint64 i = 0; i < dx + 2; ++i) {
            sxz << onUnsigned();
        }
        m_repetition.reset(new Repetition5(dx, g, sxz));
        return true;
    }
    case 6: {
        quint64 dy = onUnsigned(); // n - 2
        QVector<quint32> syz;
        for (quint64 i = 0; i < dy + 2; ++i) {
            syz << onUnsigned();
        }
        m_repetition.reset(new Repetition6(dy, syz));
        return true;
    }
    case 7: {
        quint64 dy = onUnsigned(); // n - 2
        quint64 g = onUnsigned();
        QVector<quint32> syz;
        for (quint64 i = 0; i < dy + 2; ++i) {
            syz << onUnsigned();
        }
        m_repetition.reset(new Repetition7(dy, g, syz));
        return true;
    }
    case 8: {
        quint64 dn = onUnsigned();
        quint64 dm = onUnsigned();
        DeltaG pn = onDeltaG();
        DeltaG pm = onDeltaG();
        m_repetition.reset(new Repetition8(dn, dm, pn.value, pm.value));
        return true;
    }
    case 9: {
        quint64 d = onUnsigned();
        DeltaG p = onDeltaG();
        m_repetition.reset(new Repetition9(d, p.value));
        return true;
    }
    case 10: {
        quint64 d = onUnsigned(); // p - 2
        QVector<DeltaValue> pz;
        for (quint64 i = 0; i < d + 1; ++i) {
            DeltaG p = onDeltaG();
            pz << p.value;
        }
        m_repetition.reset(new Repetition10(d, pz));
        return true;
    }
    case 11: {
        quint64 d = onUnsigned(); // p - 2
        quint64 g = onUnsigned(); // grid;
        QVector<DeltaValue> pz;
        for (quint64 i = 0; i < d + 1; ++i) {
            DeltaG p = onDeltaG();
            pz << p.value;
        }
        m_repetition.reset(new Repetition11(d, g, pz));
        return true;
    }
    default:
        throw std::domain_error("invalid Repetition");
        break;
    }
}

bool oa::Parser::onPointList(bool isPolygon) {
    quint8 type = onUnsigned();
    quint64 count = onUnsigned();
    QSharedPointer<PointList> pointList (new PointList);

    PointList& pl = *pointList;
    switch(type) {
    case 0:
    case 1: {
        bool h = type == 0;
        DeltaValue v = {0, 0};
        for (quint64 i =0; i < count; ++i) {
            // 1-delta
            qint64 d = onSigned();
            if (h) {
                v += {d, 0};
            } else {
                v += {0, d};
            }
            pl << v;
            h = !h;
        }
        if (isPolygon) {
            if (count % 2) {
                qWarning("illegal PointList");
            }
            if (h) {
                pl << DeltaValue {0, v.m_y};
            } else {
                pl << DeltaValue {v.m_x, 0};
            }
        }
        break;
    }
    case 2: {
        DeltaValue v = {0, 0};
        for (quint64 i =0; i < count; ++i) {
            v +=  onDelta2().value;
            pl << v;
        }
        break;
    }
    case 3: {
        DeltaValue v = {0, 0};
        for (quint64 i =0; i < count; ++i) {
            v +=  onDelta3().value;
            pl << v;
        }
        break;
    }
    case 4: {
        DeltaValue v = {0, 0};
        for (quint64 i =0; i < count; ++i) {
            v +=  onDeltaG().value;
            pl << v;
        }
        break;
    }
    case 5: {
        DeltaValue v = {0, 0};
        DeltaValue d = {0, 0};
        for (quint64 i =0; i < count; ++i) {
            d += onDeltaG().value;
            v += d;
            pl << v;
        }
        break;
    }
    default:
        throw std::domain_error("invalid PointList");
        break;
    }
    if (isPolygon) {
        m_polygonPointList = pointList;
        m_modalVariableSetStatus.m_d.polygonPointList = 1;
    } else {
        m_pointList = pointList;
        m_modalVariableSetStatus.m_d.pathPointList = 1;
    }

    return true;
}

oa::Parser::IntervalType oa::Parser::onInterval() {
    quint8 type = onUnsigned();
    switch(type) {
    case 0:
        return IntervalType {0, 1 << 24};
    case 1:
        return IntervalType {0, onUnsigned()};
    case 2:
        return IntervalType {onUnsigned(), 1 << 24};
    case 3: {
        quint64 b = onUnsigned();
        return IntervalType {b, b};
    }
    case 4:
        return IntervalType {onUnsigned(), onUnsigned()};
    }
}

void oa::Parser::undefineModalVariables()
{
    // placement-x, placement-y, geometry-x, geometry-y, text-x, and text-y
    m_placementX = 0;
    m_placementY = 0;
    m_geometryX = 0;
    m_geometryY = 0;
    m_textX = 0;
    m_textY = 0;
    m_isXYRelative = false;
    m_modalVariableSetStatus.m_dummy = 0;
    m_repetition.clear();
    m_polygonPointList.clear();
    m_pointList.clear();
    m_lastValuesList.clear();
}

