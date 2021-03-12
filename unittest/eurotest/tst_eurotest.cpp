/*
   Copyright (C) 2019 Arto Hyvättinen

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <QtTest>

#include "model/euro.h"

class EuroTest : public QObject
{
    Q_OBJECT

public:
    EuroTest();
    ~EuroTest();

private slots:
    void string_to_euro();
    void qlonglong_to_string();
    void euro_to_qstring();
    void euro_eq_qstring();
    void qvariant_to_euro();
};

EuroTest::EuroTest()
{

}

EuroTest::~EuroTest()
{

}

void EuroTest::string_to_euro()
{
    Euro euro1("123.45");
    QCOMPARE(euro1.cents(), 12345);

    Euro euro2("123.4");
    QCOMPARE(euro2.cents(), 12340);

    Euro euro3("-123.45");
    QCOMPARE(euro3.cents(), -12345);

    Euro euro4("-123.5");
    QCOMPARE(euro4.cents(), -12350);

    Euro euro5("123");
    QCOMPARE(euro5.cents(), 12300);
}

void EuroTest::qlonglong_to_string()
{
    Euro euro1(12345);
    QCOMPARE(euro1.toString(),"123.45");

    Euro euro2(-12304);
    QCOMPARE(euro2.toString(), "-123.04");

    Euro euro3(10000);
    QCOMPARE(euro3.toString(), "100.00");
}

void EuroTest::euro_to_qstring()
{
    qRegisterMetaTypeStreamOperators<Euro>("Euro");
    Euro euro1(12345);

    QString str;
    str << euro1;

    QCOMPARE(str, "123.45");
}

void EuroTest::euro_eq_qstring()
{
    Euro euro1("-785.52");
    QVERIFY( euro1 == "-785.52" );

}

void EuroTest::qvariant_to_euro()
{
    QVariant variant(123.5);
    Euro euro;
    euro << variant;
    QCOMPARE(euro.cents(), 12350);
}


QTEST_APPLESS_MAIN(EuroTest)

#include "tst_eurotest.moc"