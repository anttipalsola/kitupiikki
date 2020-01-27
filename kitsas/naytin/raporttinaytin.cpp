/*
   Copyright (C) 2018 Arto Hyvättinen

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
#include "raporttinaytin.h"

#include <QPainter>
#include <QDebug>

Naytin::RaporttiNaytin::RaporttiNaytin(const RaportinKirjoittaja &raportti, QObject *parent)
    : PrintPreviewNaytin (parent),
      raportti_(new RaportinKirjoittaja(raportti))
{    
}

Naytin::RaporttiNaytin::~RaporttiNaytin()
{
    delete raportti_;
}

QString Naytin::RaporttiNaytin::otsikko() const
{
    return raportti_->otsikko();
}

bool Naytin::RaporttiNaytin::csvMuoto() const
{
    return raportti_->csvKaytossa();
}

QByteArray Naytin::RaporttiNaytin::csv() const
{
    return raportti_->csv();
}

QByteArray Naytin::RaporttiNaytin::data() const
{
    return raportti_->pdf( onkoRaidat() );
}

QString Naytin::RaporttiNaytin::html() const
{
    return raportti_->html();
}

void Naytin::RaporttiNaytin::tulosta(QPrinter *printer) const
{
    QPainter painter(printer);
    raportti_->tulosta(printer, &painter, onkoRaidat());
}