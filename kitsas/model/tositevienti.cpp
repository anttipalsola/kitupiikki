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
#include "tositevienti.h"

#include "db/kirjanpito.h"

#include <QDebug>

TositeVienti::TositeVienti(const QVariantMap &vienti) :
    QVariantMap (vienti)
{

}

QVariant TositeVienti::data(int kentta) const
{
    return value( avaimet__.at(kentta) );
}

void TositeVienti::set(int kentta, const QVariant &arvo)
{    
    if( (arvo.toString().isEmpty()) && !(arvo.type() == QVariant::Map && !arvo.toMap().isEmpty()) &&
        !(arvo.type() == QVariant::List && !arvo.toList().isEmpty()))
        remove( avaimet__.at(kentta));
    else
        insert( avaimet__.at(kentta), arvo);
}

QVariant TositeVienti::tallennettava() const
{
    // Tallennettavassa erän tilalla on kyseiset id:t

    QVariantMap ulos(*this);
    return ulos;
}

qlonglong TositeVienti::debetSnt() const
{
    return qRound64( debet() * 100.0);
}

qlonglong TositeVienti::kreditSnt() const
{
    return qRound64( kredit() * 100.0);
}

QVariantMap TositeVienti::era() const
{
    return value("era").toMap();
}

int TositeVienti::eraId() const
{
    return era().value("id").toInt();
}

int TositeVienti::kumppaniId() const
{
    return value("kumppani").toMap().value("id").toInt();
}

QString TositeVienti::kumppaniNimi() const
{
    return value("kumppani").toMap().value("nimi").toString();
}

QVariantList TositeVienti::merkkaukset() const
{
    return data(MERKKAUKSET).toList();
}

void TositeVienti::setPvm(const QDate &pvm)
{
    set( PVM, pvm );
}

void TositeVienti::setTili(int tili)
{
    if(tili)
        set( TILI, tili);
    else
        remove(avaimet__.at(TILI));
}

void TositeVienti::setDebet(double euroa)
{
    Euro euro = Euro::fromDouble(euroa);
    setDebet(euro);
}

void TositeVienti::setDebet(qlonglong senttia)
{
    Euro euro = Euro(senttia);
    setDebet(euro);
}

void TositeVienti::setDebet(const QString &euroa)
{
    if( euroa.startsWith('-')) {
        set( KREDIT, euroa.mid(1));
        remove( avaimet__.at(DEBET));

    } else {
        set( DEBET, euroa);
        if( euroa.toDouble() > 1e-5)
            remove( avaimet__.at(KREDIT));
    }
}

void TositeVienti::setDebet(const Euro &euroa)
{
    setDebet( euroa.toString() );
}

void TositeVienti::setKredit(double euroa)
{
    Euro euro = Euro::fromDouble(euroa);
    setKredit(euro);
}

void TositeVienti::setKredit(qlonglong senttia)
{
    Euro euro(senttia);
    setKredit( euro );
}

void TositeVienti::setKredit(const QString &euroa)
{
    if( euroa.startsWith('-')) {
        set(DEBET, euroa.mid(1));
        remove( avaimet__.at(KREDIT));
    } else {
        set( KREDIT, euroa);
        if( euroa.toDouble() > 1e-5)
            remove( avaimet__.at(DEBET));
    }
}

void TositeVienti::setKredit(const Euro &euroa)
{
    setKredit( euroa.toString() );
}

void TositeVienti::setSelite(const QString &selite)
{
    set( SELITE, selite.trimmed());
}

void TositeVienti::setAlvKoodi(int koodi)
{
    set(ALVKOODI, koodi);
    if( kp()->alvTyypit()->nollaTyyppi(koodi) ) {
        remove( avaimet__.at(ALVPROSENTTI) );
    }
}

void TositeVienti::setAlvProsentti(double prosentti)
{
    set(ALVPROSENTTI, QString("%1").arg(prosentti,0,'f',2));
}

void TositeVienti::setAlvProsentti(const QString &prosentti)
{
    set( ALVPROSENTTI, prosentti);
}

void TositeVienti::setKohdennus(int kohdennus)
{
    set( KOHDENNUS, kohdennus);
}

void TositeVienti::setMerkkaukset(QVariantList merkkaukset)
{
    if( merkkaukset.isEmpty())
        remove( avaimet__.at(MERKKAUKSET));
    else
        insert( avaimet__.at(MERKKAUKSET), merkkaukset);
}

void TositeVienti::setJaksoalkaa(const QDate &pvm)
{
    set( JAKSOALKAA, pvm);
}

void TositeVienti::setJaksoloppuu(const QDate &pvm)
{
    set( JAKSOLOPPUU, pvm );
}

void TositeVienti::setEra(int era)
{
    QVariantMap map;
    map.insert("id",era);
    setEra(map);
}

void TositeVienti::setEra(const QVariantMap &era)
{
    if( era.isEmpty() || era.value("id").toInt() == 0)
        remove(avaimet__.at(ERA));
    else
        insert(avaimet__.at(ERA), era);
}

void TositeVienti::setArkistotunnus(const QString &tunnus)
{
    set( ARKISTOTUNNUS, tunnus.left(64));
}

void TositeVienti::setKumppani(int kumppaniId)
{
    if(!kumppaniId) {
        remove(avaimet__.at(KUMPPANI));
    } else {
        QVariantMap kmap;
        kmap.insert("id", kumppaniId);
        set( KUMPPANI, kmap );
    }
}

void TositeVienti::setKumppani(const QString &kumppani)
{
    if( kumppani.isEmpty()) {
        remove(avaimet__.at(KUMPPANI));
    } else {
        QVariantMap kmap;
        kmap.insert("nimi",kumppani);
        set( KUMPPANI, kmap);
    }
}

void TositeVienti::setKumppani(const QVariantMap &map)
{
    set( KUMPPANI, map);
}

void TositeVienti::setTyyppi(int tyyppi)
{
    set( TYYPPI, tyyppi);
}

void TositeVienti::setPalkkakoodi(const QString &palkkakoodi)
{
    set( PALKKAKOODI, palkkakoodi);
}

void TositeVienti::setTasaerapoisto(int kuukautta)
{
    if( kuukautta)
        set( TASAERAPOISTO, kuukautta);
    else
        remove( avaimet__.at(TASAERAPOISTO) );
}

void TositeVienti::setId(int id)
{
    set( ID, id);
}

void TositeVienti::setViite(const QString &viite)
{
    set( VIITE, viite);
}

void TositeVienti::setOstoPvm(const QDate &pvm)
{
    set( OSTOPVM, pvm);
}

void TositeVienti::setJaksotustili(const int tili)
{
    set( JAKSOTUSTILI, tili);
}






std::map<int,QString> TositeVienti::avaimet__ = {
    { ID, "id" },
    { PVM, "pvm"},
    { TILI, "tili"},
    { DEBET, "debet"},
    { KREDIT, "kredit"},
    { SELITE, "selite"},
    { ALVKOODI, "alvkoodi"},
    { ALVPROSENTTI, "alvprosentti"},
    { KOHDENNUS, "kohdennus"},
    { MERKKAUKSET, "merkkaukset"},
    { JAKSOALKAA, "jaksoalkaa"},
    { JAKSOLOPPUU, "jaksoloppuu"},
    { ERA, "era"},
    { ARKISTOTUNNUS, "arkistotunnus"},
    { KUMPPANI, "kumppani"},
    { TYYPPI, "tyyppi"},
    { PALKKAKOODI, "palkkakoodi"},
    { TASAERAPOISTO, "tasaerapoisto"},
    { ALKUPVIENNIT, "alkupviennit"},
    { VIITE, "viite"},
    { AALV, "aalv"},
    { OSTOPVM, "ostopvm"},
    { JAKSOTUSTILI, "jaksotustili"}
};
