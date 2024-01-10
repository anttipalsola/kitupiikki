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
#ifndef TILIOTEMODEL_H
#define TILIOTEMODEL_H

#include <QAbstractTableModel>

#include "tiliotekirjausrivi.h"
#include "tilioteharmaarivi.h"



class KitsasInterface;
class QSortFilterProxyModel;

class TilioteModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TilioteModel(QObject *parent, KitsasInterface* kitsasInterface);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertRows(int row, int count, const QModelIndex& parent) override;
    int lisaaRivi(const QDate &pvm);
    int lisaaRivi(TilioteKirjausRivi rivi);
    void poistaRivi(const int indeksi);
    void asetaRivi(int indeksi, const TilioteKirjausRivi &rivi);
    TilioteKirjausRivi rivi(const int indeksi) const;

    void lataa(const QVariantList &lista);
    void tuo(const QVariantList tuotavat);
    void lataaHarmaat(const QDate& mista, const QDate& mihin);

    int tilinumero() const { return tili_;}
    void asetaTilinumero(int tilinumero);

    QVariantList viennit() const;

    QPair<qlonglong,qlonglong> summat() const;

    void salliMuokkaus(bool sallittu);

    KitsasInterface* kitsas() const { return kitsasInterface_;}
    int lisaysIndeksi();

    void tilaaAlkuperaisTosite(int lisaysIndeksi, int eraId);

    void asetaTositeId(int id);
    int tositeId() const;

    QSortFilterProxyModel *initProxy();

protected:
    void harmaatSaapuu(QVariant* data);
    void peita(int harmaaIndeksi, int kirjausIndeksi);
    void peitaHarmailla(int harmaaIndeksi);
    void peitaHarmailla();
    void alkuperaisTositeSaapuu(int lisaysIndeksi, QVariant* data, int eraId);

private:
    QList<TilioteKirjausRivi> kirjausRivit_;
    QList<TilioteHarmaaRivi> harmaatRivit_;

    int lisaysIndeksi_ = 0;
    int tili_ = 0;
    int harmaaLaskuri_ = 0;
    bool muokkausSallittu_ = true;
    int tositeId_ = 0;

    KitsasInterface* kitsasInterface_;
    QSortFilterProxyModel* proxy_ = nullptr;

    QDate alkuPvm_;
    QDate loppuPvm_;
};

#endif // TILIOTEMODEL_H
