/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2016  颜烈彬 <slbyan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
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

#include "parsetest.h"

#include <QtTest>
#include "../src/parser.h"

QTEST_MAIN(ParseTest);

void ParseTest::initTestCase()
{
    // Called before the first testfunction is executed
}

void ParseTest::cleanupTestCase()
{
    // Called after the last testfunction was executed
}

void ParseTest::init()
{
    // Called before each testfunction is executed
}

void ParseTest::cleanup()
{
    // Called after every testfunction
}

void ParseTest::parseTest()
{
    QString oasisDataDir = "/home/yanlb/projects/klayout-r2864/testdata/oasis/";
    oa::Layout layout;
    oa::Parser parser(layout);
    QFETCH(QString, file);
    QFETCH(bool, expected);
    QCOMPARE( parser.open(oasisDataDir + file), expected);
}

void ParseTest::parseTest_data() {
    static const char* oasisFiles [54] = {
        "t10.1.oas",  "t1.1.oas",   "t1.3.oas",   "t2.4.oas",   "t3.2.oas",  "t3.9.oas",  "t7.1.oas",  "t8.7.oas",
        "t11.1.oas",  "t12.1.oas",  "t14.1.oas",  "t2.5.oas",   "t3.3.oas",  "t4.1.oas",  "t8.1.oas",  "t8.8.oas",
        "t11.2.oas",  "t1.2.oas",   "t1.4.oas",   "t2.6.oas",   "t3.4.oas",  "t4.2.oas",  "t8.2.oas",  "t9.1.oas",
        "t11.3.oas",  "t13.1.oas",  "t1.5.oas",   "t3.10.oas",  "t3.5.oas",  "t5.1.oas",  "t8.3.oas",  "t9.2.oas",
        "t11.4.oas",  "t13.2.oas",  "t2.1.oas",   "t3.11.oas",  "t3.6.oas",  "t5.2.oas",  "t8.4.oas",  "xgeometry_test.oas",
        "t11.5.oas",  "t13.3.oas",  "t2.2.oas",   "t3.12.oas",  "t3.7.oas",  "t5.3.oas",  "t8.5.oas",
        "t11.6.oas",  "t13.4.oas",  "t2.3.oas",   "t3.1.oas",   "t3.8.oas",  "t6.1.oas",  "t8.6.oas"
    };
    static const bool result [54] = {
        true,       true,           true,           true,           true,       true,       true,       true,
        true,       true,           true,           false,          false,      true,       true,       true,
        true,       true,           true,           false,          false,      true,       true,       true,
        true,       true,           true,           true,           true,       true,       true,       true,
        true,       true,           true,           false,          false,      true,       true,       true,
        true,       true,           true,           true,           false,      true,       true,
        true,       true,           false,          true,           false,      true,       true
    };
    QTest::addColumn<QString>("file");
    QTest::addColumn<bool>("expected");
    for (int i = 0; i < 54; ++i) {
        QTest::newRow(oasisFiles[i]) << oasisFiles[i] << result[i];
    }

}

void ParseTest::parseTest2()
{
    QString oasisDataDir = "/home/yanlb/projects/klayout-r2864/testdata/bool/";
    oa::Layout layout;
    oa::Parser parser(layout);
    QFETCH(QString, file);
    QCOMPARE( parser.open(oasisDataDir + file), true);
}

void ParseTest::parseTest2_data() {
    static const char* oasisFiles [] = {
"and1.oas",    "anotb3.oas",  "bnota5.oas",  "size1_au.oas",   "size4_au2.oas",   "size5_au4.oas",  "size7_au2.oas",  "size8_au3.oas",     "special2_au5.oas",  "xor1_max.oas",  "xor5_max.oas",      "xor7_max.oas",
"and2.oas",    "anotb4.oas",  "bnota6.oas",  "size1.oas",      "size4.oas",       "size5_au5.oas",  "size7_au3.oas",  "size8_au4.oas",     "special2.oas",      "xor1.oas",      "xor5.oas",          "xor7.oas",
"and3.oas",    "anotb5.oas",  "or1.oas",     "size2_au.oas",   "size5_au10.oas",  "size5_au6.oas",  "size7_au4.oas",  "size8.oas",         "special3_au1.oas",  "xor2_max.oas",  "xor6_max.oas",      "xor8_au1.oas",
"and4.oas",    "anotb6.oas",  "or2.oas",     "size2.oas",      "size5_au11.oas",  "size5.oas",      "size7_au5.oas",  "special1.oas",      "special3_au2.oas",  "xor2.oas",      "xor6.oas",          "xor8_au2.oas",
"and5.oas",    "bnota1.oas",  "or3.oas",     "size3_au1.oas",  "size5_au12.oas",  "size6_au1.oas",  "size7_au6.oas",  "special2_au1.oas",  "special3_au3.oas",  "xor3_max.oas",  "xor7_au1.oas",      "xor8.oas",
"and6.oas",    "bnota2.oas",  "or4.oas",     "size3_au2.oas",  "size5_au1.oas",   "size6_au2.oas",  "size7.oas",      "special2_au2.oas",  "special3_au4.oas",  "xor3.oas",      "xor7_au2.oas",
"anotb1.oas",  "bnota3.oas",  "or5.oas",     "size3.oas",      "size5_au2.oas",   "size6.oas",      "size8_au1.oas",  "special2_au3.oas",  "special3_au5.oas",  "xor4_max.oas",  "xor7_max_au1.oas",
"anotb2.oas",  "bnota4.oas",  "or6.oas",     "size4_au1.oas",  "size5_au3.oas",   "size7_au1.oas",  "size8_au2.oas",  "special2_au4.oas",  "special3.oas",      "xor4.oas",      "xor7_max_au2.oas" };

    QTest::addColumn<QString>("file");
    for (int i = 0; i < 92; ++i) {
        QTest::newRow(oasisFiles[i]) << oasisFiles[i];
    }

}

#include "parsetest.moc"
