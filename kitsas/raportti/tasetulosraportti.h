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
#ifndef TASETULOSRAPORTTI_H
#define TASETULOSRAPORTTI_H

#include "raporttiwidget.h"
#include "ui_muokattavaraportti.h"

class TaseTulosRaportti : public RaporttiWidget
{
    Q_OBJECT
public:


    TaseTulosRaportti(const QString& raportinTyyppi, bool kuukausittain = false, QWidget* parent = nullptr);

public slots:
//    void esikatsele() override;

protected:
    void tallenna() override;

protected slots:
    void muotoVaihtui();

    void paivitaKielet();
    void paivitaMuodot();

    void paivitaKohdennusPaivat();

    void muotoVaihdettu();
    void kieliVaihdettu();


protected:
    void paivitaUi();

private:
    void lisaaSarake(bool kaytossa, const QDate& alku, const QDate& loppu, int valintaIndeksi);


private:
    QString kaava_;

    Ui::MuokattavaRaportti *ui;
    QString tyyppi_;

    bool paivitetaan_ = false;
    bool kuukausittain_ = false;

};

#endif // TASETULOSRAPORTTI_H
