#ifndef ALVKAUDET_H
#define ALVKAUDET_H

#include <QObject>
#include <QDate>

class AlvKausi {
public:
    enum IlmoitusTila { PUUTTUVA, KASITELTY, KASITTELYSSA, ARVIOITU, ERAANTYNYT, EIKAUTTA  };
    enum KaudenPituus { KUUKAUSI, NELJANNES, VUOSI, EITIEDOSSA };

    AlvKausi();
    AlvKausi(const QVariantMap &map);

    QDate alkupvm() const { return alkupvm_;}
    QDate loppupvm() const { return loppupvm_;}
    QDate erapvm() const { return erapvm_;}
    IlmoitusTila tila() const { return tila_;}
    KaudenPituus pituus() const { return pituus_;}
    QString tilaInfo() const;

protected:
    QDate alkupvm_;
    QDate loppupvm_;
    QDate erapvm_;
    IlmoitusTila tila_;
    KaudenPituus pituus_;
};


class AlvKaudet : public QObject
{
    Q_OBJECT
public:
    enum VarmenneTila { EIKAYTOSSA, OK, VIRHE };

    explicit AlvKaudet(QObject *parent = nullptr);

    void haeKaudet();
    void tyhjenna();

    AlvKausi kausi(const QDate& date) const;
    QList<AlvKausi> kaudet() const;
    bool onko();
    QString virhe() const;

    static bool descSort(const AlvKausi &a, const AlvKausi& b);
    bool alvIlmoitusKaytossa() const { return varmenneTila == OK;}

signals:
    void haettu();

protected:
    void saapuu(QVariant* data);

private:
    QList<AlvKausi> kaudet_;
    QString errorCode_;

    int haussa_ = 0;
    VarmenneTila varmenneTila = EIKAYTOSSA;

};

#endif // ALVKAUDET_H
