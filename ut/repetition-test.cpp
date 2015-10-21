#include <QTest>
#include <QDebug>
#include <QSharedDataPointer>
// #include "../src/repetition.h"
#include "../src/parser.h"
class RepetitionTest: public QObject
{
    Q_OBJECT
private slots:
    void creation();
//     void createTest();
};


void RepetitionTest::creation()
{
    using namespace oa;
    using Rep = QSharedPointer< Repetition>;
    QVector<QSharedPointer< Repetition>> repetitions;
    repetitions.append(Rep {new Repetition1()});
    repetitions.append(Rep {new Repetition2()});
    repetitions.append(Rep {new Repetition3()});
    repetitions.append(Rep {new Repetition4()});
    repetitions.append(Rep {new Repetition5()});
    repetitions.append(Rep {new Repetition6()});
    repetitions.append(Rep {new Repetition7()});
    repetitions.append(Rep {new Repetition8()});
    repetitions.append(Rep {new Repetition9()});
    repetitions.append(Rep {new Repetition10()});
    repetitions.append(Rep {new Repetition11()});

    for (auto r : repetitions) {
        qDebug() << r->type();
    }
    qDebug() << "Repetition";
    qDebug() << sizeof(Repetition);
    qDebug() << sizeof(Repetition1);
    qDebug() << sizeof(Repetition2);
    qDebug() << sizeof(Repetition3);
    qDebug() << sizeof(Repetition4);
    qDebug() << sizeof(Repetition5);
    qDebug() << sizeof(Repetition6);
    qDebug() << sizeof(Repetition7);
    qDebug() << sizeof(Repetition8);
    qDebug() << sizeof(Repetition9);
    qDebug() << sizeof(Repetition10);
    qDebug() << sizeof(Repetition11);
    qDebug() << "Delta";
    qDebug() << sizeof(Delta1);
    qDebug() << sizeof(Delta23);
    qDebug() << sizeof(DeltaG);
    qDebug() << "PointList";
    qDebug() << sizeof(PointList);
    qDebug() << sizeof(PointList0);
    qDebug() << sizeof(PointList1);
    qDebug() << sizeof(PointList23);
    qDebug() << sizeof(PointList4);
    qDebug() << sizeof(PointList5);
    qDebug() << "Polygon";
    qDebug() << sizeof(Polygon);
    qDebug() << sizeof(Path);
        qDebug() << sizeof(Parser);
}


QTEST_MAIN(RepetitionTest)
#include "repetition-test.moc"
