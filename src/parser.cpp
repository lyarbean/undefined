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

oa::Parser::Parser() : m_layout(0)
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
    while (!m_dataStream.atEnd() && nextRecord()) ;
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

bool oa::Parser::onPad()
{
    return true;
}

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
        if (type == 3 ) {
            m_cellNameMode = Implicit;
        } else {
            m_cellNameMode = Explicit;
        }
    }
    if ((m_cellNameMode == Implicit && type == 4) ||
            (m_cellNameMode == Explicit && type == 3))
    {
        qFatal("Both Implicit and Explicit CellName!");
    }
    quint32 reference;
    if (m_cellNameMode == Explicit) {
        reference = onUnsigned();
    } else {
        reference = m_cellNameReference ++;
    }
    // TODO check for reference and name
    m_layout -> m_cellNames.insert(reference, name);
    return true;
}

bool oa::Parser::onTextString(int type)
{
    QString name = onString(N);
    if (m_textStringMode == Default) {
        if (type == 5) {
            m_textStringMode = Implicit;
        } else {
            m_textStringMode = Explicit;
        }
    }
    if ((m_textStringMode == Implicit && type == 6) ||
            (m_textStringMode == Explicit && type == 5))
    {
        qFatal("Both Implicit and Explicit textString!");
    }
    quint32 reference;
    if (m_textStringMode == Explicit) {
        reference = onUnsigned();
    } else {
        reference = m_textStringReference ++;
    }
    // TODO check for reference and name
    m_layout -> m_textStrings.insert(reference, name);
    return true;
}

bool oa::Parser::onPropName(int type)
{
    QString name = onString(N);
    if (m_propNameMode == Default) {
        if (type == 7 ) {
            m_propNameMode = Implicit;
        } else {
            m_propNameMode = Explicit;
        }
    }
    if ((m_propNameMode == Implicit && type == 8) ||
            (m_propNameMode == Explicit && type == 7))
    {
        qFatal("Both Implicit and Explicit PropName!");
    }
    quint32 reference;
    if (m_propNameMode == Explicit) {
        reference = onUnsigned();
    } else {
        reference = m_propNameReference ++;
    }
    // TODO check for reference and name
    m_layout -> m_PropNames.insert(reference, name);
    return true;
}

bool oa::Parser::onPropString(int type)
{
    QString name = onString(N);
    if (m_propStringMode == Default) {
        if (type == 9 ) {
            m_propStringMode = Implicit;
        } else {
            m_propStringMode = Explicit;
        }
    }
    if ((m_propStringMode == Implicit && type == 10) ||
            (m_propStringMode == Explicit && type == 9))
    {
        qFatal("Both Implicit and Explicit PropString!");
    }
    quint32 reference;
    if (m_propStringMode == Explicit) {
        reference = onUnsigned();
    } else {
        reference = m_propStringReference ++;
    }
    // TODO check for reference and name
    m_layout->m_PropStrings.insert(reference, name);
    return true;
}

bool oa::Parser::onLayerName(int type)
{
    QString name = onString(N);
    IntervalType boundA, boundB;
    boundA = onInterval();
    boundB = onInterval();
    // TODO How to store?
    return true;
}
//TODO 
bool oa::Parser::onCell(int type)
{
    QSharedPointer<Cell> cell(new Cell);
    m_layout->m_cells.push_back(cell);
    if (type == 13) {
        quint32 reference = onUnsigned();
        // FIXME if cellname for reference is absent
        cell->m_name = m_layout->m_cellNames[reference];
    } else {
        cell->m_name = onString(N);
    }
    m_currentCell = cell;
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
    if (info >> 8) { // C
        if (info & 0b01000000) { // N
            placement.m_referenceNumber = onUnsigned();
        } else {
            placement.m_cellnameString = onString(N);
        }
    } else {
        // use modal placement cell
        placement.m_cellnameString = m_placement.m_cellnameString;
    }
    if (type == 18) {
        if (info & 0b100) { // M
            placement.m_manification = onReal();
        } else {
            placement.m_manification = 1.0;
        }
        if (info & 0b10) { // !
            placement.m_angle = onReal();
        } else {
            placement.m_angle = 0.0;
        }
    }
    if (info & 0b00100000) { // X
        placement.m_x = onSigned();
        if (m_isXYRelative) {
            placement.m_x += m_placement.m_x;
        }
    } else {
        placement.m_x = m_placement.m_x;
    }
    if (info & 0b00010000) { // Y
        placement.m_y = onSigned();
        if (m_isXYRelative) {
            placement.m_x += m_placement.m_y;
        }
    } else {
        placement.m_y = m_placement.m_y;
    }
    if (info & 0b1000) { // R
        placement.m_repetition = onRepetition();
    }
    if (info & 0b1) { // F
        placement.m_y = -placement.m_y; // ??
    }
    m_placement = placement;
    return true;
}

bool oa::Parser::onText()
{
    return true;
}

