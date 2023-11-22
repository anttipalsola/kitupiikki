/*
   Copyright (C) 2017 Arto Hyvättinen

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

#ifndef SELAUSMODEL_H
#define SELAUSMODEL_H

#include <QAbstractTableModel>
#include <QSet>
#include <QList>
#include <QDate>
#include <QSqlQuery>

#include "db/tili.h"
#include "db/kohdennus.h"
#include "model/euro.h"

class SQLiteModel;

class SelausRivi
{
public:
     SelausRivi(const QVariantMap& data, bool samakausi = false);
     SelausRivi(QSqlQuery& data, bool samakausi, SQLiteModel *sqlite, bool merkkauksia);

     QVariant data(int sarake, int role, bool alternateColor) const;
     int getTili() const { return tili;}
     QString getEtsi() const { return etsi;}

protected:
    int tositeId;
    int vientiId;
    QString tositeTunnus;
    QString vertailuTunnus;

    QDate pvm;
    int tositeTyyppi;
    int tili;
    Euro debet;
    Euro kredit;
    QString kohdennus;
    QIcon kohdennuskuvake;
    QString kumppani;
    QString selite;
    QString etsi;
    int kohdennustyyppi;
    bool liitteita;
    int alvKoodi;
    int alvProsentti;
};

/**
 * @brief Selaussivun model vientien selaamiseen
 */
class SelausModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum SelausSarake
    {
        TOSITE, PVM, TILI, DEBET, KREDIT, KOHDENNUS, ALV,  KUMPPANI, SELITE
    };

    enum {
        IdRooli = Qt::UserRole,
        TiliRooli = Qt::UserRole + 7,
        EtsiRooli = Qt::UserRole + 128,
    };

    SelausModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;

    void lataaSqlite(SQLiteModel *sqlite, const QDate& alkaa, const QDate& loppuu, int tili=0);

    int tili(int rivi) const;
    QString etsiTeksti(int rivi) const;

    QList<int> tiliLista() const;

    int selaustili() const { return tiliselaus_; }

public slots:
    void lataa(const QDate& alkaa, const QDate& loppuu, int tili=0);

    void tietoSaapuu(QVariant *map);

protected:
    QSet<int> kaytetytTilit_;
    QList<SelausRivi> rivit_;

//    QVariantList lista_;
    bool samakausi_ = false;
    int tiliselaus_ = 0;
    bool ladataan_ = false;

};

#endif // SELAUSMODEL_H
