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

#ifndef TILITYYPPIMODEL_H
#define TILITYYPPIMODEL_H
#include <QAbstractListModel>
#include <QList>

namespace TiliLaji {
    enum TiliLuonne
    {
        TUNTEMATON         =   0,
        OTSIKKO         =   0b0000000000001,
        TASE            =   0b0000000000010,
        TULOS           =   0b0000000000100,

        VASTAAVAA        =  0b0000000001010,
        VASTATTAVAA      =  0b0000000010010,
        POISTETTAVA     =   0b0000000101010,
        SAATAVA        =    0b0000001001010,
        RAHAVARAT       =   0b0000010001010,

        TASAERAPOISTO    =  0b0000100101010,
        MENOJAANNOSPOISTO=  0b0001000101010,

        ALVSAATAVA     =    0b0000101001010,
  KOHDENTAMATONALVSAATAVA = 0b0001101001010,
        VEROSAATAVA    =    0b0001001001010,

        MYYNTISAATAVA  =    0b0010001001010,
        SIIRTOSAATAVA  =    0b0100001001010,

        PANKKITILI      =   0b0001010001010,
        KATEINEN        =   0b0010010001010,

        OMAPAAOMA       =   0b0000000110010,
        EDELLISTENTULOS =   0b0001000110010,
        KAUDENTULOS     =   0b0010000110010,
        VIERASPAAOMA    =   0b0000001010010,
        VELKA           =   0b0000011010010,
        VELKATILI       =   0b0001011010010,
        ALVVELKA        =   0b0010011010010,
 KOHDENTAMATONALVVELKA  =   0b0110011010010,
        VEROVELKA       =   0b0100011010010,
        OSTOVELKA       =   0b0000111010010,
        SIIRTOVELKA     =   0b0001011010010,

        TULO            =   0b0000000010100,
        LVTULO          =   0b0000001010100,
        MENO            =   0b0000000100100,
        POISTO          =   0b0000010100100
    };

}

/**
 * @brief Tilityypin kuvaus
 */
class TiliTyyppi
{
public:
    TiliTyyppi(int otsikkotaso);
    TiliTyyppi(QString tyyppikoodi=QString(), QString kuvaus=QString(), TiliLaji::TiliLuonne luonne=TiliLaji::TUNTEMATON, bool uniikki = false);

    QString koodi() const { return tyyppikoodi_; }
    QString kuvaus() const { return kuvaus_; }
    TiliLaji::TiliLuonne luonne() const { return luonne_; }
    int otsikkotaso() const { return otsikkotaso_; }
    /**
     * @brief Edustaako tämä tili kysyttyä "luonnetta"
     * @param luonnetta Luonne
     *
     * Esimerkiksi ALV-saatavien tili edustaa edustaa myös luonteita
     * SAAMINEN, VASTAAVA ja TASE
     *
     * @return
     */
    bool onko(TiliLaji::TiliLuonne luonnetta) const;

    bool onkoUniikki() const { return uniikki_;}

protected:
    QString tyyppikoodi_;
    QString kuvaus_;
    TiliLaji::TiliLuonne     luonne_;
    int otsikkotaso_ = -1;
    bool uniikki_ = false;
};

/**
 * @brief Valittavissa olevien tilityyppien model
 */
class TilityyppiModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum
    {
        KoodiRooli = Qt::UserRole,
        KuvausRooli = Qt::UserRole + 2,
        LuonneRooli = Qt::UserRole + 3,
        UniikkiRooli = Qt::UserRole + 4
    };

    TilityyppiModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    TiliTyyppi tyyppiKoodilla(const QString &koodi);
protected:
    void lisaa(const TiliTyyppi &tyyppi);
    QList<TiliTyyppi> tyypit;
};

#endif // TILITYYPPIMODEL_H
