/*
   Copyright (C) 2017,2018 Arto Hyvättinen

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

#include <QRect>
#include <QPainter>
#include <QFile>
#include <QFont>
#include <QPixmap>
#include <QSettings>
#include <QApplication>
#include <QBuffer>
#include "raportinkirjoittaja.h"

#include <QPdfWriter>

#include "db/kirjanpito.h"

#include <QDebug>

RaportinKirjoittaja::RaportinKirjoittaja(bool csvKaytossa) :
    csvKaytossa_( csvKaytossa)
{

}

void RaportinKirjoittaja::asetaOtsikko(const QString &otsikko)
{
    otsikko_ = otsikko;
}

void RaportinKirjoittaja::asetaKausiteksti(const QString &kausiteksti)
{
    kausiteksti_ = kausiteksti;
}

void RaportinKirjoittaja::asetaCsvKaytossa(const bool onko)
{
    csvKaytossa_ = onko;
}

void RaportinKirjoittaja::lisaaSarake(const QString &leveysteksti, RaporttiRivi::RivinKaytto kaytto)
{
    RaporttiSarake uusi;
    uusi.leveysteksti = leveysteksti;
    uusi.sarakkeenKaytto = kaytto;
    sarakkeet_.append(uusi);
}

void RaportinKirjoittaja::lisaaSarake(int leveysprosentti)
{
    RaporttiSarake uusi;
    uusi.leveysprossa = leveysprosentti;
    sarakkeet_.append(uusi);
}

void RaportinKirjoittaja::lisaaVenyvaSarake(int tekija, RaporttiRivi::RivinKaytto kaytto, const QString &leveysteksti)
{
    RaporttiSarake uusi;
    uusi.jakotekija = tekija;
    uusi.sarakkeenKaytto = kaytto;
    uusi.leveysteksti = leveysteksti;
    sarakkeet_.append(uusi);
}

void RaportinKirjoittaja::lisaaEurosarake()
{
    lisaaSarake("-9 999 999,99€XX");
}

void RaportinKirjoittaja::lisaaPvmSarake()
{
    lisaaSarake("99.99.9999XX");
}

void RaportinKirjoittaja::lisaaOtsake(const RaporttiRivi& otsikkorivi)
{
    otsakkeet_.append(otsikkorivi);
}

void RaportinKirjoittaja::lisaaRivi(const RaporttiRivi& rivi)
{
    rivit_.append(rivi);
}

void RaportinKirjoittaja::lisaaTyhjaRivi()
{
    if( rivit_.count())
        if( rivit_.last().sarakkeita() )
            rivit_.append( RaporttiRivi(RaporttiRivi::EICSV));
}

void RaportinKirjoittaja::lisaaSivunvaihto()
{
    rivit_.append(RaporttiRivi(RaporttiRivi::SIVUNVAIHTO));
}


bool RaportinKirjoittaja::mahtuukoSivulle( QPainter* painter, int sivunLeveys) const {

    int leveys = painter->fontMetrics().horizontalAdvance( "X" ) * sarakkeet_.count();

    for( int i=0; i < sarakkeet_.count(); i++)
    {

        if( !(sarakkeet_[i].sarakkeenKaytto & RaporttiRivi::PDF))
            continue;
        else if( !sarakkeet_[i].leveysteksti.isEmpty())
            leveys += painter->fontMetrics().horizontalAdvance( sarakkeet_[i].leveysteksti );
        else if( sarakkeet_[i].leveysprossa )
            leveys += sivunLeveys * sarakkeet_[i].leveysprossa / 100;
        else
            leveys += painter->fontMetrics().horizontalAdvance( "XXXXXXXXXX" );
    }
    return sivunLeveys > leveys;
}

int RaportinKirjoittaja::tulosta(QPagedPaintDevice *printer, QPainter *painter, bool raidoita, int alkusivunumero, bool naytaPaivamaara) const
{
    if( rivit_.isEmpty()) {
        tulostaYlatunniste( painter,alkusivunumero, naytaPaivamaara);
        return 1;     // Ei tulostettavaa !
    }

    const double mm = printer->width() * 1.00 / printer->widthMM();
    const int sivunleveys = painter->window().width();

    int pienennys = 0;
    QFont fontti;

    while( pienennys < 8) {
        fontti = QFont("FreeSans", 10 - pienennys );
        painter->setFont(fontti);
        if( mahtuukoSivulle(painter, sivunleveys)) break;
        pienennys++;
    }

    QFont koodifontti( "code128_XL", 36);
    koodifontti.setLetterSpacing(QFont::AbsoluteSpacing, 0.0);


    int rivinkorkeus = painter->fontMetrics().height();
    int sivunkorkeus = painter->window().height();
    const int sisennysMetrics = painter->fontMetrics().horizontalAdvance("XX");

    // Lasketaan sarakkeiden leveydet
    QVector<int> leveydet( sarakkeet_.count() );

    int tekijayhteensa = 0; // Lasketaan jäävän tilan jako
    int jaljella = sivunleveys - sarakkeet_.count() * mm;

    for( int i=0; i < sarakkeet_.count(); i++)
    {
       int leveys = 0;

       if( !(sarakkeet_[i].sarakkeenKaytto & RaporttiRivi::PDF))
           leveys = 0;
       else if( sarakkeet_[i].jakotekija)
           tekijayhteensa += sarakkeet_[i].jakotekija;
       else if( sarakkeet_[i].leveysprossa)
            leveys = sivunleveys * sarakkeet_[i].leveysprossa / 100;
       else if( !sarakkeet_[i].leveysteksti.isEmpty())
           leveys = painter->fontMetrics().horizontalAdvance( sarakkeet_[i].leveysteksti );                  

       leveydet[i] = leveys;
       jaljella -= leveys;

    }

    // Jaetaan vielä jäljellä oleva tila
    for( int i=0; i<sarakkeet_.count(); i++)
    {
        if( sarakkeet_[i].jakotekija && tekijayhteensa)
        {
            leveydet[i] = jaljella * sarakkeet_[i].jakotekija / tekijayhteensa;
        }
    }

    if( tekijayhteensa )
        jaljella = 0;   // Koko tila käytetty venyvällä sarakkeella

    // Nyt taulukosta löytyy sarakkeiden leveydet, ja tulostaminen
    // voidaan aloittaa

    int sivu = 1;
    int rivilla = 0;

    foreach (RaporttiRivi rivi, rivit_)
    {
        if( !(rivi.kaytto() & RaporttiRivi::PDF))
            continue;

        if( rivi.kaytto() == RaporttiRivi::VIIVAKOODI) {
            painter->setFont(koodifontti);
        } else {
            fontti.setPointSize( rivi.pistekoko() - pienennys );
            fontti.setBold( rivi.onkoLihava() );
            painter->setFont(fontti);
        }

        // Lasketaan ensin sarakkeiden rectit
        // ja samalla lasketaan taulukkoon liput

        QVector<QRect> laatikot( rivi.sarakkeita() );
        QVector<int> liput( rivi.sarakkeita() );
        QVector<QString> tekstit( rivi.sarakkeita() );

        int korkeinrivi = rivinkorkeus;
        int x = 0;  // Missä kohtaa ollaan leveyssuunnassa
        int sarake = 0; // Missä taulukon sarakkeessa ollaan menossa

        for(int i=0; i < rivi.sarakkeita(); i++)
        {

            int sarakeleveys = 0;
            // Korjataan tarvittaessa sisennyksen verran
            if( sarake == 0) {
                sarakeleveys = 0 - rivi.sisennys() * sisennysMetrics;
                x += rivi.sisennys() * sisennysMetrics;
            }

            // ysind (Yhdistettyjen Sarakkeiden Indeksi) kelaa ne sarakkeet läpi,
            // jotka tällä riville yhdistetty toisiinsa
            for( int ysind = 0; ysind < rivi.leveysSaraketta(i); ysind++ )
            {
                sarakeleveys += leveydet.value(sarake);
                sarake++;
                if(ysind)
                    sarakeleveys += mm;
            }

            // Nyt saatu tämän sarakkeen leveys

            int lippu = Qt::TextWordWrap;
            QString teksti = rivi.teksti(i);
            if( rivi.tasattuOikealle(i))
            {
                lippu |= Qt::AlignRight;
                teksti.append("  ");
                // Ei tasata ihan oikealle vaan välilyönnin päähän
            }
            tekstit[i] = teksti;

            liput[i] = lippu;
            // Laatikoita ei asemoida korkeussuunnassa, vaan translatella liikutaan
            laatikot[i] = sarakeleveys ? painter->boundingRect( x, 0,
                                                sarakeleveys, sivunkorkeus,
                                                lippu, teksti )
                                       : QRect();

            x += sarakeleveys + mm;
            if( laatikot[i].height() > korkeinrivi )
                korkeinrivi = laatikot[i].height();
        }

        if( painter->transform().dy() > sivunkorkeus - korkeinrivi || rivi.kaytto() == RaporttiRivi::SIVUNVAIHTO)
        {
            // Sivu tulee täyteen
            printer->newPage();
            painter->resetTransform();
            sivu++;
            rivilla = 0;            
        }

        if( painter->transform().dy() < 0.1 )
        {
            // Ollaan sivun alussa

            painter->save();
            painter->setFont(QFont("FreeSans", 10 - pienennys));

            // Tulostetaan ylätunniste
            if( !otsikko_.isEmpty())
                tulostaYlatunniste( painter, sivu + alkusivunumero - 1, naytaPaivamaara);

            if( !otsakkeet_.isEmpty())
                painter->translate(0, rivinkorkeus);

            // Otsikkorivit
            foreach (RaporttiRivi otsikkorivi, otsakkeet_)
            {
                if( otsikkorivi.kaytto() == RaporttiRivi::CSV)
                    continue;

                x = 0;
                sarake = 0;

                for( int i = 0; i < otsikkorivi.sarakkeita(); i++)
                {

                    int lippu = 0;
                    QString teksti = otsikkorivi.teksti(i);

                    if( otsikkorivi.tasattuOikealle(i))
                    {
                        lippu = Qt::AlignRight;
                        teksti.append("  ");
                    }
                    int sarakeleveys = 0;

                    for( int ysind = 0; ysind < otsikkorivi.leveysSaraketta(i); ysind++ )
                    {
                        sarakeleveys += leveydet[sarake];
                        sarake++;
                        if(ysind)
                            sarakeleveys += mm;
                    }

                    if( sarakeleveys)
                        painter->drawText( QRect(x,0,sarakeleveys,rivinkorkeus),
                                      lippu, teksti );

                    x += sarakeleveys + mm;
                }
                painter->translate(0, rivinkorkeus);
            } // Otsikkorivi
            if( !otsikko_.isEmpty() || !otsakkeet_.isEmpty())
                painter->drawLine(0,0,sivunleveys,0);
        }

        // Jos raidoitus, niin raidoitetaan eli osan rivien taakse harmaata
        if( raidoita && rivilla % 6 > 2)
        {
            painter->save();
            painter->setBrush(QBrush(QColor(222,222,222)));
            painter->setPen(Qt::NoPen);

            painter->drawRect(0,0,sivunleveys, korkeinrivi);

            painter->restore();

        }

        if( rivi.kaytto() == RaporttiRivi::VIIVAKOODI) {
            painter->setFont(koodifontti);
        } else {
            fontti.setPointSize( rivi.pistekoko() - pienennys );
            fontti.setBold( rivi.onkoLihava() );
            painter->setFont(fontti);
        }

        // Sitten tulostetaan tämä varsinainen rivi
        for( int i=0; i < rivi.sarakkeita(); i++)
        {
            if( !laatikot[i].isEmpty())
                painter->drawText( laatikot[i], liput[i] , tekstit[i] );
        }
        if( rivi.onkoViivaa())  // Viivan tulostaminen rivin ylle
        {
            painter->drawLine(0,0, sivunleveys - jaljella , 0);
        }

        painter->translate(0, korkeinrivi);
        rivilla++;
    }

    painter->restore();

    return sivu;
}

QString RaportinKirjoittaja::html(bool linkit) const
{
    QString txt;

    txt.append("<html><meta charset=\"utf-8\"><title>");
    txt.append( otsikko() );
    txt.append("</title>"
               "<style>"
               " body { font-family: Helvetica; }"
               " h1 { font-weight: normal; }"
               " .lihava { font-weight: bold; } "
               " tr {vertical-align: top;}"
               " tr.viiva td { border-top: 1px solid black; }"
               " td.oikealle { text-align: right; } "
               " th { text-align: left; color: darkgray;}"
               " a { text-decoration: none; color: black; }"
               " td { padding-right: 2em; }"
               " td:last-of-type { padding-right: 0; }"
               " table { border-collapse: collapse;}"
               " p.tulostettu { margin-top:2em; color: darkgray; }"
               " span.treeni { color: green; }"
               "</style>"
               "</head><body>");

    txt.append("<h1>" + otsikko() + "</h1>");
    txt.append("<p>" + kp()->asetukset()->asetus(AsetusModel::OrganisaatioNimi) + "<br>");
    txt.append( kausiteksti() + "</p>");
    txt.append("<table width=100%><thead>\n");

    // Otsikkorivit
    foreach (RaporttiRivi otsikkorivi, otsakkeet_ )
    {        
        if( otsikkorivi.kaytto() == RaporttiRivi::CSV)
            continue;

        txt.append("<tr>");
        int sarakkeessa = 0;
        for(int i=0; i < otsikkorivi.sarakkeita(); i++)
        {
            if( sarakkeet_.value(sarakkeessa).sarakkeenKaytto & RaporttiRivi::HTML) {
                txt.append(QString("<th colspan=%1>").arg( otsikkorivi.leveysSaraketta(i)));                
                txt.append( otsikkorivi.teksti(i));
                txt.append("</th>");
            }
            sarakkeessa += otsikkorivi.leveysSaraketta(i);
        }
        txt.append("</tr>\n");
    }

    txt.append("</thead>\n");
    // Rivit
    foreach (RaporttiRivi rivi, rivit_)
    {
        if( !(rivi.kaytto() & RaporttiRivi::HTML) )
            continue;

        QStringList trluokat;
        if( rivi.onkoLihava())
            trluokat << "lihava";
        if( rivi.onkoViivaa())
            trluokat << "viiva";

        if( trluokat.isEmpty())
            txt.append("<tr>");
        else
            txt.append("<tr class=\"" + trluokat.join(' ') + "\">");

        if( !rivi.sarakkeita())
            txt.append("<td>&nbsp;</td>"); // Tyhjätkin rivit näkyviin!

        int sarakkeessa = 0;
        for(int i=0; i < rivi.sarakkeita(); i++)
        {
            if( sarakkeet_.value(sarakkeessa).sarakkeenKaytto & RaporttiRivi::HTML) {

                // Vähentää colspanista näkymättömät sarakkeet
                int leveysSaraketta = rivi.leveysSaraketta(i);

                for(int li=sarakkeessa; li < sarakkeessa + rivi.leveysSaraketta(i); li++) {
                    if( !(sarakkeet_.value(li).sarakkeenKaytto & RaporttiRivi::HTML) ) {
                        leveysSaraketta--;
                    }
                }

                txt.append("<td");
                if(leveysSaraketta > 1) txt.append(QString(" colspan=%1").arg(leveysSaraketta));
                if(rivi.tasattuOikealle(i)) txt.append(" class=oikealle");
                txt.append("> ");


                if(linkit)
                {
                    if( rivi.sarake(i).linkkityyppi == RaporttiRiviSarake::TOSITE_ID)
                    {
                        // Linkki tositteeseen
                        txt.append( QString("<a href=\"../tositteet/%1.html\">").arg( rivi.sarake(i).linkkidata));
                    }
                    else if( rivi.sarake(i).linkkityyppi == RaporttiRiviSarake::TILI_NRO)
                    {
                        // Linkki tiliin
                        txt.append( QString("<a href=\"../raportit/paakirja.html#%2\">").arg( rivi.sarake(i).linkkidata));
                    }
                    else if( rivi.sarake(i).linkkityyppi == RaporttiRiviSarake::TILI_LINKKI)
                    {
                        // Nimiö dataan
                        txt.append( QString("<a name=\"%1\">").arg( rivi.sarake(i).linkkidata));
                    }
                }

                if( i == 0) {
                    for(int s=0; s < rivi.sisennys() * 2; s++) {
                        txt.append("&nbsp;");
                    }
                }

                QString tekstia = rivi.teksti(i).toHtmlEscaped();


                // Jotta selitteestä ei ole kohtuuttoman pitkä ja toisaalta
                // pvm-selite katkeile, ollaan valmiita katkomaan selitettä
                if( !sarakkeet_.value(sarakkeessa).jakotekija)
                    tekstia.replace(' ', "&nbsp;");
                tekstia.replace('\n', "<br>");

                txt.append(  tekstia );

                if( linkit && rivi.sarake(i).linkkityyppi )
                    txt.append("</a>");

                txt.append("&nbsp;</td>");
            }
            sarakkeessa += rivi.leveysSaraketta(i);
        }
        txt.append("</tr>\n");


    }
    txt.append("</table>");
    txt.append("<p class=tulostettu>" + kaanna("Tulostettu") + " " + QDate::currentDate().toString("dd.MM.yyyy"));
    if( kp()->onkoHarjoitus())
        txt.append("<br><span class=treeni>" + kaanna("Kirjanpito on laadittu Kitsas-ohjelman harjoittelutilassa") + "</span>");

    txt.append("</p></body></html>\n");

    return txt;
}

QByteArray RaportinKirjoittaja::pdf(bool taustaraidat, bool tulostaA4, QPageLayout *leiska) const
{
    QByteArray array;
    QBuffer buffer(&array);
    buffer.open(QIODevice::WriteOnly);

    QPdfWriter writer(&buffer);

    writer.setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);
    writer.setCreator( QString("Kitsas %1").arg( qApp->applicationVersion() ) );
    writer.setTitle( otsikko() );

    if( tulostaA4 )
        writer.setPageSize( QPageSize(QPageSize::A4) );
    else if( leiska ) {
        writer.setPageLayout(*leiska);
    } else
        writer.setPageLayout( kp()->printer()->pageLayout() );        

    QPainter painter( &writer );

    tulosta( &writer, &painter, taustaraidat );
    painter.end();

    return array;

}

QByteArray RaportinKirjoittaja::csv() const
{
    QChar erotin = kp()->settings()->value("CsvErotin", QChar(',')).toChar();

    QString txt;

    for( RaporttiRivi otsikko : otsakkeet_)
    {
        if( !(otsikko.kaytto() & RaporttiRivi::CSV))
            continue;

        QStringList otsakkeet;
        for(int i=0; i < otsikko.sarakkeita(); i++) {
            if( !(sarakkeet_.value(i).sarakkeenKaytto & RaporttiRivi::CSV))
                continue;
            otsakkeet.append( otsikko.csv(i));
        }
        txt.append( otsakkeet.join(erotin));
    }
    for( RaporttiRivi rivi : rivit_ )
    {
        if( !(rivi.kaytto() & RaporttiRivi::CSV))
            continue;

        if( rivi.sarakkeita() )
        {

            txt.append("\r\n");
            QStringList sarakkeet;
            for( int i=0; i < rivi.sarakkeita(); i++)
            {
                if( sarakkeet_.value(i).sarakkeenKaytto & RaporttiRivi::CSV) {
                    sarakkeet.append( rivi.csv(i));
                }
            }
            txt.append( sarakkeet.join(erotin));
        }
    }

    if( kp()->settings()->value("CsvKoodaus").toString() == "latin1")
    {
        txt.replace("€","EUR");
        return txt.toLatin1();
    }
    else
        return txt.toUtf8();
}

void RaportinKirjoittaja::tulostaYlatunniste(QPainter *painter, int sivu, bool naytaPaivamaara) const
{

    int sivunleveys = painter->window().width();
    int rivinkorkeus = painter->fontMetrics().height();

    QString nimi = kp()->asetukset()->asetus(AsetusModel::Logonsijainti) == "VAINLOGO" ?
                QString() : kp()->asetukset()->nimi();
    QString paivays = kp()->paivamaara().toString("dd.MM.yyyy");

    int vasenreunus = 0;
    QRect logoRect;

    if( !kp()->logo().isNull() )
    {
        double skaala = ((double) kp()->logo().width()) / kp()->logo().height();
        double skaalattu = skaala < 5.0 ? skaala : 5.0;
        logoRect = QRect(0, 0, rivinkorkeus * 2 * skaalattu, rivinkorkeus * 2);
        painter->drawPixmap( logoRect, QPixmap::fromImage( kp()->logo() ) );
        vasenreunus = rivinkorkeus * 2 * skaalattu + painter->fontMetrics().horizontalAdvance("A");
    }

    QRectF nimiRect = painter->boundingRect( vasenreunus, 0, sivunleveys / 3 - vasenreunus, painter->viewport().height(),
                                             Qt::TextWordWrap, nimi );

    QString ytunnus = kp()->asetukset()->ytunnus() ;
    QRectF ytunnusRect = painter->boundingRect( vasenreunus, nimiRect.height(), sivunleveys / 3, rivinkorkeus, Qt::AlignLeft, ytunnus );

    QRectF otsikkoRect = painter->boundingRect( sivunleveys/3, 0, sivunleveys / 3, painter->viewport().height(),
                                                Qt::AlignHCenter | Qt::TextWordWrap, otsikko());
    if( ytunnusRect.right() + painter->fontMetrics().horizontalAdvance("A") > otsikkoRect.left() )
        otsikkoRect.moveTo( ytunnusRect.right() + painter->fontMetrics().horizontalAdvance("A"), 0 );


    painter->drawText( nimiRect, Qt::AlignLeft | Qt::TextWordWrap, nimi );
    painter->drawText(ytunnusRect, Qt::AlignLeft, ytunnus);
    painter->drawText( otsikkoRect, Qt::AlignHCenter | Qt::TextWordWrap, otsikko());

    if( naytaPaivamaara ) {
        painter->drawText( QRect(sivunleveys*2/3, 0, sivunleveys/3, rivinkorkeus), Qt::AlignRight, paivays);
    } else if( sivu ) {
        painter->drawText(QRect(sivunleveys*2/3, 0, sivunleveys/3, rivinkorkeus), Qt::AlignRight, kaanna("Sivu %1").arg(sivu));
    }

    if( kp()->asetukset()->onko("Harjoitus") && !kp()->asetukset()->onko("Demo") )
    {
        painter->save();
        painter->setPen( QPen(Qt::green));
        painter->setFont( QFont("FreeSans",14));
        painter->drawText(QRect(sivunleveys / 8 * 5,0,sivunleveys/4, rivinkorkeus*2 ), Qt::AlignHCenter | Qt::AlignVCenter, kaanna("HARJOITUS") );
        painter->restore();
    }

    painter->translate(0, nimiRect.height() > otsikkoRect.height() ? nimiRect.height() : otsikkoRect.height() );
    // Alempi rivi : Kausiteksti ja päivämäärä

    painter->drawText(QRect(sivunleveys/3, 0, sivunleveys/3, rivinkorkeus  ), Qt::AlignHCenter, kausiteksti_);
    if( sivu && naytaPaivamaara)    // Jos päivämäärä ylemmällä rivillä, tulee sivunumero alemmalle
        painter->drawText(QRect(sivunleveys*2/3, 0, sivunleveys/3, rivinkorkeus), Qt::AlignRight, kaanna("Sivu %1").arg(sivu));

    painter->translate(0, rivinkorkeus );
    painter->setPen(QPen(QBrush(Qt::black),1.00));

}

void RaportinKirjoittaja::asetaKieli(const QString &kieli)
{
    kieli_ = kieli;
}

QString RaportinKirjoittaja::kaanna(const QString &teksti) const
{
    return tulkkaa(teksti, kieli_);
}
