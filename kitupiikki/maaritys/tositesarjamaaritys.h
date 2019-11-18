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
#ifndef TOSITESARJAMAARITYS_H
#define TOSITESARJAMAARITYS_H

#include "maarityswidget.h"
#include "ui_tositesarjat.h"

class TositesarjaMaaritys : public MaaritysWidget
{
public:
    TositesarjaMaaritys();

    QString ohjesivu() override { return "maaritykset/tositesarjat";}


    bool nollaa() override;
    bool tallenna() override;

protected:
    Ui::Tositesarjat *ui;
};

#endif // TOSITESARJAMAARITYS_H
