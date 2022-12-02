#ifndef LISTANPILVI_H
#define LISTANPILVI_H

#include <QPixmap>

class ListanPilvi
{
public:
    ListanPilvi();

    ListanPilvi(const QVariant& variant);

    int id() const { return id_;}
    QString nimi() const { return nimi_;}
    bool kokeilu() const { return kokeilu_;}
    QPixmap logo() const { return logo_;}


protected:
    int id_;
    QString nimi_;
    bool kokeilu_;
    QPixmap logo_;

};

#endif // LISTANPILVI_H