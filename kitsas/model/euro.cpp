/*
   Copyright (C) 2019 Arto Hyvättinen

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
#include "euro.h"
#include <QRegularExpression>

Euro::Euro()
{

}

Euro::Euro(qlonglong cents)
    : cents_(cents)
{

}


Euro::Euro(const QString &euroString)
    : cents_( stringToCents(euroString) )
{

}

qlonglong Euro::cents() const
{
    return cents_;
}

double Euro::toDouble() const
{
    return cents_ / 100.0;
}

QString Euro::toString() const
{
    const qlonglong euros = qAbs(cents_ / 100);
    const int extraCents = qAbs(cents_ % 100);
    QString merkki = cents_ < 0 ? "-" : "";

    if( extraCents < 10) {
        return merkki + QString::number(euros) + ".0" + QString::number(extraCents);
    } else {
        return merkki + QString::number(euros) + "." + QString::number(extraCents);
    }
}

QString Euro::local() const
{
    return QString("%L1").arg( cents_ / 100.0, 0, 'f', 2);
}

QString Euro::display(bool naytaNolla) const
{
    if( !cents_ && !naytaNolla)
        return QString();

    return QString("%L1 €").arg( cents_ / 100.0, 0, 'f', 2);
}

QVariant Euro::toVariant() const
{
    return QVariant( toString() );
}

Euro Euro::abs() const
{
    if( cents_ < 0)
        return Euro(0L - cents_);
    else
        return Euro(cents_);
}

Euro Euro::operator+(const Euro &other) const
{
    qlonglong sum = this->cents() + other.cents();
    return Euro(sum);
}

Euro Euro::operator -(const Euro &other) const
{
    qlonglong sub = this->cents() - other.cents();
    return Euro(sub);
}

Euro &Euro::operator+=(const Euro &other)
{
    cents_ += other.cents();
    return *this;
}

Euro &Euro::operator-=(const Euro &other)
{
    cents_ -= other.cents();
    return *this;
}

bool Euro::operator<(const Euro &other) const
{
    return this->cents() < other.cents();
}

bool Euro::operator>(const Euro &other) const
{
    return this->cents() > other.cents();
}

Euro Euro::operator*(const Euro &other) const
{
    qlonglong cents = cents_ * other.cents();
    return Euro(cents);
}

Euro Euro::operator/(const Euro &other) const
{
    qlonglong cents = qRound64( 1.0 * cents_ / other.cents() );
    return Euro(cents);
}

Euro Euro::fromVariant(const QVariant &variant)
{
    return Euro( variant.toString() );
}

Euro Euro::fromDouble(const double euro)
{
    return Euro( qRound64(euro * 100.0));
}

Euro Euro::fromString(QString euroString)
{
    bool miinus = euroString.contains('-');
    euroString.remove(cleaner__);
    euroString.replace(',','.');
    if(miinus)
        euroString = "-" + euroString;
    return Euro(euroString);
}

Euro Euro::fromCents(const qlonglong cents)
{
    return Euro( cents );
}

Euro &Euro::operator<<(const QString &string)
{
    cents_ = stringToCents(string);
    return *this;
}

Euro::operator qlonglong() const
{
    return cents_;
}


Euro::operator bool() const
{
    return cents_ != 0l;
}

Euro::operator double() const
{
    return toDouble();
}

Euro::operator QVariant() const
{
    return toVariant();
}

Euro::operator QString() const
{
    return toString();
}

Euro& Euro::operator<<(const QVariant& variant) {
    cents_ = fromVariant(variant).cents();
    return *this;
}


qlonglong Euro::stringToCents(const QString &euroString)
{
    const int decimalIndex = euroString.indexOf('.');
    const int length = euroString.length();
    const int decimals = length - decimalIndex - 1;
    const int prefix = euroString.contains('-') ? -1 : 1;

    if( decimalIndex == -1) {
        return euroString.toLongLong() * 100;
    }

    const qlonglong euros = qAbs(euroString.left(decimalIndex).toLongLong());
    const qlonglong extraCents = euroString.mid(decimalIndex + 1, 2).toLongLong();

    if( decimals == 1) {
        return ( euros * 100 + extraCents * 10 ) * prefix;
    }
    return (euros * 100 + extraCents) * prefix;

}

QDataStream& operator<<(QDataStream &out, const Euro& euro) {
    out << euro.toString();
    return out;
}

QDataStream& operator>>(QDataStream &in, Euro &euro) {
    QString euroString;
    in >> euroString;
    euro = Euro(euroString);
    return in;
}

QString& operator<<(QString& out, const Euro& euro) {
    out.append(euro.toString());
    return out;
}

QDebug operator<<(QDebug debug, const Euro& euro) {
    debug.nospace() << euro.display();
    return debug.maybeSpace();
}


bool operator==(const Euro& a, const Euro& b) {
    return a.cents() == b.cents();
}

bool operator!=(const Euro& a, const Euro& b) {
    return a.cents() != b.cents();
}

Euro operator*(const Euro& a, const int b) {
    return Euro( a.cents() * b);
}

Euro operator*(const int a, const Euro& b) {
    return Euro( a * b.cents() );
}

Euro operator*(const Euro& a, const double b) {
    return Euro( qRound64( a.cents() * b) );
}

Euro operator*(const double a, const Euro& b) {
    return Euro( qRound64( a * b.cents() ));
}

Euro operator/(const Euro& a, const int b) {
    return Euro( qRound64( a.cents() * 1.0 / b )  );
}

Euro operator/(const Euro& a, const double b) {
    return Euro( qRound64( a.cents() / b ));
}

const Euro Euro::Zero = Euro(0);

QRegularExpression Euro::cleaner__ = QRegularExpression("([+-]|\\s)");
