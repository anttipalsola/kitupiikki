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
#include "yksittainenlaskudialogi.h"

#include "model/tosite.h"
#include "rivivientigeneroija.h"
#include "db/kirjanpito.h"
#include "../tulostus/laskuntulostaja.h"

#include "../myyntilaskujentoimittaja.h"

#include "laskutus/toimittaja/laskuntoimittaja.h"

#include <QMessageBox>

YksittainenLaskuDialogi::YksittainenLaskuDialogi(Tosite *tosite, QWidget *parent)
    : KantaLaskuDialogi(tosite, parent)
{
    connect( tosite, &Tosite::tositeTallennettu, this, &YksittainenLaskuDialogi::tallennettu);
    connect( tosite, &Tosite::tallennusvirhe, [this] () { QMessageBox::critical(this, tr("Tallennusvirhe"),
                                                                                      tr("Laskun tallennus epäonnistui")); });
}

void YksittainenLaskuDialogi::tallenna(int tilaan)
{
    if( tarkasta() ) {
        tositteelle();
        valmisteleTallennus();
        tosite_->tallennaLiitteitta(tilaan);
    }
}

void YksittainenLaskuDialogi::tallennettu(QVariant *vastaus)
{
    Tosite tallennettuTosite;
    tallennettuTosite.lataaData(vastaus);
    QVariantMap data = tallennettuTosite.tallennettava();

    LaskunTulostaja* tulostaja = new LaskunTulostaja(kp());

    connect(tulostaja, &LaskunTulostaja::laskuLiiteTallennettu,
            [this, data] { this->liiteTallennettu(data); } );
    tulostaja->tallennaLaskuLiite(tallennettuTosite);
}

void YksittainenLaskuDialogi::liiteTallennettu(QVariantMap tosite)
{
    // Nyt tallennus on saatettu loppuun saakka!
    // Tähän vielä laskun toimittaminen

    QDialog::accept();
    emit kp()->kirjanpitoaMuokattu();

    int tila = tosite_->tila();
    if( tila == Tosite::LAHETETAAN) {
        LaskunToimittaja::toimita(tosite);
/*
        MyyntiLaskujenToimittaja *toimittaja = new MyyntiLaskujenToimittaja();
        QList<QVariantMap> lista;
        lista << tosite;
        toimittaja->toimitaLaskut(lista);
*/
    }
}
