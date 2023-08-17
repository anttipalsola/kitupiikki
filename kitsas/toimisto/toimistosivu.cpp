#include "toimistosivu.h"
#include "db/kirjanpito.h"
#include "db/kpkysely.h"
#include "grouptreemodel.h"
#include "groupdata.h"
#include "pilvi/pilvimodel.h"
#include "toimisto/groupuserdata.h"
#include "toimisto/groupusermembersmodel.h"
#include "uusitoimistodialog.h"
#include "bookdata.h"
#include "groupuserbooksmodel.h"
#include "groupmembersmodel.h"

#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QImage>

#include "ui_toimisto.h"
#include "uusikirjanpito/uusivelho.h"
#include "booknotificationmodel.h"
#include "ryhmaoikeusdialog.h"

#include "authlogmodel.h"
#include "groupbooksmodel.h"
#include "groupmembersmodel.h"

#include "pikavalintadialogi.h"
#include "kirjansiirtodialogi.h"
#include "alv/verovarmennetila.h"

#include "alv/varmennedialog.h"

#include "toimistokirjanpitodialogi.h"

#include "aloitussivu/kirjanpitodelegaatti.h"

#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QActionGroup>
#include <QWebEngineView>

ToimistoSivu::ToimistoSivu(QWidget *parent) :
    KitupiikkiSivu(parent),
    ui(new Ui::Toimisto()),
    groupTree_(new GroupTreeModel(this)),
    groupData_{new GroupData(this)},
    bookData_{new BookData(this)},
    userData_{new GroupUserData(this)},
    tuoteRyhma_{new QActionGroup(this)},
    tuoteMenu_{new QMenu(tr("Vaihda tuotetta"))},
    webView_{new QWebEngineView(this)},
    toimistoWidget_{new QWidget(this)}
{
    QHBoxLayout* leiska = new QHBoxLayout();
    ui->setupUi(toimistoWidget_);
    leiska->addWidget(toimistoWidget_);
    leiska->addWidget(webView_);
    // ui->setupUi(this);
    setLayout(leiska);

    ui->hakuList->hide();

    pikavalinnatAktio_ = new QAction(QIcon(":/pic/ratas.png"), tr("Pikavalinnat..."), this);

    muokkaaRyhmaAktio_ = new QAction(QIcon(":/pic/muokkaa.png"),tr("Muokkaa..."), this);
    poistaRyhmaAktio_ = new QAction(QIcon(":/pic/roskis.png"), tr("Poista"), this);

    siirraKirjaAktio_ = new QAction(QIcon(":/pic/siirra.png"),tr("Siirrä..."), this);
    poistaKirjaAktio_ = new QAction(QIcon(":/pic/roskis.png"),tr("Poista..."), this);
    tukiKirjautumisAktio_ = new QAction(QIcon(":/freeicons/adminkey.png"),tr("Tukikirjautuminen"), this);


    bookData_->setShortcuts(groupData_->shortcuts());

    treeSort_ = new QSortFilterProxyModel(this);
    treeSort_->setSourceModel(groupTree_);
    treeSort_->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->treeView->setModel(treeSort_);
    ui->treeView->setSortingEnabled(true);

    QSortFilterProxyModel *bookSort = new QSortFilterProxyModel(this);
    bookSort->setSourceModel(groupData_->books());
    bookSort->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->groupBooksView->setModel(bookSort);
    ui->groupBooksView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->groupBooksView->setItemDelegateForColumn(GroupBooksModel::NIMI, new KirjanpitoDelegaatti(this));

    QSortFilterProxyModel *memberSort = new QSortFilterProxyModel(this);
    memberSort->setSourceModel(groupData_->members());
    memberSort->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->groupMembersView->setModel(memberSort);
    ui->groupMembersView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    QSortFilterProxyModel* duSort = new QSortFilterProxyModel(this);
    duSort->setSourceModel( bookData_->directUsers() );
    duSort->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->bKayttajatView->setModel(duSort);
    ui->bKayttajatView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    QSortFilterProxyModel* guSort = new QSortFilterProxyModel(this);
    guSort->setSourceModel( bookData_->groupUsers() );
    ui->bRyhmaView->setModel(guSort);
    ui->bRyhmaView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->bLokiView->setModel( bookData_->authLog() );
    ui->bLokiView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->udView->setModel( userData_->books() );
    ui->umView->setModel( userData_->members());
    ui->gbooksView->setModel( userData_->groupBooks() );

    ui->notifyView->setModel( bookData_->notificatios() );

    connect( ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged,
             this, &ToimistoSivu::nodeValittu);
    connect( ui->treeView, &QTreeView::clicked, this, &ToimistoSivu::nodeValittu);

    connect( ui->groupBooksView->selectionModel(), &QItemSelectionModel::currentRowChanged,
             this, &ToimistoSivu::kirjaValittu);

    connect( ui->groupMembersView->selectionModel(), &QItemSelectionModel::currentRowChanged,
             this, &ToimistoSivu::kayttajaValittu);

    connect( ui->bKayttajatView->selectionModel(), &QItemSelectionModel::currentRowChanged,
             this, &ToimistoSivu::kirjanKayttajaValittu);

    connect( groupData_, &GroupData::loaded, this, &ToimistoSivu::toimistoVaihtui);
    connect( groupTree_, &GroupTreeModel::modelReset, this, [this] { this->treeSort_->sort(0); });

    connect( bookData_, &BookData::loaded, this, &ToimistoSivu::kirjaVaihtui);
    connect( groupData_->books(), &GroupBooksModel::modelReset, this, [bookSort] { bookSort->sort(0);});

    connect( groupData_->members(), &GroupMembersModel::modelReset, this, [memberSort] { memberSort->sort(0);});

    connect( bookData_->directUsers(), &GroupMembersModel::modelReset, this, [duSort] { duSort->sort(0); });

    connect( bookData_->groupUsers(), &GroupMembersModel::modelReset, this, [guSort] { guSort->sort(0);});

    connect( ui->uusiRyhmaNappi, &QPushButton::clicked, this, &ToimistoSivu::lisaaRyhma);
    connect( ui->uusiToimistoNappi, &QPushButton::clicked, this, &ToimistoSivu::lisaaToimisto);

    connect( ui->uusiKayttajaNappi, &QPushButton::clicked, this, &ToimistoSivu::uusiKayttajaRyhmaan);
    connect( ui->uMuokkaaNappi, &QPushButton::clicked, this, &ToimistoSivu::muokkaaRyhmaOikeuksia);
    connect( ui->uPoistaNappi, &QPushButton::clicked, this, [this] { if(this->groupData_) this->groupData_->deleteMembership(this->userInfo_.userid()); });

    connect( ui->uusiKirjanpitoNappi, &QPushButton::clicked, this, &ToimistoSivu::uusiKirjanpito);

    connect( ui->bAvaaNappi, &QPushButton::clicked, bookData_, &BookData::openBook);
    connect( ui->pUusiNappi, &QPushButton::clicked, this, &ToimistoSivu::lisaaOikeus);
    connect( ui->pMuokkaaNappi, &QPushButton::clicked, this, &ToimistoSivu::muokkaaOikeus);
    connect( ui->pPoistaNappi, &QPushButton::clicked, this, &ToimistoSivu::poistaOikeus);    

    connect( ui->varmenneNappi, &QPushButton::clicked, this, &ToimistoSivu::lisaaVarmenne);
    connect( ui->poistaVarmenneNappi, &QPushButton::clicked, this, &ToimistoSivu::poistaVarmenne);

    connect( ui->mainTab, &QTabWidget::currentChanged, this, &ToimistoSivu::paaTabVaihtui);

    toimistoVaihtui();

    connect( groupTree_, &GroupTreeModel::dataChanged, groupData_, &GroupData::reload);
    connect( pikavalinnatAktio_, &QAction::triggered, this, &ToimistoSivu::pikavalinnat);
    connect( poistaRyhmaAktio_, &QAction::triggered, this, [this]
        { this->groupTree_->remove( this->groupData_->id() ); });
    connect( muokkaaRyhmaAktio_, &QAction::triggered, this, &ToimistoSivu::muokkaaRyhma);
    connect( siirraKirjaAktio_, &QAction::triggered, this, &ToimistoSivu::siirraKirjanpito);
    connect( poistaKirjaAktio_, &QAction::triggered, this, &ToimistoSivu::poistaKirjanpito);
    connect( tukiKirjautumisAktio_, &QAction::triggered, bookData_, &BookData::supportLogin);

    connect( userData_, &GroupUserData::loaded, this, &ToimistoSivu::kayttajaLadattu);

    connect( ui->hakuEdit, &QLineEdit::textEdited, this, &ToimistoSivu::haku);
    connect( ui->hakuList, &QListWidget::itemClicked, this, &ToimistoSivu::hakuValittu);

    QMenu* ryhmaMenu = new QMenu(this);
    ryhmaMenu->addAction(pikavalinnatAktio_);
    ryhmaMenu->addAction(muokkaaRyhmaAktio_);
    ryhmaMenu->addAction(poistaRyhmaAktio_);
    ui->ryhmaMenuNappi->setMenu(ryhmaMenu);

    QMenu* kirjaMenu = new QMenu(this);
    tuoteMenu_->setIcon(QIcon(":/freeicons/timantti.svg"));
    kirjaMenu->addMenu(tuoteMenu_);
    kirjaMenu->addAction(siirraKirjaAktio_);
    kirjaMenu->addAction(poistaKirjaAktio_);
    kirjaMenu->addAction(tukiKirjautumisAktio_);
    ui->bMenuNappi->setMenu(kirjaMenu);

}

ToimistoSivu::~ToimistoSivu()
{
    delete ui;
}

void ToimistoSivu::siirrySivulle()
{
    // TODO: Onko optiona suoraan siirto ???
    if( 1 == 1) {
        webView_->show();
        toimistoWidget_->hide();
        webView_->load(QUrl("https://possu.kitsas.fi"));
    } else {
        webView_->hide();
        toimistoWidget_->show();
        if( ui->treeView->selectionModel()->selectedIndexes().isEmpty() ) {
            if( groupTree_->nodes() < 50) {
                ui->treeView->expandAll();
            }
            if( ui->treeView->model()->rowCount()==1) {
                ui->treeView->selectionModel()->select( groupTree_->index(0,0), QItemSelectionModel::SelectCurrent );
                nodeValittu( groupTree_->index(0,0) );
            }
        } else {
            groupData_->reload();
        }
    }
}

void ToimistoSivu::nodeValittu(const QModelIndex &index)
{
    groupData_->load( index.data(GroupTreeModel::IdRole).toInt());
}

void ToimistoSivu::kirjaValittu(const QModelIndex &index)
{
    vaihdaLohko( KIRJANPITOLOHKO );
    kirjanKayttajaValittu(QModelIndex());
    bookData_->load(index.data(GroupBooksModel::IdRooli).toInt());
}

void ToimistoSivu::kayttajaValittu(const QModelIndex &index)
{    
    vaihdaLohko( KAYTTAJALOHKO );
    userInfo_ = groupData_->members()->getMember( index.data(GroupMembersModel::IdRooli).toInt() );
    // Näytettäisiinkö tässä käyttäjä nimi, yhteystiedot ja oikeudet
    // nykyisessä kirjanpidossa?
    ui->uNimi->setText( userInfo_.name());
    ui->uInfo->setText( QString("%1\n%2").arg(userInfo_.email(), userInfo_.phone()) );
    ui->uBrowser->setHtml( userInfo_.oikeusInfo( groupData_->isUnit() ) );

    ui->uMuokkaaNappi->setEnabled( userInfo_.userid() );
    ui->uPoistaNappi->setEnabled( userInfo_.userid());

    userData_->load( userInfo_.userid() );
}

void ToimistoSivu::kirjanKayttajaValittu(const QModelIndex &index)
{
    ui->pMuokkaaNappi->setEnabled( index.data(GroupMembersModel::IdRooli).toInt() );
    ui->pPoistaNappi->setEnabled(index.data(GroupMembersModel::IdRooli).toInt() );
}

void ToimistoSivu::vaihdaLohko(Lohko lohko)
{
    const QStringList oikeudet = groupData_->adminRights();

    ui->subTab->setTabVisible( RYHMATAB, lohko == RYHMALOHKO );
    ui->subTab->setTabVisible( KAYTTAJATAB, lohko == KAYTTAJALOHKO );
    ui->subTab->setTabVisible( KIRJANPITO_TIEDOT, lohko == KIRJANPITOLOHKO );
    ui->subTab->setTabVisible( KIRJANPITO_SUORAT, lohko == KIRJANPITOLOHKO && oikeudet.contains("OP"));
    ui->subTab->setTabVisible( KIRJANPITO_RYHMAT, lohko == KIRJANPITOLOHKO && oikeudet.contains("OM"));
    ui->subTab->setTabVisible( KIRJANPITO_LOKI, lohko == KIRJANPITOLOHKO && oikeudet.contains("OL"));
    ui->subTab->setTabVisible( OIKEUDET_SUORAT, false);
    ui->subTab->setTabVisible( OIKEUDET_RYHMAT, false);
    ui->subTab->setTabVisible( KIRJAT_RYHMAT, false);
}

void ToimistoSivu::paaTabVaihtui(int tab)
{
    if( tab == PAA_KIRJANPIDOT && ui->groupBooksView->currentIndex().isValid())
        ui->subTab->setCurrentIndex( KIRJANPITO_TIEDOT );
    else if( tab == PAA_JASENET && ui->groupMembersView->currentIndex().isValid())
        ui->subTab->setCurrentIndex( KAYTTAJATAB );
    else
        ui->subTab->setCurrentIndex( RYHMATAB );
}

void ToimistoSivu::toimistoVaihtui(int bookId)
{
    vaihdaLohko( RYHMALOHKO );
    const QStringList oikeudet = groupData_->adminRights();
    ui->mainTab->setTabVisible( PAA_JASENET, oikeudet.contains("OM") );

    ui->uusiRyhmaNappi->setVisible(oikeudet.contains("OG"));
    ui->uusiRyhmaNappi->setIcon( groupData_->isUnit() ? QIcon(":/pic/folder.png") : QIcon(":/pic/kansiot.png") );
    ui->uusiToimistoNappi->setVisible( groupData_->isUnit() && oikeudet.contains("OO"));
    ui->uusiKirjanpitoNappi->setVisible( !groupData_->isUnit() && oikeudet.contains("OB")  );

    ui->toimistoNimiLabel->setText(  groupData_->name() );
    ui->toimistoYtunnusLabel->setText( groupData_->isOffice() ? groupData_->businessId() : groupData_->officeName() );
    ui->talousverkkoLabel->setVisible( groupData_->officeType() == "Talousverkko" );

    ui->varmenneRyhma->setVisible( groupData_->isOffice() );
    ui->varmenneInfo->setText(groupData_->varmenneTila()->toString() );

    bool varmenneOikeus = oikeudet.contains("OC") &&
            !groupData_->businessId().isEmpty() &&
            groupData_->varmenneTila()->status() != "PG";   // Pending

    ui->varmenneNappi->setVisible(!groupData_->varmenneTila()->isValid() && varmenneOikeus);
    ui->poistaVarmenneNappi->setVisible(groupData_->varmenneTila()->isValid() && varmenneOikeus);

    ui->ryhmaMenuNappi->setVisible( oikeudet.contains("OG"));

    GroupNode* node = groupTree_->nodeById( groupData_->id() );
    poistaRyhmaAktio_->setEnabled( groupData_->books()->rowCount() == 0 &&
                                   node && node->subGroupsCount() == 0);
    tuoteMenu_->clear();
    for(QAction* action : tuoteRyhma_->actions()) {
        tuoteRyhma_->removeAction(action);
        delete action;
    }
    for( const auto& item : groupData_->products()) {
        const QVariantMap& map = item.toMap();
        QAction* tuoteAktio = new QAction( map.value("name").toString() );
        tuoteAktio->setData( map.value("id") );
        tuoteAktio->setCheckable(true);
        tuoteAktio->setActionGroup(tuoteRyhma_);
        connect( tuoteAktio, &QAction::triggered, this, &ToimistoSivu::vaihdaTuote);
        tuoteMenu_->addAction(tuoteAktio);
    }

    if(bookId) {
        for(int i=0; i < ui->groupBooksView->model()->rowCount(); i++) {
            const QModelIndex& bookIndex = ui->groupBooksView->model()->index(i,0);
            if( bookIndex.data(GroupBooksModel::IdRooli).toInt() == bookId) {
                ui->groupBooksView->selectRow(i);
                break;
            }
        }
    } else {
        kirjaVaihtui();
    }
}

void ToimistoSivu::kirjaVaihtui()
{    
    ui->bNimi->setText( bookData_->companyName() );
    ui->bYtunnus->setText( bookData_->businessId());

    const QImage scaled = bookData_->logo().scaledToHeight(64, Qt::SmoothTransformation);
    ui->logo->setVisible(!scaled.isNull());
    ui->logo->setPixmap(QPixmap::fromImage(scaled));

    ui->bHarjoitus->setVisible( bookData_->trial() );
    ui->alustamatonLabel->setVisible( !bookData_->initialized() );

    ui->bLuotu->setText( bookData_->created().toString("dd.MM.yyyy") );
    ui->bMuokattu->setText( bookData_->modified().toString("dd.MM.yyyy"));
    ui->bTositteita->setText( QString("%1").arg( bookData_->documents() ) );
    ui->bKoko->setText( bookData_->prettySize() );

    const QString& varmenneTila = bookData_->certInfo();
    ui->bVarmenneLabel->setVisible(!varmenneTila.isEmpty());
    ui->bVarmenne->setVisible(!varmenneTila.isEmpty());
    ui->bVarmenne->setText( varmenneTila );

    ui->bDealLabel->setVisible( !bookData_->dealOfficeName().isEmpty() );
    ui->bDealText->setVisible( !bookData_->dealOfficeName().isEmpty());
    ui->bDealText->setText( bookData_->dealOfficeName() );

    if( bookData_->ownerName().isEmpty()) {
        ui->bTuoteLabel->setText(tr("Tuote"));
        ui->bTuote->setText( bookData_->planName() );
    } else {
        ui->bTuoteLabel->setText(tr("Omistaja"));
        ui->bTuote->setText( bookData_->ownerName());
    }

    const QString& alvInfo = bookData_->vatString();
    ui->bAlvLabel->setVisible( !alvInfo.isEmpty());
    ui->bAlvText->setVisible( !alvInfo.isEmpty());
    ui->bAlvText->setText( alvInfo );

    ui->bAlvDueLabel->setVisible( bookData_->vatDueDate().isValid() );
    ui->bAlvDueText->setVisible( bookData_->vatDueDate().isValid());
    ui->bAlvDueText->setText( bookData_->vatDueDate().toString("dd.MM.yyyy"));

    ui->bAvaaNappi->setVisible( bookData_->loginAvailable() );


    const bool luontiOikeus = groupData_->adminRights().contains("OB");
    const bool muokkausOikeus = groupData_->adminRights().contains("OD");
    const bool tukiKirjautumisOikeus = groupData_->adminRights().contains("OS");

    ui->bMenuNappi->setVisible( bookData_->id() && (luontiOikeus || muokkausOikeus || tukiKirjautumisOikeus) );
    tuoteMenu_->setEnabled( muokkausOikeus );
    siirraKirjaAktio_->setEnabled( muokkausOikeus );
    poistaKirjaAktio_->setEnabled( muokkausOikeus );
    tukiKirjautumisAktio_->setEnabled( tukiKirjautumisOikeus );

    for(QAction *action : tuoteRyhma_->actions()) {
        if( action->data().toInt() == bookData_->planId()) {
            action->setChecked(true);
        }
    }

}


void ToimistoSivu::lisaaRyhma()
{
    const QString nimi = QInputDialog::getText(this, tr("Uusi ryhmä"), tr("Ryhmän nimi"));
    if( !nimi.isEmpty()) {
        QVariantMap payload;
        payload.insert("name", nimi);
        groupTree_->addGroup( groupData_->id(), payload );
    }
}

void ToimistoSivu::muokkaaRyhma()
{
    if( groupData_->isOffice() ) {
        UusiToimistoDialog dlg(this);
        dlg.editOffice(groupTree_, groupData_);
    } else {
        const QString nimi = QInputDialog::getText(this, tr("Muokkaa ryhmää"), tr("Ryhmän nimi"), QLineEdit::Normal, groupData_->name());
        if( !nimi.isEmpty()) {
            QVariantMap payload;
            payload.insert("name", nimi);
            groupTree_->edit( groupData_->id(), payload);
        }
    }
}

void ToimistoSivu::lisaaToimisto()
{
    UusiToimistoDialog dlg(this);
    dlg.newOffice(groupTree_, groupData_);
}

void ToimistoSivu::lisaaOikeus()
{
    RyhmaOikeusDialog dlg(this, groupData_);
    dlg.lisaa(bookData_);
    bookData_->reload();
}

void ToimistoSivu::muokkaaOikeus()
{
    int userid = ui->bKayttajatView->currentIndex().data(GroupMembersModel::IdRooli).toInt();
    if( userid ) {
        const GroupMember member = bookData_->directUsers()->getMember(userid);
        RyhmaOikeusDialog dlg(this, groupData_);
        dlg.muokkaa(member, bookData_);
        bookData_->reload();
    }
}

void ToimistoSivu::poistaOikeus()
{
    int userid = ui->bKayttajatView->currentIndex().data(GroupMembersModel::IdRooli).toInt();
    if( userid) {
        bookData_->removeRights(userid);
        bookData_->reload();
    }
}

void ToimistoSivu::lisaaVarmenne()
{
    VarmenneDialog dlg(this);
    dlg.toimistoVarmenne(groupData_);
}

void ToimistoSivu::poistaVarmenne()
{
    groupData_->poistaVarmenne();
}

void ToimistoSivu::muokkaaRyhmaOikeuksia()
{
    if( userInfo_.userid() ) {
        RyhmaOikeusDialog dlg(this, groupData_);
        dlg.muokkaa(userInfo_);
        groupData_->reload();
    }
}

void ToimistoSivu::uusiKayttajaRyhmaan()
{
    RyhmaOikeusDialog dlg(this, groupData_);
    dlg.lisaa();
    groupData_->reload();

}

void ToimistoSivu::uusiKirjanpito()
{
    ToimistoKirjanpitoDialogi dlg(this, groupData_);
    dlg.exec();

    /**
    UusiVelho velho(this);
    if( velho.toimistoVelho(groupData_)) {
        QVariantMap map = velho.data();
        groupData_->addBook(map);
    }
    */
}

void ToimistoSivu::vaihdaTuote()
{
    int plan = tuoteRyhma_->checkedAction()->data().toInt();
    if( plan && plan != bookData_->planId()) {
        groupData_->books()->changePlan(bookData_->id(), tuoteRyhma_->checkedAction()->text());
        bookData_->changePlan(plan);
    }
}

void ToimistoSivu::poistaKirjanpito()
{
    if( QMessageBox::question(this, tr("Kirjanpidon poistaminen"),
                              tr("Haluatko todellakin poistaa kirjanpidon %1 pysyvästi?").arg(bookData_->companyName())) == QMessageBox::Yes) {

        groupData_->deleteBook( bookData_->id() );
    }
}

void ToimistoSivu::siirraKirjanpito()
{
    KirjanSiirtoDialogi dlg(this);
    dlg.siirra(bookData_->id(), groupTree_, groupData_);
}

void ToimistoSivu::kayttajaLadattu()
{
    vaihdaLohko( KAYTTAJALOHKO );
    ui->subTab->setTabVisible( OIKEUDET_SUORAT, userData_->books()->rowCount());
    ui->subTab->setTabVisible( OIKEUDET_RYHMAT, userData_->members()->rowCount());
    ui->subTab->setTabVisible( KIRJAT_RYHMAT, userData_->groupBooks()->rowCount());

    if( userData_->id() != userInfo_.userid()) {
        ui->uNimi->setText( userData_->name());
        ui->uInfo->setText( QString("%1\n%2").arg(userData_->email(), userData_->phone()) );
        ui->uBrowser->clear();
    }

}

void ToimistoSivu::pikavalinnat()
{
    PikavalintaDialogi dlg(this, groupData_);
    dlg.exec();
}

void ToimistoSivu::haku(const QString &teksti)
{
    if( teksti.length() < 3) {
        ui->hakuList->hide();
    } else {
        KpKysely* kysely = kp()->pilvi()->loginKysely("/groups");
        kysely->lisaaAttribuutti("search", teksti);
        connect( kysely, &KpKysely::vastaus, this, &ToimistoSivu::hakuSaapuu);
        kysely->kysy();
    }
}

void ToimistoSivu::hakuSaapuu(QVariant *data)
{
    QVariantMap map = data->toMap();
    ui->hakuList->clear();

    QVariantList books = map.value("books").toList();
    for(const auto& item : qAsConst(books)) {
        QVariantMap book = item.toMap();
        QListWidgetItem* li = new QListWidgetItem(book.value("name").toString(), ui->hakuList);
        li->setData(TYYPPIROOLI, KIRJANPITO);
        li->setData(IDROOLI, book.value("id").toInt());
        li->setData(RYHMAROOLI, book.value("groupid").toInt());
        QByteArray ba = QByteArray::fromBase64( book.value("logo").toByteArray() );
        if( ba.isEmpty() )
            li->setIcon(QIcon(":/pic/tyhja.png"));
        else
            li->setIcon( QIcon(  QPixmap::fromImage( QImage::fromData(ba) )) );
    }
    QVariantList groups = map.value("groups").toList();
    for(const auto& item: qAsConst(groups)) {
        QVariantMap group = item.toMap();
        QListWidgetItem* li = new QListWidgetItem(group.value("name").toString(), ui->hakuList);
        li->setData(TYYPPIROOLI, RYHMA);
        li->setData(RYHMAROOLI, group.value("id"));
        const QString type = group.value("type").toString();
        if( type == "UNIT") li->setIcon(QIcon(":/pic/folder.png"));
        else if(type == "OFFICE") li->setIcon(QIcon(":/pic/pixaby/toimisto.svg"));
        else if(type == "GROUP") li->setIcon(QIcon(":/pic/kansiot.png"));

    }
    QVariantList users = map.value("users").toList();
    for(const auto& item: qAsConst(users)) {
        QVariantMap user = item.toMap();
        QListWidgetItem* li = new QListWidgetItem(user.value("name").toString(), ui->hakuList);
        li->setData(TYYPPIROOLI, KAYTTAJA);
        li->setData(IDROOLI, user.value("id").toInt());
        li->setIcon(QIcon(":/pic/mies.png"));
    }

    ui->hakuList->setVisible( ui->hakuList->count() );
}

void ToimistoSivu::hakuValittu(QListWidgetItem *item)
{    
    const int id = item->data(IDROOLI).toInt();
    const int ryhma = item->data(RYHMAROOLI).toInt();

    if( ryhma ) {
        groupData_->load(ryhma, id);
        const QModelIndex& index = groupTree_->indexById(ryhma);
        const QModelIndex& mapped = treeSort_->mapFromSource(index);

        qDebug() << " RYHMÄ " << index.data(Qt::DisplayRole).toString();
        ui->treeView->setExpanded(mapped, true);
        ui->treeView->selectionModel()->select(mapped, QItemSelectionModel::SelectCurrent);
    } else {
        ui->bKayttajatView->selectionModel()->clearSelection();
        userData_->load(id);
    }

}


