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

#ifndef ASETUSMODEL_H
#define ASETUSMODEL_H

#include <QObject>
#include <QHash>
#include <QDateTime>
#include <QVariant>
#include <QColor>

/**
 * @brief Asetusten käsittely
 *
 * Asetukset tallennetaan välittömästi tietokantaan
 * Yleisimmille tietotyypeille on valmiiksi muunnosfunktiot
 *
 * Tyhjä arvo (tai nolla) tarkoittaa, että kyseinen asetus poistetaan
 *
 */
class AsetusModel : public QObject
{
    Q_OBJECT
public:
    explicit AsetusModel(QObject *parent);

    enum Asetukset {
        AlvVelvollinen /** AlvVelvollinen - Maksaa arvonlisäveroa */,
        EriSarjaan /** Tositteet numeroidaan sarjoittain */,
        KateisSarjaan /** Käteistositteet omaan sarjaansa */,
        OrganisaatioNimi /** Oman organisaation nimi */,
        Katuosoite /** Oma katuosoite */,
        Postinumero /** Oma postinumero */,
        Kaupunki /** Oma postitoimipaikka */,
        Aputoiminimi /** Laskulle tulostuva aputoiminimi */,
        Logonsijainti /** Logon sijainti */,
        LaskuIkkunaKorkeus,
        Puhelin,
        Email,
        Kotisivu,
        Kotipaikka,
        Ytunnus,
        LaskuIbanit,
        LaskuRF,
        LaskuQR,
        LaskuViivakoodi,
        LaskuVirtuaaliviivakoodi,
        LaskuTilisiirto,
        LaskuKateistili,
        LaskuEnnakkosaatavaTili,
        LaskuSaatavaTili,
        EnnakkoAlvTili,        
        LaskuMaksuaika,
        EmailSaate,
        LaskuPeruskorko,
        LaskuEnnakkoTili,
        UID,
        Muistiinpanot,
        AlvKausi,
        ArkistoRaportit,
        Muoto,
        Laajuus,
        Tositesarjat,
        Tilikartta,
        SmtpServer,
        SmtpPort,
        SmtpUser,
        SmtpPassword,
        EmailNimi,
        EmailOsoite,
        EmailKopio,
        EmailSSL,
        EmailMuoto,
        TilinpaatosValinnat,
        TilinpaatosKieli,
        VeroYhteysPuhelin,
        VeroYhteysHenkilo,
        Palkkatilit,
        OvtTunnnus,
        Operaattori,
        TilikarttaPvm,
        VakioTilikartta,
        FinvoiceKaytossa,
        VeroTuloViite,
        PalkkaFiTilit,
        Piilotilit,
        Suosikkitilit,
        Naytetaantilit,
        Laajuudet,
        OletusMenotili,
        LaskuRiviTyyppi,
        OletusMyyntitili,
        VariKehys,
        VariVarjo,
        LaskuKorttitili,
        LaskuLisatiedot,
        Toiminimet,
        TilitietoMaksuHyvaksytty,
        AvausKuukausittain,
        TilinAvausPvm,
        MaventaAutentikointiTila,
        VeroOmaViite,
        OhitaAlvLukko,
        AlvAlkaa
    };


    /**
     * @brief Palauttaa asetuksen annetulla avaimella
     * @param avain Haettava avain
     * @return asetus, tai String() jos asetusta ei ole
     */
    QString asetus(const QString& avain, const QString oletus = QString()) const;
    QString asetus(int avain, const QString& oletus = QString()) const;

    void aseta(const QString& avain, const QString& arvo);
    void aseta(const QVariantMap& map);

    void aseta(int tunnus, const QString& arvo);
    /**
     * @brief Poistaa asetuksen annetulla avaimella
     * @param avain
     */
    void poista(const QString& avain);
    void poista(int tunnus);

    QDate pvm(const QString& avain, const QDate oletus = QDate()) const;
    QDate pvm(int tunnus, const QDate& oletus = QDate()) const;

    void aseta(const QString &avain, const QDate& pvm);

    bool onko(const QString& avain) const;
    bool onko(int tunnus) const;
    void aseta(const QString& avain, bool totuusarvo);

    void asetaVar(const QString& avain, const QVariant& arvo);

    QStringList lista(const QString& avain) const;
    void aseta(const QString& avain, const QStringList& arvo);

    int luku(const QString& avain, int oletusarvo = 0) const;
    int luku(int tunnus, int oletusarvo = 0) const;

    QColor vari(int tunnus, const QColor oletus = QColor(Qt::darkGray));

    qulonglong isoluku(const QString &avain, qulonglong oletusarvo = 0) const;
    void aseta(const QString &avain, int luku);
    void aseta(int tunnus, int luku);
    void aseta(const QString& avain, qulonglong luku);

    /**
     * @brief Palauttaa listan avaimista, jotka alkavat annetulla alulla
     * @param avaimenAlku Teksti, jolla haettavat avaimet alkavat, tyhjä hakee kaikki
     * @return
     */
    QStringList avaimet(const QString& avaimenAlku = QString()) const;

    QStringList kielet() const;
    QString kieli(const QString& lyhenne) const;

    void lataa(const QVariantMap &lista);
    void tyhjenna() { asetukset_.clear(); }

    QString nimi() const { return asetus(AsetusModel::OrganisaatioNimi);}
    QString ytunnus() const { return asetus(AsetusModel::Ytunnus);}
    QString uid() const { return asetus(AsetusModel::UID);}

signals:
    void asetusMuuttui();


protected:
    QHash<QString,QString> asetukset_;

    static std::map<int,QString> avaimet__;


};

#endif // ASETUSMODEL_H
