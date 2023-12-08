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

#include <QJsonDocument>
#include <QSettings>

#include "tilikausimodel.h"
#include "kirjanpito.h"

TilikausiModel::TilikausiModel(QObject *parent) :
    QAbstractTableModel(parent)
{

}

int TilikausiModel::rowCount(const QModelIndex & /* parent */) const
{
    return kaudet_.count();
}

int TilikausiModel::columnCount(const QModelIndex & /* parent */) const
{
    return 8;
}

QVariant TilikausiModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role == Qt::TextAlignmentRole)
        return QVariant( Qt::AlignCenter | Qt::AlignVCenter);

    else if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch (section)
        {
        case KAUSI :
            return tr("Tilikausi");
        case LIIKEVAIHTO:
            return tr("Liikevaihto");
        case TASE:
            return tr("Tase");
        case TULOS:
            return tr("Yli/alijäämä");
        case LIITEKOKO:
            return tr("Liitteiden koko");
        case ARKISTOITU:
            return tr("Arkistoitu");
        case TILINPAATOS:
            return tr("Tilinpäätös");
        case LYHENNE:
            return QString("Tunnus");
        }
    }
    return QVariant();

}

QVariant TilikausiModel::data(const QModelIndex &index, int role) const
{
    if( !index.isValid())
        return QVariant();

    Tilikausi kausi = kaudet_.value(index.row());

    if( role == Qt::DisplayRole)
    {
        if( index.column() == KAUSI)
            return QVariant( tr("%1 - %2")
                             .arg(kausi.alkaa().toString("dd.MM.yyyy"),
                                kausi.paattyy().toString("dd.MM.yyyy")));
        else if( index.column() == TULOS)
            return QString("%L1 €").arg( kausi.tulos()  / 100.0,0,'f',2);
        else if(index.column() == LIIKEVAIHTO)
            return QString("%L1 €").arg( kausi.liikevaihto()  / 100.0,0,'f',2);
        else if(index.column() == TASE)
            return QString("%L1 €").arg( kausi.tase()  / 100.0,0,'f',2);
        else if( index.column() == LIITEKOKO) {
            const qlonglong megat = kausi.koko() / ( 1024L * 1024L );
            if( megat < 1) return QVariant();
            return QString("%L1 Mt").arg(megat);
        } else if( index.column() == ARKISTOITU )
            return kausi.arkistoitu().date();
        else if( index.column() == TILINPAATOS )
        {
            if( kausi.tilinpaatoksenTila() == Tilikausi::VAHVISTETTU)
                return tr("Vahvistettu");
            else if( kausi.tilinpaatoksenTila() == Tilikausi::KESKEN)
                return tr("Keskeneräinen");
            else if( kausi.tilinpaatoksenTila() == Tilikausi::EILAADITATILINAVAUKSELLE)
                return tr("Tilinavaus");
            else if( kausi.paattyy().daysTo( kp()->paivamaara()) > 1 &&
                     kausi.paattyy().daysTo( kp()->paivamaara()) < 4 * 30 )

            {
                if(  kp()->asetukset()->asetus(AsetusModel::Muoto) == "tmi" && kausi.pieniElinkeinonharjoittaja() < 1 )
                    return tr("Ei pakollinen");
                else
                    return tr("Aika laatia!");
            }
        }
        else if( index.column() == LYHENNE)
            return kausi.kausitunnus();
    }
    else if( role == AlkaaRooli)
        return QVariant( kausi.alkaa());
    else if( role == PaattyyRooli )
        return QVariant( kausi.paattyy());
    else if( role == HenkilostoRooli )
        return kausi.luku("Henkilosto");
    else if( role == LyhenneRooli)
        return  kausi.kausitunnus();
    else if( role == Qt::TextAlignmentRole)
    {
        if( index.column()>=TASE && index.column() <= LIITEKOKO )
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        else
            return QVariant( Qt::AlignLeft | Qt::AlignVCenter);

    }
    else if( role == Qt::DecorationRole )
    {
        if( index.column() == KAUSI)
        {

        if( kp()->tilitpaatetty() >= kausi.paattyy() )
            return QIcon(":/pic/lukittu.png");
        }
        else if(  index.column() == ARKISTOITU)
        {
            if( kausi.arkistoitu() > kausi.viimeinenPaivitys() &&
                    QFile::exists( kausi.uusiArkistopolku() + "/index.html") )
                return QIcon(":/pic/ok.png");
        }
        else if( index.column() == TILINPAATOS)
        {
            if( kausi.tilinpaatoksenTila() == Tilikausi::VAHVISTETTU)
                return QIcon(":/pic/ok.png");
            else if( kausi.tilinpaatoksenTila() == Tilikausi::EILAADITATILINAVAUKSELLE)
                return QIcon(":/pic/rahaa.png");
            else if( kausi.tilinpaatoksenTila() == Tilikausi::KESKEN &&
                     kausi.paattyy().daysTo( kp()->paivamaara()) > 4 * 30)                
                return QIcon(":/pic/varoitus.png");            
            else if( kausi.paattyy().daysTo( kp()->paivamaara()) > 1 &&
                     kausi.paattyy().daysTo( kp()->paivamaara()) < 4 * 30)
                return QIcon(":/pic/info.png");                        
        }
    }

    return QVariant();
}



Tilikausi TilikausiModel::tilikausiPaivalle(const QDate &paiva) const
{
    foreach (Tilikausi kausi, kaudet_)
    {
        // Osuuko pyydetty päivä kysyttyyn jaksoon
        if( kausi.alkaa().daysTo(paiva) >= 0 && paiva.daysTo(kausi.paattyy()) >= 0)
            return kausi;
    }
    return Tilikausi(QDate(), QDate()); // Kelvoton tilikausi

}


int TilikausiModel::indeksiPaivalle(const QDate &paiva) const
{
    for(int i=0; i < kaudet_.count(); i++)
        if( kaudet_[i].alkaa().daysTo(paiva) >= 0 && paiva.daysTo(kaudet_[i].paattyy()) >= 0)
            return i;
    return -1;

}

bool TilikausiModel::onkoTilikautta(const QDate &paiva) const
{
    return indeksiPaivalle(paiva) > -1;
}

Tilikausi TilikausiModel::tilikausiIndeksilla(int indeksi) const
{
    return kaudet_.value(indeksi, Tilikausi());
}

Tilikausi &TilikausiModel::viiteIndeksilla(int indeksi)
{
    return kaudet_[indeksi];
}

QDate TilikausiModel::kirjanpitoAlkaa() const
{
    if( kaudet_.count())
        return kaudet_.first().alkaa();
    return {};
}

QDate TilikausiModel::kirjanpitoLoppuu() const
{
    if( kaudet_.count())
        return kaudet_.last().paattyy();
    return {};
}


void TilikausiModel::lataa(const QVariantList &lista)
{
    beginResetModel();
    kaudet_.clear();

    for(const auto& item : lista )
    {
        QVariantMap map = item.toMap();
        kaudet_.append( Tilikausi(map) );
    }
    paivitaKausitunnukset();
    endResetModel();
}

void TilikausiModel::tallenna(const Tilikausi &kausi)
{
    KpKysely* kysely = nullptr;

    for(int i=0; i < kaudet_.count(); i++) {
        if( kausi.alkaa() == kaudet_[i].alkaa()) {
            kaudet_[i] = kausi;
            emit dataChanged(index(i,0), index(i, rowCount() ));
            kysely = kpk("/tilikaudet/" + kausi.alkaa().toString(Qt::ISODate), KpKysely::PUT);
            break;
        }
    }
    if( !kysely) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        kysely = kpk("/tilikaudet/" + kausi.alkaa().toString(Qt::ISODate), KpKysely::POST);
        kaudet_.append(kausi);
        endInsertRows();
    }
    QObject::connect(kysely, &KpKysely::vastaus, this, &TilikausiModel::paivita);
    QObject::connect(kysely, &KpKysely::vastaus, kp(), &Kirjanpito::tilikausiAvattu);
    kysely->kysy( kausi.data() );
}

void TilikausiModel::poista(const QDate &alkupvm)
{
    for(int i=0; i < kaudet_.count(); i++) {
        if( alkupvm == kaudet_[i].alkaa()) {
            beginRemoveRows(QModelIndex(), i, i);
            kaudet_.removeAt(i);
            endRemoveRows();
            break;
        }
    }
    KpKysely *poisto = kpk(QString("/tilikaudet/%1").arg(alkupvm.toString(Qt::ISODate)), KpKysely::DELETE);
    QObject::connect(poisto, &KpKysely::vastaus, kp(), &Kirjanpito::tilikausiAvattu);
    poisto->kysy();
}

QString TilikausiModel::tositeTunnus(int tunniste, const QDate &pvm, const QString &sarja, bool samakausi, bool vertailu) const
{
    if( vertailu)
        return QString("%1 %2 %3")
                .arg(sarja)
                .arg( tilikausiPaivalle(pvm).pitkakausitunnus() )
                .arg( tunniste, 8, 10, QChar('0')  );

    if( samakausi )
        return QString("%1%2")
                .arg(sarja)
                .arg(tunniste);

    return QString("%1%2/%3")
            .arg(sarja)
            .arg(tunniste)
            .arg( tilikausiPaivalle(pvm).kausitunnus() );
}


void TilikausiModel::paivita()
{
    KpKysely *kysely = kpk("/tilikaudet");
    connect( kysely, &KpKysely::vastaus, this, &TilikausiModel::lataaData );
    kysely->kysy();
}


void TilikausiModel::paivitaKausitunnukset()
{
    // Päivittää kausitunnukset. Kausitunnus on päättyvän tilikauden vuosiluku 17
    // paitsi jos kausia ko. vuodella on useita, niin 17B jne.
    QString edellinenvuosi;
    int samoja = 0;

    for(int i=0; i < kaudet_.count(); i++)
    {
        QString vuositxt = kaudet_.at(i).paattyy().toString("yyyy");
        if( vuositxt != edellinenvuosi)
        {
            samoja = 0;
            edellinenvuosi = vuositxt;
            kaudet_[i].asetaKausitunnus(vuositxt);
        }
        else
        {
            samoja++;
            kaudet_[i].asetaKausitunnus( vuositxt + QChar(65 + samoja) );
        }

    }
}

void TilikausiModel::lataaData(const QVariant *lista)
{
    lataa( lista->toList() );
}
