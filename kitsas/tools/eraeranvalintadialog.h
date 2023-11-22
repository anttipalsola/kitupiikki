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
#ifndef ERAERANVALINTADIALOG_H
#define ERAERANVALINTADIALOG_H

#include "eranvalintadialog.h"

class EranValintaModel;

class EraEranValintaDialog : public EranValintaDialog
{
    Q_OBJECT
public:
    EraEranValintaDialog(int tili, int asiakas, int nykyinen, QWidget* parent = nullptr);

protected:    
    void paivita();


private:    
    EranValintaModel* model_;

};

#endif // ERAERANVALINTADIALOG_H
