#ifndef PILVIKAYTTAJA_H
#define PILVIKAYTTAJA_H

#include <QVariant>
#include <QDate>

#include "model/euro.h"
#include "palvelunkertoja.h"

class PilviKayttaja : public PalvelunKertoja
{
public:
    enum KayttajaMoodi {
        NORMAALI,
        PRO
    };

    enum Suljettu {
        KAYTOSSA = 0,
        MAKSAMATON = 1,
        EHTOJEN_VASTAINEN = 2,
        KAYTTAJAN_PYYNNOSTA = 3,
        MUU_SYY = 99
    };

    PilviKayttaja();    
    PilviKayttaja(const QVariant& data);

    operator bool() const;

    int id() const { return id_; }
    QString uuid() const { return uuid_;}
    QString nimi() const { return nimi_;}
    QString email() const { return email_; }
    QString phone() const { return phone_;}

    int planId() const { return plan_id_; }
    QString planName() const { return plan_name_; }

    QDate trialPeriod() const { return trial_; }
    bool trialActive() const;

    KayttajaMoodi moodi() const { return moodi_;}
    Suljettu suljettu() const { return blocked_;}

    int cloudCount() const { return cloudCount_;}
    Euro extraMonthly() const { return extraMonthly_; }
    int capacity() const { return capacity_; }

    bool admin() const { return admin_; }    

    bool with2FA() const { return with2FA_;}

    static void asetaVersioMoodi(const KayttajaMoodi versio);

private:
    int id_ = 0;
    QString uuid_;
    QString nimi_;
    QString email_;
    QString phone_;

    int plan_id_ = 0;
    QString plan_name_;

    QDate trial_;
    bool admin_ = false;

    KayttajaMoodi moodi_ = NORMAALI;
    Suljettu blocked_ = KAYTOSSA;    

    int cloudCount_ = 0;
    int capacity_ = 0;
    Euro extraMonthly_;

    bool with2FA_ = false;

    static KayttajaMoodi versio__;
};

#endif // PILVIKAYTTAJA_H
