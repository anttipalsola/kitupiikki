#include "procountortuontidialog.h"
#include "ui_procountortuontidialog.h"

#include "db/kirjanpito.h"
#include <QFileDialog>
#include <QPushButton>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

#include "tuonti/tilimuuntomodel.h"

#include "model/tosite.h"
#include "model/tositeviennit.h"
#include "db/tositetyyppimodel.h"

#include <QJsonDocument>
#include <QMessageBox>

ProcountorTuontiDialog::ProcountorTuontiDialog(QWidget *parent, TilinavausModel* tilinavaus) :
    QDialog(parent), ui(new Ui::ProcountorTuontiDialog),
    tilinavaus_(tilinavaus)
{
    ui->setupUi(this);

    const TilikausiModel* tilikaudet = kp()->tilikaudet();
    if( tilikaudet->rowCount() > 1) {
        ui->edKausiLabel->setText( tilikaudet->tilikausiIndeksilla(0).kausivaliTekstina() );
        ui->nykyKausiLabel->setText( tilikaudet->tilikausiIndeksilla(1).kausivaliTekstina() );
    }

    paivitaRuksit();

    connect( ui->valitseTiedostoNappi, &QPushButton::clicked, this, &ProcountorTuontiDialog::avaaTiedosto);
    connect( ui->buttonBox, &QDialogButtonBox::accepted, this, &ProcountorTuontiDialog::accept);
    connect( ui->buttonBox, &QDialogButtonBox::rejected, this, &ProcountorTuontiDialog::reject);
    connect( ui->buttonBox, &QDialogButtonBox::helpRequested, this, [] { kp()->ohje("asetukset/tilinavaus/procountor"); });

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    setAcceptDrops(true);
}

ProcountorTuontiDialog::~ProcountorTuontiDialog()
{
    delete ui;
}

void ProcountorTuontiDialog::avaaTiedosto()
{
    QString tiedosto = QFileDialog::getOpenFileName(this, tr("Tuo alkusaldot tiedostosta"),
                                                    QString(), tr("CSV-tiedostot (*.csv);;Kaikki tiedostot(*.*)"));
    if(!tiedosto.isEmpty())
        tuoTiedosto(tiedosto);

}

bool ProcountorTuontiDialog::tuoTiedosto(const QString &tiedostonnimi)
{
    ProcountorTuontiTiedosto tuotu;
    const ProcountorTuontiTiedosto::TuontiStatus status = tuotu.tuo(tiedostonnimi);

    if( status == ProcountorTuontiTiedosto::TUONTI_OK) {
        if( onkoJo(tuotu)) {
            // Ilmoita virheestä
            QString tyyppiTeksti = tuotu.tyyppi() && ProcountorTuontiTiedosto::TASE ? tr("Tase") : tr("Tuloslaskelma");
            QString kausiTeksti = tuotu.kausi() && ProcountorTuontiTiedosto::EDELLINEN ? tr("edellinen") : tr("nykyinen");

            QMessageBox::critical(this, tr("Procountor-tuonti"), tr("Samankaltainen avaustiedosto (%2 %3 tilikausi) on jo lisätty. Tätä tiedostoa %1 ei käytetä.").arg(tiedostonnimi, tyyppiTeksti, kausiTeksti));
            return false;
        }

        if( (tuotu.tyyppi() & ProcountorTuontiTiedosto::TASE) && tuotu.kausi() == ProcountorTuontiTiedosto::EDELLINEN)
            taseEdellinen_ = tuotu;
        else if( tuotu.tyyppi() == ProcountorTuontiTiedosto::TULOSLASKELMA && tuotu.kausi() == ProcountorTuontiTiedosto::EDELLINEN)
            tulosEdellinen_ = tuotu;
        else if( (tuotu.tyyppi() & ProcountorTuontiTiedosto::TASE) && tuotu.kausi() == ProcountorTuontiTiedosto::TAMA) {
            taseNykyinen_ = tuotu;
            ui->oikaiseTaseCheck->setChecked( tuotu.oikaistavaTase() );
        } else if( (tuotu.tyyppi() == ProcountorTuontiTiedosto::TULOSLASKELMA) && tuotu.kausi() == ProcountorTuontiTiedosto::TAMA)
            tulosNykyinen_ = tuotu;

        const bool kelpaa =
            (taseEdellinen_.validi() && tulosEdellinen_.validi() && taseNykyinen_.validi() && tulosNykyinen_.validi()) ||
            (taseEdellinen_.validi() && tulosEdellinen_.validi() && !taseNykyinen_.validi() && !tulosNykyinen_.validi());

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(kelpaa);


        paivitaRuksit();
        return true;
    } else if( status == ProcountorTuontiTiedosto::TIEDOSTO_TUNTEMATON)
        QMessageBox::critical(this, tr("Procountor-tuonti"), tr("Tiedosto %1 ei ole muodoltaan oikeanlainen. Muodosta tuontitiedostot Procountorissa ohjeen mukaisesti.").arg(tiedostonnimi));
    else if( status == ProcountorTuontiTiedosto::VIRHEELLISET_KAUDET)
        QMessageBox::critical(this, tr("Procountor-tuonti"), tr("Tiedostossa %1 olevat tilikaudet eivät vastaa Kitsaassa määriteltyjä kahta ensimmäistä tilikautta.").arg(tiedostonnimi));
    else if( status == ProcountorTuontiTiedosto::TIEDOSTO_VIRHE)
        QMessageBox::critical(this, tr("Procountor-tuonti"), tr("Tiedoston %1 lukeminen ei onnistunut tiedostovirheen takia.").arg(tiedostonnimi));
    else if( status == ProcountorTuontiTiedosto::PAIVAT_PUUTTUU)
        QMessageBox::critical(this, tr("Procountor-tuonti"), tr("Tiedosto %1 ei ole muodoltaan oikea - päivämääriä ei löydetty.").arg(tiedostonnimi));


    return false;
}

bool ProcountorTuontiDialog::onkoJo(const ProcountorTuontiTiedosto &t)
{
    return
        ( ( t.tyyppi() & ProcountorTuontiTiedosto::TASE ) && ( t.kausi() == ProcountorTuontiTiedosto::EDELLINEN) && taseEdellinen_.validi() ) ||
        ( ( t.tyyppi() == ProcountorTuontiTiedosto::TULOSLASKELMA) && ( t.kausi() == ProcountorTuontiTiedosto::EDELLINEN) && tulosEdellinen_.validi()) ||
        ( ( t.tyyppi() & ProcountorTuontiTiedosto::TASE) && (t.kausi() == ProcountorTuontiTiedosto::TAMA) && taseNykyinen_.validi()) ||
        ( ( t.tyyppi() == ProcountorTuontiTiedosto::TULOSLASKELMA) && (t.kausi() == ProcountorTuontiTiedosto::TAMA) && tulosNykyinen_.validi() ) ;
}

void ProcountorTuontiDialog::paivitaRuksit()
{
    ui->edTaseCheck->setVisible( taseEdellinen_.validi() );
    ui->edTulosCheck->setVisible( tulosEdellinen_.validi() );

    ui->edellinenEiTasmaa->setVisible( taseEdellinen_.validi() && tulosEdellinen_.validi() && ( taseEdellinen_.tiedostoSumma() + tulosEdellinen_.tiedostoSumma() ) );

    ui->nykTaseCheck->setVisible( taseNykyinen_.validi() );
    ui->nykTulosCheck->setVisible( tulosNykyinen_.validi() );

    ui->oikaiseTaseCheck->setEnabled( taseNykyinen_.validi() );

    ui->nykyinenEiTasmaa->setVisible( taseNykyinen_.validi() && tulosNykyinen_.validi() && ( taseNykyinen_.tiedostoSumma() + tulosNykyinen_.tiedostoSumma()) );
}


void ProcountorTuontiDialog::dragEnterEvent(QDragEnterEvent *event)
{
    if( event->mimeData()->hasUrls() )
    {
        for( const QUrl& url: event->mimeData()->urls())
        {
            if(url.isLocalFile())
                event->accept();
        }
    }
}

void ProcountorTuontiDialog::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void ProcountorTuontiDialog::dropEvent(QDropEvent *event)
{
    if( event->mimeData()->hasUrls())
    {
        QList<QUrl> urlit = event->mimeData()->urls();
        foreach (QUrl url, urlit)
        {
            if( url.isLocalFile())
            {
                tuoTiedosto(url.toLocalFile());
            }
        }
    }

}

void ProcountorTuontiDialog::accept()
{
    TiliMuuntoModel muunto;
    muunto.asetaMoodi(TiliMuuntoModel::TILINAVAUS_NAYTA_SALDO);

    taseEdellinen_.lisaaMuuntoon(&muunto);
    tulosEdellinen_.lisaaMuuntoon(&muunto);
    taseNykyinen_.lisaaMuuntoon(&muunto);
    tulosNykyinen_.lisaaMuuntoon(&muunto);

    if(!muunto.naytaMuuntoDialogi(this)) return;

    // Sitten pitäisi tehdä itse tallennus
    taseEdellinen_.tallennaTilinavaukseen(tilinavaus_, &muunto);
    tulosEdellinen_.tallennaTilinavaukseen(tilinavaus_, &muunto);

    if( taseNykyinen_.validi()) {

        if( ui->oikaiseTaseCheck->isChecked()) {
            taseNykyinen_.oikaiseTilinavaus(taseEdellinen_);
            taseNykyinen_.oikaiseEdellinenTulos( tulosEdellinen_.tiedostoSumma(), &muunto );
        }

        Tosite kausitosite;

        const QDate avausPvm = kp()->tilikaudet()->tilikausiIndeksilla(1).alkaa();
        kausitosite.asetaPvm( avausPvm );
        kausitosite.asetaTyyppi( TositeTyyppi::TILINAVAUS);
        kausitosite.asetaOtsikko(tr("Tilinavaus Procountorista (keskeneräinen tilikausi)"));

        taseNykyinen_.tallennaAlkutositteeseen(&kausitosite, &muunto);
        tulosNykyinen_.tallennaAlkutositteeseen(&kausitosite, &muunto);


        QJsonDocument json = QJsonDocument::fromVariant(kausitosite.tallennettava());
        qDebug() << json.toJson(QJsonDocument::Compact);

        if( kausitosite.viennit()->debetKreditTasmaa()) {
            kausitosite.tallenna();
            QMessageBox::information(this, tr("Aloitussaldojen tuonti"),
                                     tr("Keskeneräisen tilikauden saldoista on muodostettu tosite päivämäärällä %1").arg(avausPvm.toString("dd.MM.yyyy")));
        } else {
            kausitosite.tallenna(Tosite::LUONNOS);
            QMessageBox::information(this, tr("Aloitussaldojen tuonti"),
                                     tr("Keskeneräisen tilikauden saldoista on muodostettu tosite päivämäärällä %1").arg(avausPvm.toString("dd.MM.yyyy")) +
                                     "\n" +
                                     tr("Näiden saldojen debet ja kredit ei täsmää, joten tosite löytyy Selaa-toiminnon Luonnokset-välilehdeltä."));
        }



    }

    QDialog::accept();

}
