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

/**
  * @dir selaus
  * @brief Tositteiden ja vientien selaaminen
  */

#ifndef SELAUSWG_H
#define SELAUSWG_H

#include <QWidget>

#include "ui_selauswg.h"
#include "db/tilikausi.h"

#include "kitupiikkisivu.h"

class SelausModel;
class TositeSelausModel;

#include "tositeselausproxymodel.h"
#include "selausproxymodel.h"
#include "model/euro.h"
#include "kirjaus/kirjaussivu.h"

/**
 * @brief Sivu kirjausten selaamiseen
 *
 * Sivulla on taulukko vienneistä sekä widgetit selauksen rajaamiseen
 * ajan ja tilin perusteella
 *
 */
class SelausWg : public KitupiikkiSivu
{
    Q_OBJECT

public:
    enum {
        TOSITTEET = 0,        
        LUONNOKSET = 1,
        VIENNIT = 2,
        HUOMIO = 3,
        POISTETUT = 4,
        SAAPUNEET = 5,
    };

    SelausWg(QWidget *parent = nullptr);
    ~SelausWg() override;

public slots:
    void alusta();
    void paivita();
    void suodata();
    void paivitaSuodattimet();
    void paivitaSummat(QVariant* data = nullptr);
    void naytaTositeRivilta(QModelIndex index);

    void selaa(int tilinumero, const Tilikausi &tilikausi);


    void naytaSaapuneet();
    void naytaHuomioitavat();

    void alkuPvmMuuttui();

    /**
     * @brief Selaa tositteita tai vientejä
     * @param kumpi 0-tositteet, 1 viennit
     */
    void selaa(int kumpi);

    void contextMenu(const QPoint& pos);
public:
    void siirrySivulle() override;

    QString ohjeSivunNimi() override { return "selaaminen"; }

protected slots:
    void modelResetoitu();
    void etsi(const QString& teksti);
    void tallennaKoot();
    void lataaKoot();

    void nuoliSelaus(bool seuraava);
    void tamaKuukausi();
    void tamaTilikausi();
    void palautaValinta();

    void poistoMoodiin(bool poistoon = false);
    void paivitaPoistoOhje();
    void teePoisto();
    void poistoValmis();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void tositeValittu(int id, QList<int> selauslista, KirjausSivu::Takaisinpaluu paluu);


private:
    Ui::SelausWg *ui;
    SelausModel *model;
    TositeSelausModel *tositeModel;

    TositeSelausProxyModel* tositeProxy_;
    SelausProxyModel* selausProxy_;

    /**
     * @brief Pitääkö sivu päivittää ennen sen näyttämistä
     */
    bool paivitettava = true;
    int valittu_ = 0;
    int skrolli_ = -1;
    Euro saldo_;
    int selaustili_ = -1;
    bool lataaKoon_ = false;
    int poistoLaskuri_ = 0;
};

#endif // SELAUSWG_H
