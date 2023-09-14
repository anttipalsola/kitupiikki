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
#include "eramap.h"
#include <QIcon>

EraMap::EraMap()
{

}

EraMap::EraMap(int id)
{
    insert("id", id);
}

EraMap::EraMap(const QVariantMap &map) :
    QVariantMap(map)
{

}

Euro EraMap::saldo() const
{
    if( contains("saldo"))
        return Euro::fromVariant(value("saldo"));
    else
        return Euro::fromVariant(value("avoin"));
}

QIcon EraMap::tyyppiKuvake() const
{
    return kuvakeTyypilla( eratyyppi());
}

int EraMap::eratyyppi() const
{
    if( id() > 0)
        return Lasku;
    else
        return id() % 10;
}


EraMap EraMap::AsiakasEra(int id, const QString &nimi)
{
    EraMap map;
    map.insert("id", -10 * id - 3);
    QVariantMap asMap;
    asMap.insert("nimi", nimi);
    asMap.insert("id", id);
    map.insert("asiakas", asMap);
    return map;
}

QIcon EraMap::kuvakeTyypilla(int tyyppi)
{
    switch( tyyppi ) {
    case Asiakas: return QIcon(":/pic/mies.png");
    case Huoneisto: return QIcon(":/pic/talo.png");
    case Lasku: return QIcon(":/pic/lasku.png");
    case Uusi: return QIcon(":/pic/lisaa.png");
    default: return QIcon(":/pic/tyhja.png");
    }
}

QIcon EraMap::kuvakeIdlla(int eraid)
{
    if( eraid > 0)
        return kuvakeTyypilla(Lasku);
    else
        return kuvakeTyypilla( eraid % 10 );
}
