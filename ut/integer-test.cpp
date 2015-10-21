#include <QTest>
#include "../src/dataconstructs.h"
class IntegerTest: public QObject
{
    Q_OBJECT
private slots:
    void decodeTest_data();
    void decodeTest();
//     void createTest();
};

void IntegerTest::decodeTest_data()
{
    QTest::addColumn<QByteArray>("raw");
    QTest::addColumn<quint64>("result");
    QByteArray a;
    a += char(0b00000000);
    QTest::newRow("0") << a << quint64(0);
    a.clear();
    a += char(0b01111111);
    QTest::newRow("127")  << a << quint64(127);
    a.clear();
    a += char(0b10000000);
    a += char(0b00000001);
    QTest::newRow("128")  << a << quint64(128);
    a.clear();
    a += char(0b11111111);
    a += char(0b01111111);
    QTest::newRow("16383") << a << quint64(16383);
    a.clear();
    a += char(0b10000000);
    a += char(0b10000000);
    a += char(0b00000001);
    QTest::newRow("16384") << a << quint64(16384);
}

void IntegerTest::decodeTest()
{
    QFETCH(QByteArray, raw);
    QFETCH(quint64, result);
    QCOMPARE(oa::UnsignedInteger::decode(raw), result);
}



QTEST_MAIN(IntegerTest)
#include "integer-test.moc"
