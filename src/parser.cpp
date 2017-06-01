#include <GL/gl.h>
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
#include <cmath>
#include <QDataStream>
#include <QFile>
#include <QUuid>
#include <QBuffer>
#include <QDebug>
#include <stdexcept>

oa::Parser::Parser(Layout &layout)
    : m_layout(layout), m_currentCell(nullptr), m_offsetFlag(true),
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
      m_xNameReference(0),
      m_repetitionOffset(0),
      m_repetitionCount(0),
      m_polygonPointListOffset(-1),
      m_polygonPointListCount(-1),
      m_pointListOffset(-1),
      m_pointListCount(-1)
{
    undefineModalVariables();
}

bool oa::Parser::open(const QString &filename)
{
    QFile dataStream(filename);
    if (!dataStream.open(QIODevice::ReadOnly)) {
        return false;
    }
    bool r = false;
    try {
        char magicBytes[13] = {0};
        if (dataStream.read(magicBytes, sizeof(magicBytes)) != sizeof(magicBytes)) {
            throw std::domain_error("open: Read error");
        }
        if (memcmp(magicBytes, "%SEMI-OASIS\r\n",  sizeof(magicBytes))) {
            throw std::domain_error("No magicBytes found");

        }
        r = parse(dataStream);
//         m_layout.put();
    } catch (std::exception &e) {
        qDebug() << e.what();
    }
    dataStream.close();
    return r;
}

bool oa::Parser::parse(QIODevice& dataStream)
{
    // Handle START and END first
    beginStart(dataStream);
    // A2-1.1 Any file-level standard properties must appear immediately after the START record in an OASIS file. Use of
    // file-level standard properties is optional—OASIS processors may omit/ignore any or all of them.
    while (!dataStream.atEnd()) {
        quint8 type = dataStream.peek(1)[0];
        switch (type) {
        case CBLOCK:
        case PAD:
        case PROPERTY:
        case PROPERTYX:
        case CELL:
        case CELLX:
        case CELLNAME ... LAYERNAMEX:
            nextRecord(dataStream);
            break;
        case END: {
            if (dataStream.pos() != dataStream.size() - 256) {
                qWarning("parse %lld / %lld", dataStream.pos(), dataStream.size());
                throw std::domain_error("parse End Read error");
            }
            if (!m_unresolvedCellName.isEmpty()) {
                throw std::domain_error("unresolved cellname");
            }
            if (!m_unresolvedPropName.isEmpty()) {
                throw std::domain_error("unresolved propname");
            }
            if (!m_unresolvedTextString.isEmpty()) {
                throw std::domain_error("unresolved textstring");
            }
            for (auto &namedCell : m_layout.m_cells) {
                if (namedCell.m_name.isEmpty()) {
                    throw std::domain_error("Empty CellName");
                }
            }
            return true;
        }
        case START: // START again
            throw std::domain_error("parse: START again? Parse Error");
            break;
        default:
            throw std::domain_error(QString("parse: Parse Error:%1").arg(type).toStdString());
            break;
        }
    }
    return false; // No END read
}

void oa::Parser::beginStart(QIODevice& dataStream)
{
    quint8 type = onUnsigned(dataStream);
    if (type != 1) {
        throw std::domain_error("beginStart: No START?");
    }
    onStart(dataStream);
}


void oa::Parser::nextRecord(QIODevice& dataStream)
{
    quint64 type = onUnsigned(dataStream);
    switch (type) {
    case PAD:
        onPad(dataStream);
        break;
    case CELLNAME:
    case CELLNAMEX:
        onCellName(dataStream, type);
        break;
    case TEXTSTRING:
    case TEXTSTRINGX:
        onTextString(dataStream, type);
        break;
    case PROPNAME:
    case PROPNAMEX:
        onPropName(dataStream, type);
        break;
    case PROPSTRING:
    case PROPSTRINGX:
        onPropString(dataStream, type);
        break;
    case LAYERNAME:
    case LAYERNAMEX:
        onLayerName(dataStream, type);
        break;
    case CELL:
    case CELLX:
        onCell(dataStream, type);
        break;
    case XYABSOLUTE:
        onXYAbsolute();
        break;
    case XYRELATIVE:
        onXYRelative();
        break;
    case PLACEMENT:
    case PLACEMENTX:
        onPlacement(dataStream, type);
        break;
    case TEXT:
        onText(dataStream);
        break;
    case RECTANGLE:
        onRectangle(dataStream);
        break;
    case POLYGON:
        onPolygon(dataStream);
        break;
    case PATH:
        onPath(dataStream);
        break;
    case TRAPEZOID:
    case TRAPEZOIDX:
    case TRAPEZOIDY:
        onTrapezoid(dataStream, type);
        break;
    case CTRAPEZOID:
        onCTrapezoid(dataStream);
        break;
    case CIRCLE:
        onCircle(dataStream);
        break;
    case PROPERTY:
    case PROPERTYX:
        onProperty(dataStream, type);
        break;
    case XNAME:
    case XNAMEX:
        onXName(dataStream, type);
        break;
    case XELEMENT:
        onXElement(dataStream);
        break;
    case XGEOMETRY:
        onXGeometry(dataStream);
        break;
    case CBLOCK:
        onCBlock(dataStream);
        break;
    default:
        throw std::domain_error(QString("nextRecord: unintended type:%1").arg(type).toStdString());
    }
}
void oa::Parser::onStart(QIODevice& dataStream)
{
    QString version = onString(dataStream, N);
    if (version != "1.0") {
        throw std::domain_error(QString("onStart: invalid VERSION [%1][%2]").arg(version).arg(version.count()).toStdString());
    }
    m_layout.m_unit = onReal(dataStream); // validation
    qint64 currentPos;
    m_offsetFlag = onUnsigned(dataStream);

    if (!m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_tableOffsets << onUnsigned(dataStream); // TODO rename
        }
        currentPos = dataStream.pos();
    } else { // onEndEx()
        currentPos = dataStream.pos();
        qint64 endOffset = dataStream.size() - 255; // END at -256
        dataStream.seek(endOffset);
        onEnd(dataStream);
    }
    readTableOffsets(dataStream);
    dataStream.seek(currentPos);
}


void oa::Parser:: readTableOffsets(QIODevice& dataStream)
{
    quint32 cellnameOffset, textstringOffset, propnameOffset, propstringOffset, layernameOffset, xnameOffset;
    cellnameOffset = m_tableOffsets.at(1);
    textstringOffset = m_tableOffsets.at(3);
    propnameOffset = m_tableOffsets.at(5);
    propstringOffset = m_tableOffsets.at(7);
    layernameOffset = m_tableOffsets.at(9);
    xnameOffset = m_tableOffsets.at(11);

    if (cellnameOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(cellnameOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == RecordType::CELLNAME || type == RecordType::CELLNAMEX) {
                dataStream.read(1);
                onCellName(dataStream, type);
            } else {
                break;
            }
        }
    }
    if (textstringOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(textstringOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == 5 || type == 6) {
                dataStream.read(1);
                onTextString(dataStream, type);
            } else {
                break;
            }
        }
    }
    if (propnameOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(propnameOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == 7 || type == 8) {
                dataStream.read(1);
                onPropName(dataStream, type);
            } else {
                break;
            }
        }
    }
    if (propstringOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(propstringOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == 9 || type == 10) {
                dataStream.read(1);
                onPropString(dataStream, type);
            } else {
                break;
            }
        }
    }
    if (layernameOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(layernameOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == 11 || type == 12) {
                dataStream.read(1);
                onLayerName(dataStream, type);
            } else {
                break;
            }
        }
    }
    if (xnameOffset) {
        // peek first since we're doing read ahead
        dataStream.seek(xnameOffset);
        while (true) {
            quint8 type = dataStream.peek(1)[0];
            if (type == 30 || type == 31) {
                dataStream.read(1);
                onXName(dataStream, type);
            } else {
                break;
            }
        }
    }
}

void oa::Parser::onPad(QIODevice& dataStream)
{
    return;
}

// TODO CRC
void oa::Parser::onEnd(QIODevice& dataStream)
{
    qInfo("onEnd");
    if (m_offsetFlag) {
        for (int i = 0; i < 12; ++i) {
            m_tableOffsets << onUnsigned(dataStream);
        }
    }
    // TODO validation
    QString padding = onString(dataStream, B);
    quint64 scheme = onUnsigned(dataStream);
    switch (scheme) {
    case 0:
        break;
    case 1: {
        char endBytes[4];
        if (dataStream.read(endBytes, 4) != 4) {
            throw std::domain_error("Read signature error");
        }
        // do crc32 validation
        break;
    }
    case 2: {
        char endBytes[4];
        if (dataStream.read(endBytes, 4) != 4) {
            throw std::domain_error("Read signature error");
        }
        // do checksum32 validation
        break;
    }
    default:
        throw std::domain_error("invalid scheme type");
    }
}

void oa::Parser::onCellName(QIODevice& dataStream, int type)
{
    qInfo("onCellName");
    QString name = onString(dataStream, N);
    if (m_cellNameMode == Default) {
        if (type == 3) {
            m_cellNameMode = Implicit;
        } else {
            m_cellNameMode = Explicit;
        }
    }
    if ((m_cellNameMode == Implicit && type == 4) || (m_cellNameMode == Explicit && type == 3)) {
        throw std::domain_error("Both Implicit and Explicit CellName!");
    }
    quint32 reference;
    if (m_cellNameMode == Explicit) {
        reference = onUnsigned(dataStream);
    } else {
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
    if (m_unresolvedCellName.find(reference) != m_unresolvedCellName.end()) {
        QString unresolved = m_unresolvedCellName[reference];
        m_unresolvedCellName.remove(reference);
        for (auto &namedCell : m_layout.m_cells) {
            if (namedCell.m_name == unresolved) {
                namedCell.m_name = name;
            }
            QVector<Placement> &placement = namedCell.m_cell->m_placements;
            for (auto &p : placement) {
                if (p.m_cellName == unresolved) {
                    p.m_cellName = name;
                }

        }            }
        if (m_textString == unresolved) {
            m_textString = name;
        }
        if (m_placementCell == unresolved) {
            m_placementCell = name;
        }
    }
    // A2-2.1 Any cell-level standard properties must appear immediately after the corresponding CELLNAME record in
    // an OASIS file. Use of cell-level standard properties is optional—OASIS processors may omit/ignore any or all of
    // them.

    // TODO try to handle cell properties immediately if we care
}

void oa::Parser::onTextString(QIODevice& dataStream, int type)
{
    qInfo("onTextString");
    QString name = onString(dataStream, N);
    if (m_textStringMode == Default) {
        if (type == 5) {
            m_textStringMode = Implicit;
        } else {
            m_textStringMode = Explicit;
        }
    }
    if ((m_textStringMode == Implicit && type == 6) || (m_textStringMode == Explicit && type == 5)) {
        throw std::domain_error("Both Implicit and Explicit textString!");
    }
    quint32 reference;
    if (m_textStringMode == Explicit) {
        reference = onUnsigned(dataStream);
    } else {
        reference = m_textStringReference;
        m_textStringReference++;
    }
    if (m_layout.m_textStrings.find(reference) != m_layout.m_textStrings.end()) {
        throw std::domain_error("Duplicated TEXTSTRING reference-number");
    }
    m_layout.m_textStrings.insert(reference, name);
    if (m_unresolvedTextString.find(reference) != m_unresolvedTextString.end()) {
        QString unresolved = m_unresolvedTextString[reference];
        m_unresolvedTextString.remove(reference);
        for (auto &namedCell : m_layout.m_cells) {
            QVector<Text> &texts = namedCell.m_cell->m_texts;
            for (auto &t : texts) {
                if (t.m_string == unresolved) {
                    t.m_string = name;
                }
            }
            if (m_textString == unresolved) {
                m_textString = name;
            }
        }
    }
}

void oa::Parser::onPropName(QIODevice& dataStream, int type)
{
    qInfo("onPropName");
    QString name = onString(dataStream, N);
    if (m_propNameMode == Default) {
        if (type == 7) {
            m_propNameMode = Implicit;
        } else {
            m_propNameMode = Explicit;
        }
    }
    if ((m_propNameMode == Implicit && type == 8) || (m_propNameMode == Explicit && type == 7)) {
        throw std::domain_error("Both Implicit and Explicit PropName!");
    }
    quint32 reference;
    if (m_propNameMode == Explicit) {
        reference = onUnsigned(dataStream);
    } else {
        reference = m_propNameReference++;
    }
    if (m_layout.m_propNames.find(reference) != m_layout.m_propNames.end()) {
        throw std::domain_error("Duplicated PROPNAME reference-number");
    }
    m_layout.m_propNames.insert(reference, name);

    if (m_unresolvedPropName.find(reference) != m_unresolvedPropName.end()) {
        QString unresolved = m_unresolvedPropName[reference];
        m_unresolvedPropName.remove(reference);
        m_layout.m_propNames[reference] = name;
        if (m_lastPropertyName == unresolved) {
            m_lastPropertyName = name;
        }
    }
}

void oa::Parser::onPropString(QIODevice& dataStream, int type)
{
    qInfo("onPropString");
    QString name = onString(dataStream, B);
    if (m_propStringMode == Default) {
        if (type == 9) {
            m_propStringMode = Implicit;
        } else {
            m_propStringMode = Explicit;
        }
    }
    if ((m_propStringMode == Implicit && type == 10) || (m_propStringMode == Explicit && type == 9)) {
        throw std::domain_error("Both Implicit and Explicit PropString!");
    }
    quint32 reference;
    if (m_propStringMode == Explicit) {
        reference = onUnsigned(dataStream);
    } else {
        reference = m_propStringReference++;
    }
    if (m_layout.m_propStrings.find(reference) != m_layout.m_propStrings.end()) {
        throw std::domain_error("Duplicated PROPSTRING reference-number");
    }
    m_layout.m_propStrings.insert(reference, name);
}

void oa::Parser::onLayerName(QIODevice& dataStream, int type)
{
    qInfo("onLayerName");
    QString name = onString(dataStream, N);
    IntervalType layerInterval = onInterval(dataStream);
    IntervalType datatypeInterval = onInterval(dataStream);
    if (type == LAYERNAME) {
        QVector<QPair<IntervalType, IntervalType>> &layerName = m_layout.m_layerDatatypeNames[name];
        layerName.append(QPair<IntervalType, IntervalType>(layerInterval, datatypeInterval));
    } else { // LAYERNAMEX
        QVector<QPair<IntervalType, IntervalType>> &layerName = m_layout.m_textLayerDatatypeNames[name];
        layerName.append(QPair<IntervalType, IntervalType>(layerInterval, datatypeInterval));
    }
}

void oa::Parser::onCell(QIODevice& dataStream, int type)
{
    qInfo("onCell");
    // QSharedPointer<Cell> cell(new Cell);
    m_currentCell = new Cell;
    if (type == 13) {
        quint32 cellReference = onUnsigned(dataStream);
        auto c = m_layout.m_cells.find(cellReference);
        if (c != m_layout.m_cells.end()) {
            if (c->m_cell) {
                throw std::domain_error("Duplicated  cell reference-number");
            }
            c->m_cell = m_currentCell;
        } else {
            // name is not read yet, shall be read in a CellName record
            m_layout.m_cells[cellReference].m_cell = m_currentCell;
        }
    } else {
        // TODO name duplication check
        QString name = onString(dataStream, N);
        m_cellLocalNameReference ++;
        m_layout.m_cells[- static_cast<qint64>(m_cellLocalNameReference)] = {name, m_currentCell};
        m_layout.m_cellNameToReference[name] = -static_cast<qint64>(m_cellLocalNameReference);
    }
    // reset modal variables
    undefineModalVariables();
    while (!dataStream.atEnd()) {
        quint8 type = 0;
        if (dataStream.peek((char *)&type, sizeof(type)) != sizeof(type)) {
            throw std::domain_error("fail to peek type");
        }
        switch (type) {
        case CBLOCK:
        case PAD:
        case XYABSOLUTE ... PROPERTYX:
        case XELEMENT:
        case XGEOMETRY:
            nextRecord(dataStream);
            break;
        case START: // START again
            throw std::domain_error("onCell: START again");
            break;
        default:
            return; // Handle on parse()
            break;
        }
    }
}

void oa::Parser::onXYAbsolute()
{
    qInfo("onXYAbsolute");
    m_isXYRelative = false;
}

void oa::Parser::onXYRelative()
{
    qInfo("onXYRelative");
    m_isXYRelative = true;
}

void oa::Parser::onPlacement(QIODevice& dataStream, int type)
{
    qInfo("onPlacement");
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onPlacement: read error");
    }
    Placement placement;
    // CNXYRAAF or CNXYRMAF
    if (info >> 7) { // C
        if ((info >> 6) & 1) { // N
            quint32 reference = onUnsigned(dataStream);
            if (m_layout.m_cells.find(reference) == m_layout.m_cells.end()) {
                // CELLNAME is not read yet
                if (m_unresolvedCellName.find(reference) == m_unresolvedCellName.end()) {
                    m_unresolvedCellName[reference] = QUuid::createUuid().toString();
                }
                m_placementCell = m_unresolvedCellName[reference];
            } else {
                m_placementCell = m_layout.m_cells[reference].m_name;
            }
        } else {
            m_placementCell = onString(dataStream, N);
        }
        m_modalVariableSetStatus.m_d.placementCell = 1;
    }
    if (!m_modalVariableSetStatus.m_d.placementCell) {
        throw std::domain_error("Modal variable placementCell is undefined");
    }

    placement.m_cellName = m_placementCell;
    qreal mag(1.0), angle(0.0);
    if (type == 18) {
        if ((info >> 2) & 1) { // M
            mag = onReal(dataStream);
        }
        if ((info >> 1) & 1) { // A
            angle = onReal(dataStream);
        } else {
            angle = 0.0;
        }
        if (mag <= 0.0) {
            throw std::domain_error("onPlacement: Magnification is not positive");
        }
        if (std::isnan(mag) || std::isinf(mag)) {
            throw std::domain_error("onPlacement: Magnification is out of range");
        }
        if (std::isnan(angle) || std::isinf(angle)) {
            throw std::domain_error("onPlacement: Angle is out of range");
        }
    }
    if ((info >> 5) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_placementX += x;
        } else {
            m_placementX = x;
        }
    }

    // placement.m_x = m_placementX;

    if ((info >> 4) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_placementY += y;
        } else {
            m_placementY = y;
        }
    }
    // placement.m_y = m_placementY;

    if ((info >> 3) & 1) { // R
        onRepetition(dataStream);
        //  placement.m_repetition = m_repetition;
    }
    //FIXME if no repetition
    qint32 flip = (info & 0b1) ? -1 : 1;
    qint32 x00 = cos(angle/90) * mag;
    qint32 x01 = sin(angle/90) * mag;
    qint32 x10 = -flip * x01;
    qint32 x11 = flip * x00;
    for (auto i = 0; i < m_repetitionCount; ++i)
    {
        Placement::X p;
        p.x00 = x00;
        p.x01 = x01;
        p.x10 = x10;
        p.x11 = x11;
        p.x20 = m_placementX + m_layout.m_repetitions[m_repetitionOffset + i].m_x;
        p.x21 = m_placementY + m_layout.m_repetitions[m_repetitionOffset + i].m_y;
        placement.m_matrixes.append(p);
    }
    m_currentCell->m_placements.append(placement);
    // TODO onProperty
}

void oa::Parser::onText(QIODevice& dataStream)
{
    qInfo("onText");
    Text text;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onText: read error");
    }
    // 0CNXYRTL
    if (info >> 6) { // C
        if ((info >> 5) & 1) { // N
            quint32 reference = onUnsigned(dataStream);
            if (m_layout.m_textStrings.find(reference) == m_layout.m_textStrings.end()) {
                if (m_unresolvedTextString.find(reference) == m_unresolvedTextString.end()) {
                    m_unresolvedTextString[reference] = QUuid::createUuid().toString();
                }
                m_textString = m_unresolvedTextString[reference];
            } else {
                m_textString = m_layout.m_textStrings[reference];
            }
        } else {
            m_textString = onString(dataStream, N);
        }
        m_modalVariableSetStatus.m_d.textString = 1;
    }

    if (info & 1) { // L
        m_textLayer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.textlayer = 1;
    }

    if ((info >> 1) & 1) { // T
        m_textType = onUnsigned(dataStream);
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
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_textX += x;
        } else {
            m_textX = x;
        }
    }
    text.m_x = m_textX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_textY += y;
        } else {
            m_textY = y;
        }
    }
    text.m_y = m_textY;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
        text.m_repetitionOffset = m_repetitionOffset;
        text.m_repetitionCount = m_repetitionCount;
    }
    m_currentCell->m_texts.append(text);
}

void oa::Parser::onRectangle(QIODevice& dataStream)
{
    qInfo("onRectangle");
    // Rectangle rectangle;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onRectangle: read error");
    }
    // SWHXTRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        if ((info >> 7) & 1) { // S
            throw std::domain_error("onRectangle: S and H are set");
        }
        m_geometryH = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryH = 1;
    } else if ((info >> 7) & 1) { // S
        m_geometryH = m_geometryW;
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
//     rectangle.m_layer = m_layer;
//     rectangle.m_datatype = m_datatype;
//     rectangle.m_width = m_geometryW;
//     rectangle.m_height = m_geometryH;
    if (!(m_modalVariableSetStatus.m_d.layer & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.geometryW  & m_modalVariableSetStatus.m_d.geometryH)) {
        throw std::domain_error("onRectangle: required modal variables not set");
    }
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     rectangle.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     rectangle.m_y = m_geometryY;
    // Generate Rectangle
    Mesh mesh;
    mesh.m_datatype = m_datatype;
    mesh.m_layer = m_layer;
    mesh.m_oasisType = (qint16)RECTANGLE;
    mesh.m_drawType = (qint16)GL_TRIANGLES;
    mesh.m_baseVertex = m_layout.m_vertexes.size();
    m_layout.m_vertexes.append(DeltaValue{m_geometryX, m_geometryY});
    m_layout.m_vertexes.append(DeltaValue{m_geometryX + m_geometryW, m_geometryY});
    m_layout.m_vertexes.append(DeltaValue{m_geometryX + m_geometryW, m_geometryY + m_geometryH});
    m_layout.m_vertexes.append(DeltaValue{m_geometryX, m_geometryY + m_geometryH});
    mesh.m_vertexCount = 4;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
        mesh.m_repetitionOffset = m_repetitionOffset;
        mesh.m_repetitionCount = m_repetitionCount;
    }
    m_currentCell->m_meshes.append(mesh);
    // m_currentCell->m_rectangles.append(rectangle);
}

void oa::Parser::onPolygon(QIODevice& dataStream)
{
    qInfo("onPolygon");
//     Polygon polygon;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onPolygon: read error");
    }
    // 00PXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }

    if ((info >> 5) & 1) { // P
        onPointList(dataStream, true);
        m_modalVariableSetStatus.m_d.polygonPointList = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.polygonPointList)) {
        throw std::domain_error("onPolygon: required modal variables not set");
    }
//     polygon.m_layer = m_layer;
//     polygon.m_datatype = m_datatype;
//     polygon.m_pointList = m_polygonPointList;

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     polygon.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     polygon.m_y = m_geometryY;

//     if ((info >> 2) & 1) { // R
//         onRepetition(dataStream);
//         polygon.m_repetition = m_repetition;
//     }
//     m_currentCell->m_polygons.append(polygon);
    Mesh mesh;
    mesh.m_datatype = m_datatype;
    mesh.m_layer = m_layer;
    mesh.m_oasisType = (qint16)RECTANGLE;
    mesh.m_drawType = (qint16)GL_TRIANGLES;
    mesh.m_baseVertex = m_polygonPointListOffset;
    mesh.m_vertexCount = m_polygonPointListCount;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
        mesh.m_repetitionOffset = m_repetitionOffset;
        mesh.m_repetitionCount = m_repetitionCount;
    }
    m_currentCell->m_meshes.append(mesh);
}


void oa::Parser::onPath(QIODevice& dataStream)
{
    qInfo("onPath");
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onPath: read error");
    }
    // EWPXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }

    if ((info >> 6) & 1) { // W
        m_halfWidth = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.pathHalfwidth = 1;
    }

    if ((info >> 7) & 1) { // E
        quint8 ext = onUnsigned(dataStream);
        quint8 ss = (ext >> 2) & 3;
        if (ss == 3) { // SS
            m_startExtension = onSigned(dataStream);
        } else if (ss == 1) { // 0S
            m_startExtension = 0;
        } else if (ss == 2) { // S0
            m_startExtension = m_halfWidth;
        }
        if (ss) {
            m_modalVariableSetStatus.m_d.pathStartExtention = 1;
        }
        quint8 ee = ext & 3;
        if (ee == 3) { // EE
            m_endExtension = onSigned(dataStream);
        } else if (ee == 1) { // 0E
            m_endExtension = 0;
        } else if (ee == 2) { // E0
            m_endExtension = m_halfWidth;
        }
        if (ee) {
            m_modalVariableSetStatus.m_d.pathEndExtention = 1;
        }

    }

    if ((info >> 5) & 1) { // P
        onPointList(dataStream, false);
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
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
    Mesh mesh;
    mesh.m_datatype = m_datatype;
    mesh.m_layer = m_layer;
    mesh.m_oasisType = (qint16)RECTANGLE;
    mesh.m_drawType = (qint16)GL_TRIANGLES;
    mesh.m_baseVertex = m_pointListOffset;
    mesh.m_vertexCount = m_pointListCount;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
        mesh.m_repetitionOffset = m_repetitionOffset;
        mesh.m_repetitionCount = m_repetitionCount;
    }
    m_currentCell->m_meshes.append(mesh);
    
//     Path path;
//     path.m_layer = m_layer;
//     path.m_datatype = m_datatype;
//     path.m_halfWidth = m_halfWidth;
//     path.m_startExtension = m_startExtension;
//     path.m_endExtension = m_endExtension;
//     path.m_pointList = m_pointList;
//     path.m_x = m_geometryX;
//     path.m_y = m_geometryY;
//     if ((info >> 2) & 1) { // R
//         onRepetition(dataStream);
//         path.m_repetition = m_repetition;
//     }
//     m_currentCell->m_paths.append(path);
}

// FIXME
void oa::Parser::onTrapezoid(QIODevice& dataStream, int type)
{
//     Trapezoid trapezoid;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onTrapezoid: read error");
    }
    // 0WHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.geometryH
            & m_modalVariableSetStatus.m_d.geometryW)) {
        throw std::domain_error("onTrapezoid: required modal variables not set");
    }
//     trapezoid.m_layer = m_layer;
//     trapezoid.m_datatype = m_datatype;
//     trapezoid.m_width = m_geometryW;
//     trapezoid.m_height = m_geometryH;
    qint64 a = 0, b = 0;
    if (type == 23) {
        a = onSigned(dataStream);
        b = onSigned(dataStream);
    } else if (type == 24) {
        a = onSigned(dataStream);
    } else if (type == 25) {
        b = onSigned(dataStream);
    }
    // FIXME
    // construct points
//     trapezoid.m_isHorizontal = ((info >> 7) & 1);
//     if ((info >> 7) & 1) { // O
//         trapezoid.m_points << DeltaValue {0, qMax<qint64>(a, 0)};
//         trapezoid.m_points << DeltaValue {0, m_geometryH + qMin<qint64>(b, 0)};
//         trapezoid.m_points << DeltaValue {m_geometryW, m_geometryH - qMax<qint64>(b, 0)};
//         trapezoid.m_points << DeltaValue {m_geometryW, -qMin<qint64>(a, 0)};
//     } else {
//         trapezoid.m_points << DeltaValue {qMax<qint64>(a, 0), m_geometryH};
//         trapezoid.m_points << DeltaValue {m_geometryW + qMin<qint64>(b, 0), m_geometryH};
//         trapezoid.m_points << DeltaValue {m_geometryW - qMax<qint64>(b, 0), 0};
//         trapezoid.m_points << DeltaValue {-qMin<qint64>(a, 0), 0};
//     }
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     trapezoid.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     trapezoid.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
        //trapezoid.m_repetition = m_repetition;
    }
//     m_currentCell->m_trapezoids.append(trapezoid);
}

// FIXME
void oa::Parser::onCTrapezoid(QIODevice& dataStream)
{
//     CTrapezoid ctrapezoid;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onCTrapezoid: read error");
    }
    // TWHXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 7) & 1) { // T
        m_ctrapezoidType = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.ctrapezoidType = 1;
    }
    if ((info >> 6) & 1) { // W
        m_geometryW = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryW = 1;
    }
    if ((info >> 5) & 1) { // H
        m_geometryH = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.geometryH = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.ctrapezoidType)) {
        throw std::domain_error("onCTrapezoid: required modal variables not set");
    }
    switch (m_ctrapezoidType) {
    case 0 ... 15:
    case 20:
    case 21:
    case 24: {
        if (!m_modalVariableSetStatus.m_d.geometryH) {
            throw std::domain_error("onCTrapezoid: required modal variables not set");
        }
        break;
    }
    default:
        break;
    }
    switch (m_ctrapezoidType) {
    case 0 ... 19:
    case 22 ... 25: {
        if (!m_modalVariableSetStatus.m_d.geometryW) {
            throw std::domain_error("onCTrapezoid: required modal variables not set");
        }
        break;
    }
    default:
        break;
    }
//     ctrapezoid.m_layer = m_layer;
//     ctrapezoid.m_datatype = m_datatype;
//     ctrapezoid.m_type = m_ctrapezoidType;
//     ctrapezoid.m_width = m_geometryW;
//     ctrapezoid.m_height = m_geometryH;
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     ctrapezoid.m_x = m_geometryX;
    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     ctrapezoid.m_y = m_geometryY;
    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
//         ctrapezoid.m_repetition = m_repetition;
    }
//     m_currentCell->m_ctrapezoids.append(ctrapezoid);
}

// FIXME
void oa::Parser::onCircle(QIODevice& dataStream)
{
//     Circle circle;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onCircle: read error");
    }
    // 00rXYRDL
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }
    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if ((info >> 5) & 1) { // r
        m_circleRadius = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.circleRadius = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype
            & m_modalVariableSetStatus.m_d.circleRadius)) {
        throw std::domain_error("onCircle: required modal variables not set");
    }
//     circle.m_layer = m_layer;
//     circle.m_datatype = m_datatype;
//     circle.m_radius = m_circleRadius;
    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     circle.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     circle.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
//         circle.m_repetition = m_repetition;
    }
//     m_currentCell->m_circles.append(circle);
}

// TODO How to associate properties to elements
// If no m_currentCell set, then this property applies to the whole layout
void oa::Parser::onProperty(QIODevice& dataStream, int type)
{
    qInfo("onProperty");
    if (type == 29) {
        if (!m_modalVariableSetStatus.m_d.lastPropertyName) {
            throw std::domain_error("onProperty: type 29, but lastPropertyName is not set");
        }
        return; // m_lastPropertyName
    }
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onProperty: read error");
    }
    // UUUUVCNS
    if ((info >> 2) & 1) { // C
        if ((info >> 1) & 1) { // N
            quint64 i = onUnsigned(dataStream);
            if (m_layout.m_propNames.find(i) == m_layout.m_propNames.end()) {
                // throw std::domain_error("PropertyName not found");
                if (m_unresolvedPropName.find(i) == m_unresolvedPropName.end()) {
                    m_unresolvedPropName[i] = QUuid::createUuid().toString();
                }
                m_lastPropertyName = m_unresolvedPropName[i];
            } else {
                m_lastPropertyName = m_layout.m_propNames[i];
            }
        } else {
            m_lastPropertyName = onString(dataStream, N);
        }
        m_modalVariableSetStatus.m_d.lastPropertyName = 1;
    }

    if (!m_modalVariableSetStatus.m_d.lastPropertyName) {
        throw std::domain_error("onProperty: required variables not set");
    }

    quint32 valueCount = 0;
    if ((info >> 3) & 1) { // V
        if (info >> 4) {
            throw std::domain_error("onProperty: V=1 but UUUU != 0");
        }
        if (!m_modalVariableSetStatus.m_d.lastValueList) {
            throw std::domain_error("onProperty: lastValueList not set");
        }
    } else {
        quint8 uuuu = info >> 4;
        if (uuuu == 15) {
            valueCount = onUnsigned(dataStream);
        }
        if (uuuu < 15) {
            valueCount = uuuu;
        }
        m_lastValuesList.clear();
        for (auto i = 0; i < valueCount; ++i) {
            quint8 kind = 0;
            if (dataStream.peek((char *)&kind, 1) != 1) {
                throw std::domain_error("onProperty: read error");
            }
            switch (kind) {
            case 0 ... 7:
                m_lastValuesList << onReal(dataStream);
                break;
            case 8: {
                onUnsigned(dataStream);
                m_lastValuesList << onUnsigned(dataStream);
                break;
            }
            case 9: {
                onUnsigned(dataStream);
                m_lastValuesList << onSigned(dataStream);
                break;
            }
            case 10: {
                onUnsigned(dataStream);
                m_lastValuesList << onString(dataStream, A);
                break;
            }
            case 11: {
                onUnsigned(dataStream);
                m_lastValuesList << onString(dataStream, B);
                break;
            }
            case 12: {
                onUnsigned(dataStream);
                m_lastValuesList << onString(dataStream, N);
                break;
            }
            case 13 ... 15: {
                onUnsigned(dataStream);
                m_lastValuesList << onUnsigned(dataStream);
                break;
            }
            default:
                // parse error
                throw std::domain_error("onProperty: invalid lastValueList kind");
                break;
            }

        }
        m_modalVariableSetStatus.m_d.lastValueList = 1;
    }
}

void oa::Parser::onXName(QIODevice& dataStream, int type)
{
    qInfo("onXName");
    if (m_xNameMode == Default) {
        if (type == 30) {
            m_xNameMode = Implicit;
        } else {
            m_xNameMode = Explicit;
        }
    }
    if ((m_xNameMode == Implicit && type == 31) || (m_xNameMode == Explicit && type == 30)) {
        throw std::domain_error("Both Implicit and Explicit XName!");
    }

    onUnsigned(dataStream);
    onString(dataStream, N);
    if (m_xNameMode == Explicit) {
        onUnsigned(dataStream);
    }
    /*
    XName xname;
    xname.m_attribute = onUnsigned(dataStream);
    xname.m_string = onString(dataStream, N); // ABN
    quint32 reference;
    if (m_xNameMode == Explicit) {
        reference = onUnsigned(dataStream);
    } else {
        reference = m_xNameReference++;
    }
    m_layout.m_xNames[reference] = xname;
    */
}

void oa::Parser::onXElement(QIODevice& dataStream)
{
    qInfo("onXElement");
    // Assert state Element
    onUnsigned(dataStream);
    onString(dataStream, N);
    /**
    XELement xelement;
    xelement.m_attribute = onUnsigned(dataStream);
    xelement.m_string = onString(dataStream, B);
    m_currentCell->m_xelements.push_back(xelement);
    **/
}

// FIXME one ut case
void oa::Parser::onXGeometry(QIODevice& dataStream)
{
    qInfo("onXGeometry");
//     XGeometry xgeometry;
    quint8 info = 0;
    if (dataStream.read((char *) &info, 1) != 1) {
        throw std::domain_error("onXGeometry: read error");
    }
    // 000XYRDL
//     xgeometry.m_attribute =
    onSigned(dataStream);
    if (info & 1) { // L
        m_layer = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.layer = 1;
    }

    if ((info >> 1) & 1) { // D
        m_datatype = onUnsigned(dataStream);
        m_modalVariableSetStatus.m_d.datatype = 1;
    }
    if (!(m_modalVariableSetStatus.m_d.layer
            & m_modalVariableSetStatus.m_d.datatype)) {
        throw std::domain_error("onXGeometry: required modal variables not set");
    }
//     xgeometry.m_layer = m_layer;
//     xgeometry.m_datatype = m_datatype;
//     xgeometry.m_string = onString(dataStream, B);
    onString(dataStream, B);

    if ((info >> 4) & 1) { // X
        qint64 x = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryX += x;
        } else {
            m_geometryX = x;
        }
    }
//     xgeometry.m_x = m_geometryX;

    if ((info >> 3) & 1) { // Y
        qint64 y = onSigned(dataStream);
        if (m_isXYRelative) {
            m_geometryY += y;
        } else {
            m_geometryY = y;
        }
    }
//     xgeometry.m_y = m_geometryY;

    if ((info >> 2) & 1) { // R
        onRepetition(dataStream);
//         xgeometry.m_repetition = m_repetition;
    }
//     m_currentCell->m_xgeometries.append(xgeometry);
}

void oa::Parser::onCBlock(QIODevice& dataStream)
{
    qInfo("onCBlock");
    quint64 comType = onUnsigned(dataStream);
    quint64 uncomByteCount = onUnsigned(dataStream);
    quint64 comByteCount = onUnsigned(dataStream);
    QVarLengthArray<char, 1024> cblockData(comByteCount);
    if (dataStream.read(cblockData.data(), comByteCount) != comByteCount) {
        throw std::domain_error("onCBlock: read error");
    }
//  switch (comType) {
//      case 0:
//          deflate
//      break;
//      default::
//        throw std::domain_error("onCBLOCK: unsupport compress method");
//  }
//     QBuffer buffer;
//     // uncblockData = ucompress cblockData
//     buffer.setData(uncblockData.data(), comByteCount);
//     buffer.open(QIODevice::WriteOnly);
//      while (!buffer.atEnd()) {
//         quint8 type = 0;
//         if (buffer.peek((char *)&type, sizeof(type)) != sizeof(type)) {
//             throw std::domain_error("fail to peek type");
//         }
//         switch (type) {
//         case START:
//         case END:
//         case CELL:
//         case CBLOCK:
//             throw std::domain_error("onCBLOCK: START END CELL CBLOCK");
//          default:
//             nextRecord(buffer);
//             break;
//         }
//     }
}

quint64 oa::Parser::onUnsigned(QIODevice& dataStream)
{
    quint64 v = 0;
    quint64 p = 1;
    uchar c = 0;
    static quint64 threshold = std::numeric_limits <quint64>::max() / 128;
    do {
        if (dataStream.atEnd()) {
            /*abort();*/
            throw std::domain_error("onUnsigned: Fail atEnd");
        }
        if (dataStream.read((char *)&c, 1) != 1) {
            throw std::domain_error("onUnsigned: fail to read ");
        }
        if (p > threshold && (quint64)(c & 0x7F) > (std::numeric_limits <quint64>::max() / p)) {
            throw std::domain_error("onUnsigned: overflow");
        }
        v += static_cast<quint64>(c & 0x7F) * p;
        p <<= 7;
    } while (c & 0x80);
    return v;
}

qint64 oa::Parser::onSigned(QIODevice& dataStream)
{
    quint64 us = onUnsigned(dataStream);
    if (us & 1) {
        return - static_cast<qint64>(us >> 1);
    }
    return static_cast<qint64>(us >> 1);
}

double oa::Parser::onReal(QIODevice& dataStream)
{
    quint64 k = onUnsigned(dataStream);
    switch (k) {
    case 0:
        return onUnsigned(dataStream);
    case 1: {
        return - double(onUnsigned(dataStream));
    }
    case 2:
        return 1.0 / ((double) onUnsigned(dataStream));
    case 3:
        return - 1.0 / ((double) onUnsigned(dataStream));
    case 4:
        return ((double) onUnsigned(dataStream)) / ((double) onUnsigned(dataStream));
    case 5:
        return - ((double) onUnsigned(dataStream)) / ((double) onUnsigned(dataStream));
    case 6: {
        float v;
        if (dataStream.read((char *) &v, sizeof(float)) != sizeof(float)) {

            throw std::domain_error("onReal: fail to read float");
        }
        return v;
    }
    case 7: {
        // FIXME byteorder?
        double v;
        if (dataStream.read((char *) &v, sizeof(double)) != sizeof(double)) {
            throw std::domain_error("onReal: fail to read double");
        }
        return v;
    }
    default:
        throw std::domain_error("onReal: fail to read real");
    }
}


oa::Delta23 oa::Parser::onDelta2(QIODevice& dataStream)
{
    return Delta23(onUnsigned(dataStream), true);
}

oa::Delta23 oa::Parser::onDelta3(QIODevice& dataStream)
{
    return Delta23(onUnsigned(dataStream), false);
}


oa::DeltaG oa::Parser::onDeltaG(QIODevice& dataStream)
{
    quint64 v = onUnsigned(dataStream);
    if (v & 1) {
        qint64 x = (v & 2) ? - static_cast<qint64>(v >> 2) : static_cast<qint64>(v >> 2);
        return DeltaG(x, onSigned(dataStream));
    }
    return DeltaG(v);
}

// t2.6 type validation
QString oa::Parser::onString(QIODevice& dataStream, StringType type)
{
    quint32 len = onUnsigned(dataStream);
    if (len > 1024) {
        qWarning("A long string");
    }
    QVarLengthArray<char, 1024> s(len);
    if (dataStream.read(s.data(), len) != len) {
//         abort();
        throw std::domain_error("onString: read error");
    }
    QString string = QString::fromLatin1(s.constData(), len);
    if (type == N) {
        if (len == 0) {
            throw std::domain_error("invalid n-string");
        }
        for (auto i : string) {
            if (i < 0x21 || i > 0x7E) {
                throw std::domain_error("invalid n-string");
            }
        }
    }
    if (type == A) {
        // Printable
        for (auto i : string) {
            if (i < 0x20 || i > 0x7E) {
                throw std::domain_error("invalid a-string");
            }
        }
    }
    return string;
}

void oa::Parser::onRepetition(QIODevice& dataStream)
{
    quint8 type = onUnsigned(dataStream);
    if (type && type < 12) {
        m_modalVariableSetStatus.m_d.repetition = 1;
    }
    switch (type) {
    case 0: {
        if (!m_modalVariableSetStatus.m_d.repetition) {
            throw std::domain_error("invalid Repetition");
        }
        break;
    }
    case 1: {
        qInfo("onRepetition1");
        int dx = onUnsigned(dataStream);
        int dy = onUnsigned(dataStream);
        int sx = onUnsigned(dataStream);
        int sy = onUnsigned(dataStream);
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = (dx + 2) * (dy + 2);
        for (int i = 0; i < dx + 2; ++i) {
            for (int j = 0; j < dy + 2; ++j) {
                m_layout.m_repetitions.append(oa::DeltaValue(sx * i, sy * j));
            }
        }
        break;
    }
    case 2: {
        qInfo("onRepetition2");
        int dx = onUnsigned(dataStream);
        int sx = onUnsigned(dataStream);
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = dx + 2;
        for (int i = 0; i < dx + 2; ++i) {
            m_layout.m_repetitions.append(oa::DeltaValue(sx * i, 0));
        }

        break;
    }
    case 3: {
        qInfo("onRepetition3");
        int dy = onUnsigned(dataStream);
        int sy = onUnsigned(dataStream);
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = dy + 2;
        for (int i = 0; i < dy + 2; ++i) {
            m_layout.m_repetitions.append(oa::DeltaValue(0, sy * i));
        }
        break;
    }
    case 4: {
        qInfo("onRepetition4");
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        quint64 dx = onUnsigned(dataStream);
        m_repetitionCount = dx + 2;
        quint32 s = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(s, 0));
        for (quint64 i = 0; i < dx + 1; ++i) {
            s += onUnsigned(dataStream);
            m_layout.m_repetitions.append(oa::DeltaValue(s, 0));
        }
        break;
    }
    case 5: {
        qInfo("onRepetition5");
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        quint64 dx = onUnsigned(dataStream); // n - 2
        quint64 g = onUnsigned(dataStream);
        m_repetitionCount = dx + 2;
        quint32 s = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(s, 0));
        for (quint64 i = 0; i < dx + 1; ++i) {
            s += onUnsigned(dataStream);
            m_layout.m_repetitions.append(oa::DeltaValue(s * g, 0));
        }
        break;
    }
    case 6: {
        qInfo("onRepetition6");
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        quint64 dy = onUnsigned(dataStream);
        m_repetitionCount = dy + 2;
        quint32 s = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(0, s));
        for (quint64 i = 0; i < dy + 1; ++i) {
            s += onUnsigned(dataStream);
            m_layout.m_repetitions.append(oa::DeltaValue(0, s));
        }
        break;
    }
    case 7: {
        qInfo("onRepetition7");
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        quint64 dy = onUnsigned(dataStream); // n - 2
        quint64 g = onUnsigned(dataStream);
        m_repetitionCount = dy + 2;
        quint32 s = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(0, s));
        for (quint64 i = 0; i < dy + 1; ++i) {
            s += onUnsigned(dataStream);
            m_layout.m_repetitions.append(oa::DeltaValue(0, s * g));
        }
        break;
    }
    case 8: {
        qInfo("onRepetition8");
        quint64 dn = onUnsigned(dataStream);
        quint64 dm = onUnsigned(dataStream);
        oa::DeltaValue pn = onDeltaG(dataStream).value;
        oa::DeltaValue pm = onDeltaG(dataStream).value;
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = (dn + 2) * (dm + 2);
        for (quint64 i = 0; i < dn + 2; ++i) {
            for (quint64 j = 0; j < dm + 2; ++j) {
                m_layout.m_repetitions.append(oa::DeltaValue(i * pn.m_x + j * pm.m_x, i * pn.m_y + j * pm.m_y));
            }
        }
        break;
    }
    case 9: {
        qInfo("onRepetition9");
        quint64 d = onUnsigned(dataStream);
        oa::DeltaValue p = onDeltaG(dataStream).value;
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = d + 2;
        for (quint64 i = 0; i < d + 2; ++i) {
            m_layout.m_repetitions.append(oa::DeltaValue(i * p.m_x, i * p.m_y));
        }
        break;
    }
    case 10: {
        qInfo("onRepetition10");
        quint64 d = onUnsigned(dataStream); // p - 2
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = d + 2;
        qint32 sx = 0, sy = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(0, 0));
        for (quint64 i = 0; i < d + 1; ++i) {
            oa::DeltaValue p = onDeltaG(dataStream).value;
            sx += p.m_x;
            sy += p.m_y;
            m_layout.m_repetitions.append(oa::DeltaValue(sx, sy));
        }
        break;
    }
    case 11: {
        qInfo("onRepetition11");
        quint64 d = onUnsigned(dataStream); // p - 2
        quint64 g = onUnsigned(dataStream); // grid;
        m_repetitionOffset = m_layout.m_repetitions.size();
        m_modalVariableSetStatus.m_d.repetition = 1;
        m_repetitionCount = d + 2;
        qint32 sx = 0, sy = 0;
        m_layout.m_repetitions.append(oa::DeltaValue(0, 0));
        for (quint64 i = 0; i < d + 1; ++i) {
            oa::DeltaValue p = onDeltaG(dataStream).value;
            sx += p.m_x;
            sy += p.m_y;
            m_layout.m_repetitions.append(oa::DeltaValue(sx * g, sy * g));
        }
        break;
    }
    default:
        throw std::domain_error("invalid Repetition");
        break;
    }
}

void oa::Parser::onPointList(QIODevice& dataStream,  bool isPolygon)
{
    quint8 type = onUnsigned(dataStream);
    quint64 count = onUnsigned(dataStream);

    PointList pl;
    switch (type) {
    case 0:
    case 1: {
        bool h = type == 0;
        DeltaValue v = {0, 0};
        for (quint64 i = 0; i < count; ++i) {
            // 1-delta
            qint64 d = onSigned(dataStream);
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
                throw std::domain_error("illegal PointList");
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
        for (quint64 i = 0; i < count; ++i) {
            v += onDelta2(dataStream).value;
            pl << v;
        }
        break;
    }
    case 3: {
        DeltaValue v = {0, 0};
        for (quint64 i = 0; i < count; ++i) {
            v += onDelta3(dataStream).value;
            pl << v;
        }
        break;
    }
    case 4: {
        DeltaValue v = {0, 0};
        for (quint64 i = 0; i < count; ++i) {
            v += onDeltaG(dataStream).value;
            pl << v;
        }
        break;
    }
    case 5: {
        DeltaValue v = {0, 0};
        DeltaValue d = {0, 0};
        for (quint64 i = 0; i < count; ++i) {
            d += onDeltaG(dataStream).value;
            v += d;
            pl << v;
        }
        break;
    }
    default:
        throw std::domain_error("invalid PointList");
        break;
    }
	if (pl.isEmpty())
	{
        throw std::domain_error("invalid PointList");
	}
    if (isPolygon) {
        m_polygonPointListOffset = m_layout.m_vertexes.size();
        m_polygonPointListCount = pl.size();
        m_layout.m_vertexes.append(pl);
        m_modalVariableSetStatus.m_d.polygonPointList = 1;
    } else {
        m_pointListOffset = m_layout.m_vertexes.size();
        m_pointListCount = pl.size();
        m_layout.m_vertexes.append(pl);
        m_modalVariableSetStatus.m_d.pathPointList = 1;
    }
}

oa::IntervalType oa::Parser::onInterval(QIODevice& dataStream)
{
    quint8 type = onUnsigned(dataStream);
    switch (type) {
    case 0:
        return IntervalType {0,  1U << 31};
    case 1:
        return IntervalType {0, onUnsigned(dataStream)};
    case 2:
        return IntervalType {onUnsigned(dataStream), 1U << 31};
    case 3: {
        quint64 b = onUnsigned(dataStream);
        return IntervalType {b, b};
    }
    case 4: {
        return IntervalType {onUnsigned(dataStream), onUnsigned(dataStream)};
    }
    default:
        throw std::domain_error("invalid Interval");
        break;
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
//     m_repetition.clear();
//     m_polygonPointList.clear();
//     m_pointList.clear();
    m_lastValuesList.clear();
	m_layout.m_repetitions.clear();
	m_layout.m_repetitions.push_back({0,0});
    m_repetitionOffset = 0;
    m_repetitionCount = 1;
    m_polygonPointListOffset = -1;
    m_polygonPointListCount = -1;
    m_pointListOffset = -1;
    m_pointListCount = -1;
}

