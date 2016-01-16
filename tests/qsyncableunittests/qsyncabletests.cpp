#include <QVariantList>
#include <QTest>
#include "qsdiffrunner.h"
#include "qsyncabletests.h"
#include "qsvariantlistmodel.h"

QSyncableTests::QSyncableTests(QObject *parent) : QObject(parent)
{

}

void QSyncableTests::change()
{
    QSChange c1,c2;

    c1.setType(QSChange::Insert);
    c2.setType(QSChange::Move);

    QCOMPARE(c1 == c2, false);

    c1.setType(QSChange::Move);

    QCOMPARE(c1 == c2, true);

    QVariantMap d1;
    d1["key"] = "1";

    c1.setData(d1);

    QCOMPARE(c1 == c2, false);
}

void QSyncableTests::changeMerge()
{
    QSChange c1,c2,c3,c4;

    c1 = QSChange(QSChange::Move);
    c2 = QSChange(QSChange::Insert);

    QVERIFY(!c1.canMerge(c2));

    c1 = QSChange(QSChange::Remove,0,0);
    c2 = QSChange(QSChange::Remove,1,1);

    QVERIFY(c1.canMerge(c2));
    QVERIFY(c2.canMerge(c1));

    c3 = c1.merge(c2);
    c4 = c2.merge(c1);

    QCOMPARE(c3.type(), QSChange::Remove);
    QCOMPARE(c3.from(), 0);
    QCOMPARE(c3.to(), 1);

    QVERIFY(c3 == c4);

    c2 = QSChange(QSChange::Remove,2,2);

    QVERIFY(!c1.canMerge(c2));
    QVERIFY(!c2.canMerge(c1));

    /* Merge move */

    c1 = QSChange(QSChange::Move, 1, 0, 1);
    c2 = QSChange(QSChange::Move, 2, 1, 1);
    QVERIFY(c1.canMerge(c2));
    QVERIFY(!c2.canMerge(c1));

    c3 = c1.merge(c2);
    c4 = c2.merge(c1);

    QCOMPARE(c3.type(), QSChange::Move);
    QCOMPARE(c3.from(), 1);
    QCOMPARE(c3.to(), 0);
    QCOMPARE(c3.count(), 2);

    QCOMPARE(c4.type(), QSChange::Null);

}

void QSyncableTests::diffRunner()
{
    QFETCH(QVariantList, previous);
    QFETCH(QVariantList, current);
    QFETCH(QString, keyField);
    QFETCH(QList<QSChange>, changes);

    QList<QSChange> expectedChanges = changes;

    QSDiffRunner runner;
    runner.setKeyField(keyField);

    QList<QSChange> result = runner.compare(previous, current);

    if (changes.size() != result.size()) {
        for (int i = 0 ; i < result.size() ; i++) {
            qDebug() << result.at(i);
        }
    }

    QCOMPARE(expectedChanges.size(), result.size());

    for (int i = 0; i < changes.size(); i++) {
        QSChange expected = changes.at(i);
        QSChange real = result.at(i);
        if (!(expected == real))  {
            qDebug() << expected;
            qDebug() << real;
        }
        QVERIFY(expected == real);
    }

}

void QSyncableTests::diffRunner_data()
{
    QTest::addColumn<QVariantList>("previous");
    QTest::addColumn<QVariantList>("current");
    QTest::addColumn<QString>("keyField");
    QTest::addColumn<QList<QSChange> >("changes");

    QVariantList previous;
    QVariantList current;
    QVariantList keyField;
    QList<QSChange> changes;
    QSChange c1;

    QVariantMap a,b,c,d;
    a["id"] = "a";
    b["id"] = "b";
    c["id"] = "c";
    d["id"] = "d";

    previous << a << b << c;
    current << a << b << c;

    QTest::newRow("No Changes") << previous << current << "id" << changes;

    /* Remove first element */
    previous.clear();current.clear();changes.clear();
    c1 = QSChange();
    previous << a << b << c;
    current << b << c;
    c1.setType(QSChange::Remove);
    c1.setFrom(0);
    c1.setTo(0);
    changes << c1;
    QTest::newRow("Remove first element") << previous << current << "id" << changes;

    /* Remove all element */
    previous.clear();current.clear();changes.clear();
    c1 = QSChange();
    previous << a << b << c;
    c1.setType(QSChange::Remove);
    c1.setFrom(0);
    c1.setTo(2);
    changes << c1;
    QTest::newRow("Remove all element") << previous << current << "id" << changes;

    /* Add new element to end */

    previous.clear();current.clear();changes.clear();
    previous << a << b << c;
    current << a << b << c << d;
    changes << QSChange(QSChange::Insert, 3, 3, 1, d);
    QTest::newRow("Add new element to end") << previous << current << "id" << changes;

    /* Move from last to first */
    previous.clear();
    previous << a << b << c;
    current.clear();
    current << c << a << b;
    changes.clear();
    changes << QSChange(QSChange::Move,2,0,1);

    QTest::newRow("Move from last to first") << previous << current << "id" << changes;

    /* Move 2 from first to last */
    previous.clear();
    previous << a << b << c;
    current.clear();
    current << c << a << b;
    changes.clear();
    changes << QSChange(QSChange::Move,2,0,1);

    QTest::newRow("Move 2 from first to last") << previous << current << "id" << changes;


    /* Move 2 from last to first */
    previous.clear();
    previous << a << b << c;
    current.clear();
    current << b << c << a;
    changes.clear();
    changes << QSChange(QSChange::Move,1,0,2);

    QTest::newRow("Move 2 from last to first") << previous << current << "id" << changes;
}

