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
#include "saldotroute.h"

#include "db/kirjanpito.h"

#include <QDebug>

SaldotRoute::SaldotRoute(SQLiteModel* model) :
    SQLiteRoute(model, "/saldot")
{

}

QVariant SaldotRoute::get(const QString &/*polku*/, const QUrlQuery &urlquery)
{
    QDate pvm = QDate::fromString(urlquery.queryItemValue("pvm"),Qt::ISODate);
    Tilikausi kausi = kp()->tilikaudet()->tilikausiPaivalle(pvm);
    QDate kaudenalku = kausi.alkaa();
    if( urlquery.hasQueryItem("alkupvm"))
        kaudenalku = QDate::fromString( urlquery.queryItemValue("alkupvm"), Qt::ISODate );
    if( urlquery.hasQueryItem("kustannuspaikat"))
        return kustannuspaikat( kaudenalku, pvm, false );
    if( urlquery.hasQueryItem("projektit")) {
        int kuuluu = -1;
        if( urlquery.hasQueryItem("kohdennus"))
            kuuluu = urlquery.queryItemValue("kohdennus").toInt();
        return kustannuspaikat( kaudenalku, pvm, true, kuuluu );
    }

    QSqlQuery kysely(db());
    QVariantMap saldot;


    if( urlquery.hasQueryItem("tili")) {
        Tili* tili = kp()->tilit()->tili(urlquery.queryItemValue("tili").toInt());
        if(tili) {
            QString kysymys = QString("SELECT sum(debetsnt), sum(kreditsnt) "
                                      "FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id "
                                      "WHERE Vienti.tili = %1 AND Tosite.tila >= 100 AND ").arg(tili->numero()) +
                   ( tili->onko(TiliLaji::TULOS) ? QString("vienti.pvm BETWEEN '%1' AND '%2").arg(kaudenalku.toString(Qt::ISODate).arg(pvm.toString(Qt::ISODate)))
                                                 : ( urlquery.hasQueryItem("alkusaldot")  ? QString("vienti.pvm < '%1'").arg(pvm.toString(Qt::ISODate))  :  QString("vienti.pvm <= '%1'").arg(pvm.toString(Qt::ISODate)) ));
            kysely.exec(kysymys);
            if(kysely.next()) {
                double saldo = tili->onko(TiliLaji::VASTAAVAA)
                        ? (kysely.value(0).toLongLong() - kysely.value(1).toLongLong()) / 100.0
                        : (kysely.value(1).toLongLong() - kysely.value(0).toLongLong()) / 100.0;
                saldot.insert(urlquery.queryItemValue("tili"), saldo);
            }
        }
        return saldot;
    }




    if( !urlquery.hasQueryItem("tuloslaskelma")) {
        QString kysymys = "SELECT tili, sum(debetsnt), sum(kreditsnt) FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id WHERE vienti.pvm ";
        kysymys += urlquery.hasQueryItem("alkusaldot") ? "<" : "<=";
        kysymys += QString("'%1' ").arg(pvm.toString(Qt::ISODate));
        if( urlquery.hasQueryItem("tili"))
            kysymys += QString(" AND tili=%1 ").arg(urlquery.queryItemValue("tili").toInt());
        kysymys += " AND CAST(tili as text) < '3' AND Tosite.tila >= 100 GROUP BY tili ORDER BY tili ";

        kysely.exec(kysymys);
        while (kysely.next()) {
            QString tilistr = kysely.value(0).toString();
            Euro debet = Euro::fromCents(kysely.value(1).toLongLong());
            Euro kredit = Euro::fromCents(kysely.value(2).toLongLong());
            if( tilistr.startsWith(QChar('1')))
                saldot.insert( tilistr, (debet - kredit).toString());
            else
                saldot.insert( tilistr, (kredit - debet).toString());
        }

        // Edellisten tulos
        kysely.exec(QString("SELECT sum(kreditsnt), sum(debetsnt) FROM Vienti Join Tosite ON Vienti.tosite=Tosite.id WHERE CAST(tili as text) >= '3' "
                            "AND vienti.pvm<'%1' AND Tosite.tila >= 100").arg(kausi.alkaa().toString(Qt::ISODate)));
        if( kysely.next()) {
            QString edtili = QString::number( kp()->tilit()->tiliTyypilla(TiliLaji::EDELLISTENTULOS).numero() ) ;
            double saldo = ( qRound64(saldot.value(edtili).toDouble()*100) + kysely.value(0).toLongLong() - kysely.value(1).toLongLong() ) / 100.0 ;
            saldot[edtili] = saldo;
        }

        if( !urlquery.hasQueryItem("alkusaldot")) {
            // Nykyisen tulos
            kysely.exec(QString("SELECT sum(kreditsnt), sum(debetsnt) FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id WHERE CAST(tili as text) >= '3' "
                                "AND vienti.pvm BETWEEN '%1' AND '%2' AND Tosite.tila >= 100")
                        .arg(kausi.alkaa().toString(Qt::ISODate), pvm.toString(Qt::ISODate)));
            if( kysely.next()) {
                QString tulostili = QString::number( kp()->tilit()->tiliTyypilla(TiliLaji::KAUDENTULOS).numero() ) ;
                saldot.insert(tulostili, (kysely.value(0).toLongLong() - kysely.value(1).toLongLong()) / 100.0 );
            }
        }
    }

    if( !urlquery.hasQueryItem("tase")) {
        Kohdennus kohdennus = kp()->kohdennukset()->kohdennus( urlquery.queryItemValue("kohdennus").toInt() );

        QString kysymys("SELECT tili, SUM(kreditsnt), SUM(debetsnt) FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id ");
        if( kohdennus.tyyppi() == Kohdennus::KUSTANNUSPAIKKA || kohdennus.tyyppi() == Kohdennus::PROJEKTI ) {
            kysymys.append("JOIN Kohdennus ON Vienti.kohdennus=Kohdennus.id");
        } else if( kohdennus.tyyppi() == Kohdennus::MERKKAUS) {
            kysymys.append("LEFT OUTER JOIN Merkkaus ON Vienti.id=Merkkaus.vienti ");
        }


        kysymys.append(" WHERE vienti.pvm ");

        if( urlquery.hasQueryItem("alkusaldot"))
            kysymys += "<";
        else
            kysymys += "<=";
        kysymys += QString(" '%1' ").arg(pvm.toString(Qt::ISODate));

        if( kohdennus.tyyppi() == Kohdennus::KUSTANNUSPAIKKA || kohdennus.tyyppi() == Kohdennus::PROJEKTI ) {
            kysymys += QString(" AND (kohdennus.id=%1 OR kohdennus.kuuluu=%1) ").arg(kohdennus.id());
        } else if( kohdennus.tyyppi() == Kohdennus::MERKKAUS) {
            kysymys += QString(" AND Merkkaus.kohdennus=%1 ").arg(kohdennus.id());
        }

        kysymys += QString(" AND vienti.pvm >= '%1' AND CAST(tili as text) >= 3 AND Tosite.tila >= 100 GROUP BY tili ORDER BY tili")
                .arg(kaudenalku.toString(Qt::ISODate));

        if( !kysely.exec(kysymys) )
            throw SQLiteVirhe(kysely);

        while( kysely.next()) {
            Euro kredit = Euro::fromCents(kysely.value(1).toLongLong());
            Euro debet = Euro::fromCents(kysely.value(2).toLongLong());
            saldot.insert( kysely.value(0).toString(), (kredit-debet).toString());
        }
    }
    return saldot;
}

QVariant SaldotRoute::kustannuspaikat(const QDate &mista, const QDate &mihin, bool projektit, int kuuluu)
{
    QVariantMap kohdennukset;

    QSqlQuery kysely(db());
    QString kysymys;

    if( projektit) {
        kysymys = QString("SELECT kohdennus, tili, SUM(kreditsnt) as ks, SUM(debetsnt) as ds "
                          "FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id "
                          "JOIN Kohdennus ON Vienti.kohdennus=Kohdennus.id "
                          "WHERE Tosite.tila >= 100 AND Kohdennus.tyyppi=2 AND CAST(tili as text) >= '3' "
                          "AND Vienti.pvm BETWEEN '%1' AND '%2' %3"
                          "GROUP BY kohdennus,tili ")
                .arg(mista.toString(Qt::ISODate), mihin.toString(Qt::ISODate),
                     kuuluu > -1 ? QString("AND kuuluu=%1 ").arg(kuuluu) : "" );
    }
    else
        kysymys = QString("SELECT kohdennus, kuuluu, tili, SUM(kreditsnt) as ks, SUM(debetsnt) as ds "
                          "FROM Vienti JOIN Tosite ON Vienti.tosite=Tosite.id "
                          "JOIN Kohdennus ON Vienti.kohdennus=Kohdennus.id "
                          "WHERE Tosite.tila >= 100 AND CAST(tili as text) >= '3' "
                          "AND Vienti.pvm BETWEEN '%1' AND '%2' "
                          "GROUP BY kohdennus,tili ")
                .arg(mista.toString(Qt::ISODate)).arg(mihin.toString(Qt::ISODate));

    if( !kysely.exec(kysymys) )
        throw SQLiteVirhe(kysely);

    while( kysely.next()) {
        QString kohdennus = kysely.value("kohdennus").toString();
        if( !projektit && !kysely.value("kuuluu").isNull())
            kohdennus = kysely.value("kuuluu").toString();

        QString tilistr = kysely.value("tili").toString();

        QVariantMap kmap = kohdennukset.value(kohdennus).toMap();
        kmap.insert(tilistr, (  qRound64(kmap.value(tilistr).toDouble() * 100.0 ) +
                                qRound64(kysely.value("ks").toDouble() ) -
                                qRound64(kysely.value("ds").toDouble() )) / 100.0 );

        kohdennukset.insert( kohdennus, kmap);
    }


    return kohdennukset;
}
