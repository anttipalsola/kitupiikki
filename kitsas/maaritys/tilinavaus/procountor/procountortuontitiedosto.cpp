#include "procountortuontitiedosto.h"
#include "tuonti/csvtuonti.h"

#include <QFile>

#include "db/kirjanpito.h"
#include "model/tositevienti.h"
#include "model/tositeviennit.h"

#include "procountortuontidialog.h"

ProcountorTuontiTiedosto::ProcountorTuontiTiedosto()
{

}

ProcountorTuontiTiedosto::TuontiStatus ProcountorTuontiTiedosto::tuo(const QString &polku)
{
    QFile file(polku);

    if( !file.open( QIODevice::ReadOnly | QIODevice::Text)) {
        return TIEDOSTO_VIRHE;
    }

    QByteArray data = file.readAll();
    file.close();

    QList<QStringList> csv = Tuonti::CsvTuonti::csvListana(data);

    tyyppi_ = TASE;

    for(const QStringList& rivi : qAsConst(csv)) {
        if( rivi.length() < 2 ) continue;

        // Haetaan päivämäärälista
        if(paivat_.isEmpty() && rivi.at(0).isEmpty() && paivaksi(rivi.at(1)).isValid()) {
            QList<QDate> paivalista;
            for(int i=1; i < rivi.length(); i++) {
                QDate paiva = paivaksi(rivi.at(i));
                if( !paiva.isValid() || paivalista.contains(paiva))
                    break;
                paivat_ << paiva;
            }           
            continue;
        }
        if( paivat_.isEmpty()) continue;

        const QString tilistr = rivi.value(0);
        QRegularExpressionMatch mats = tiliRE__.match(tilistr);
        if( mats.hasMatch()) {
            const QString tili = mats.captured(1);
            const QString nimi = mats.captured(2);
            QList<Euro> saldot;

            for(int i=1; i < rivi.length() && i <= paivat_.length(); i++) {
                saldot << Euro::fromString(rivi.at(i));
            }
            SaldoTieto tieto = SaldoTieto(tili, nimi, saldot);

            if( tili > '3') tyyppi_ = TULOSLASKELMA;
            if( tili.startsWith("2") && nimi.contains("Edellisten tilikausien") && tieto.summa())
                oikaistavaTase_ = true;  // Tase on avattu, koska siinä edellisten tilikausien tulos

            saldot_.append(tieto);
        }
    }

    if( paivat_.isEmpty()) return PAIVAT_PUUTTUU;

    // Tsekataan tilikaudet
    const TilikausiModel* kaudet = kp()->tilikaudet();
    if( kaudet->rowCount() < 2) return ProcountorTuontiTiedosto::KAUDET_PUUTTUU;

    if( paivat_.last() == kaudet->tilikausiIndeksilla(0).paattyy()) {
        kausi_ = EDELLINEN;        
    } else if( paivat_.last() == kaudet->tilikausiIndeksilla(1).paattyy())
        kausi_ = TAMA;
    else {
        return VIRHEELLISET_KAUDET;
    }

    if( !validi())
        return TIEDOSTO_TUNTEMATON;

    return TUONTI_OK;
}

void ProcountorTuontiTiedosto::lisaaMuuntoon(TiliMuuntoModel *muunto)
{
    for(const auto& saldo : saldot_) {
        muunto->lisaa(saldo.tilinumero(), saldo.tilinimi(), saldo.summa());
    }
}

bool ProcountorTuontiTiedosto::validi()
{
    return tyyppi_ && kausi_;
}

void ProcountorTuontiTiedosto::tallennaTilinavaukseen(TilinavausModel *tilinavaus, TiliMuuntoModel *muunto)
{
    if( paivat_.count() > 1)
        tilinavaus->setKuukausittain(true);

    for(const auto& saldo : saldot_) {
        const int muunnettu = muunto->muunnettu(saldo.tilinumero());
        if( !muunnettu) continue;

        QList<AvausEra> erat;
        QList<AvausEra> vanhatErat = tilinavaus->erat(muunnettu);

        for(int i=0; i < paivat_.count(); i++) {
            const QDate& pvm = paivat_.at(i);
            const Euro kkSaldo = saldo.saldot().value(i) + vanhatErat.value(i).saldo();

            AvausEra era(kkSaldo, pvm);
            erat << era;
        }

        tilinavaus->asetaErat(muunnettu, erat);
    }

}

void ProcountorTuontiTiedosto::oikaiseTilinavaus(const ProcountorTuontiTiedosto &edellinenTase)
{
    for(const auto& edellinenSaldo : edellinenTase.saldot_) {
        oikaiseTili(edellinenSaldo);
    }
}

void ProcountorTuontiTiedosto::oikaiseTili(const SaldoTieto &saldotieto)
{
    for(int i=0; i < saldot_.count(); i++) {
        if( saldotieto.tilinumero() == saldot_.at(i).tilinumero()) {
            saldot_[i].oikaiseAvauksesta(saldotieto.summa());
            return;
        }
    }
    // Tämä saldo on nollattu, eli nollaaminen on lisättävä saldoihin
    QList<Euro> nollaus;
    nollaus << Euro::Zero - saldotieto.summa();
    for(int i=1; i < paivat_.count(); i++)
        nollaus << Euro::Zero;
    saldot_.append(SaldoTieto(saldotieto.tili(), saldotieto.tilinimi(), nollaus));
}

void ProcountorTuontiTiedosto::oikaiseEdellinenTulos(const Euro &euroa, TiliMuuntoModel *muunto)
{
    for(int i=0; i < saldot_.count(); i++) {
        const int muunnettu = muunto->muunnettu(saldot_.at(i).tilinumero());
        Tili* tili = kp()->tilit()->tili(muunnettu);
        if( tili && tili->onko(TiliLaji::EDELLISTENTULOS)) {
            saldot_[i].oikaiseAvauksesta(euroa);
            return;
        }
    }
}

void ProcountorTuontiTiedosto::tallennaAlkutositteeseen(Tosite *tosite, TiliMuuntoModel *muunto)
{
    for( const auto& saldo : saldot_) {
        const int muunnettu = muunto->muunnettu(saldo.tilinumero());
        if( !muunnettu ) continue;

        Tili* tili = kp()->tilit()->tili(muunnettu);
        if(!tili) continue;

        for(int i=0; i < paivat_.count(); i++) {
            const Euro& maara = saldo.saldot().value(i);
            if( !maara ) continue;

            TositeVienti vienti;
            vienti.setPvm( paivat_.at(i) );
            vienti.setTili( muunnettu );
            vienti.setSelite( ProcountorTuontiDialog::tr("Procountorista %3 %1 %2").arg(saldo.tilinumero()).arg(saldo.tilinimi()).arg(paivat_.at(i).toString("MM/yyyy"))  );

            if( tili->onko(TiliLaji::VASTAAVAA) ^ (maara < Euro::Zero)) {
                vienti.setDebet(maara.abs());
            } else {
                vienti.setKredit(maara.abs());
            }
            tosite->viennit()->lisaa(vienti);
        }
    }
}

Euro ProcountorTuontiTiedosto::tiedostoSumma() const
{
    Euro summa;
    for( const auto& saldo : saldot_) {
        if( saldo.tili().startsWith('1'))
            summa -= saldo.summa();
        else
            summa += saldo.summa();
    }
    return summa;
}

QDate ProcountorTuontiTiedosto::paivaksi(const QString &teksti)
{
    // Mahdolliset päivämäärämuodot 31.12.2023 TAI 12/2023

    QRegularExpressionMatchIterator matsi = pvmRE__.globalMatch(teksti);
    if( matsi.hasNext() ) {
        QRegularExpressionMatch mats;
        while(matsi.hasNext()) mats = matsi.next();
        return QDate::fromString(mats.captured(), "dd.MM.yyyy");
    }

    matsi = kausiRE__.globalMatch(teksti);
    if( matsi.hasNext() ) {
        QRegularExpressionMatch mats;
        while(matsi.hasNext()) mats = matsi.next();
        const int kk = mats.captured(1).toInt();
        const int vvvv = mats.captured(2).toInt();
        const QDate eka(vvvv, kk, 1);
        return QDate(vvvv, kk, eka.daysInMonth());
    }

    return QDate();
}

QRegularExpression ProcountorTuontiTiedosto::tiliRE__ = QRegularExpression(R"(\D*(\d{3,8})\W*(.+))", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression ProcountorTuontiTiedosto::pvmRE__ = QRegularExpression(R"([0-3]?\d[.][01]\d[.]2\d{3})", QRegularExpression::UseUnicodePropertiesOption);
QRegularExpression ProcountorTuontiTiedosto::kausiRE__ = QRegularExpression(R"((1?\d)\s*/\s*(2\d{3}))", QRegularExpression::UseUnicodePropertiesOption);

ProcountorTuontiTiedosto::SaldoTieto::SaldoTieto()
{

}

ProcountorTuontiTiedosto::SaldoTieto::SaldoTieto(const QString& tilinumero, const QString &tilinimi, const QList<Euro> &saldot) :
    tilinumero_(tilinumero), tilinimi_(tilinimi), saldot_(saldot)
{

}

Euro ProcountorTuontiTiedosto::SaldoTieto::summa() const
{
    Euro yhteensa = Euro::Zero;
    for(const auto& euro : saldot_) {
        yhteensa += euro;
    }
    return yhteensa;
}

void ProcountorTuontiTiedosto::SaldoTieto::oikaiseAvauksesta(const Euro &euro)
{
    if( saldot_.empty()) return;
    saldot_[0] = saldot_.at(0) - euro;
}
