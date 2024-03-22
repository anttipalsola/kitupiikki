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
#ifndef TOSITEVIENTI_H
#define TOSITEVIENTI_H

#include <QVariant>
#include <QDate>
#include <map>

#include "euro.h"

class TositeVienti : public QVariantMap
{
public:
    enum Avain {
        ID,
        PVM,
        TILI,
        DEBET,
        KREDIT,
        SELITE,
        ALVKOODI,
        ALVPROSENTTI,
        KOHDENNUS,
        MERKKAUKSET,
        JAKSOALKAA,
        JAKSOLOPPUU,
        ERA,
        ARKISTOTUNNUS,
        KUMPPANI,
        TYYPPI,
        PALKKAKOODI,
        TASAERAPOISTO,
        ALKUPVIENNIT,
        VIITE,
        AALV,
        OSTOPVM,
        JAKSOTUSTILI,
        TILIOTE
    };

    enum VientiTyyppi {
        TUNTEMATON = 0,
        KIRJAUS = 1,
        VASTAKIRJAUS = 2,
        ALVKIRJAUS = 3,
        MAAHANTUONTIVASTAKIRJAUS = 31,
        VAHENNYSKELVOTON = 32,
        OSTO = 100,
        MYYNTI = 200,
        SUORITUS = 300,
        SIIRTO = 400,
        BRUTTOOIKAISU = 91091,
        POISTO = 99100,
        JAKSOTUS_TP = 99210,
        JAKSOTUS_TA = 99220

    };


    TositeVienti(const QVariantMap& vienti = QVariantMap());

    QVariant data(int kentta) const;
    void set(int kentta, const QVariant& arvo);
    QVariant tallennettava() const;

    int id() const { return data(ID).toInt();}
    QDate pvm() const { return  data(PVM).toDate();}
    int alvkoodi() const { return data(ALVKOODI).toInt();}
    int tili() const { return data(TILI).toInt();}

    double debet() const { return data(DEBET).toDouble();}
    qlonglong debetSnt() const;
    Euro debetEuro() const { return Euro::fromVariant(data(DEBET)); }

    double kredit() const { return data(KREDIT).toDouble();}
    qlonglong kreditSnt() const;
    Euro kreditEuro() const { return Euro::fromVariant(data(KREDIT));}

    int alvKoodi() const { return data(ALVKOODI).toInt();}
    double alvProsentti() const { return data(ALVPROSENTTI).toDouble();}
    int kohdennus() const { return  data(KOHDENNUS).toInt();}
    QString selite() const { return data(SELITE).toString();}
    QVariantMap era() const;
    int eraId() const;
    int kumppaniId() const;
    QString kumppaniNimi() const;
    QVariantMap kumppaniMap() const { return data(KUMPPANI).toMap();}
    QVariantList merkkaukset() const;
    QString arkistotunnus() const { return data(ARKISTOTUNNUS).toString();}
    int tyyppi() const { return data(TYYPPI).toInt(); }
    QString palkkakoodi() const { return data(PALKKAKOODI).toString(); }
    int tasaerapoisto() const { return data(TASAERAPOISTO).toInt();}
    QDate jaksoalkaa() const { return data(JAKSOALKAA).toDate();}
    QDate jaksoloppuu() const { return data(JAKSOLOPPUU).toDate();}
    QString viite() const { return data(VIITE).toString();}
    QDate ostopvm() const { return data(OSTOPVM).toDate(); }
    QVariantMap tilioteTieto() const { return data(TILIOTE).toMap();}

    void setPvm(const QDate& pvm);
    void setTili(int tili);

    void setDebet(double euroa);
    void setDebet(qlonglong senttia);
    void setDebet(const QString& euroa);
    void setDebet(const Euro& euroa);

    void setKredit(double euroa);
    void setKredit(qlonglong senttia);
    void setKredit(const QString& euroa);
    void setKredit(const Euro& euroa);

    void setSelite(const QString& selite);
    void setAlvKoodi(int koodi);
    void setAlvProsentti(double prosentti);
    void setAlvProsentti(const QString& prosentti);
    void setKohdennus(int kohdennus);
    void setMerkkaukset( QVariantList merkkaukset);
    void setJaksoalkaa( const QDate& pvm);
    void setJaksoloppuu( const QDate& pvm );
    void setEra(int era);
    void setEra(const QVariantMap& era);
    void setArkistotunnus(const QString& tunnus);
    void setKumppani(int kumppaniId);
    void setKumppani(const QString& kumppani);
    void setKumppani(const QVariantMap& map);
    void setTyyppi(int tyyppi);
    void setPalkkakoodi(const QString& palkkakoodi);
    void setTasaerapoisto(int kuukautta);
    void setId(int id);
    void setViite(const QString& viite);
    void setOstoPvm(const QDate& pvm);
    void setJaksotustili(const int tili);
    void setTilioteTieto(const QVariantMap& tieto);

    void siivoa();

private:
    static std::map<int,QString> avaimet__;

};

#endif // TOSITEVIENTI_H
