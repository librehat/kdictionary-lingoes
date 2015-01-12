/*
 *   Read Lingoes Dictionary Files (*.ld2 or *.ldx)
 *   Copyright (C) 2013-2015 by Symeon Huang <hzwhuang@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 3 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef lingoes_H
#define lingoes_H

#include <QObject>
#include <QTextDecoder>
#include <QTextCodec>
#include <exception>

class Lingoes : public QObject
{
Q_OBJECT

public:
    Lingoes(const QString &);
    void extractToFile(QString&);
    int getInt(int);
    int getInt(QByteArray&, int);
    qint16 getShort(int);//the short is 16-bit integer
    qint32 getLong(int);//while the long is actually 32-bit integer.
    QByteArray toHexString(qint32);
    QByteArray toHexString(qint16);

private:
    int position;
    int inflated_pos;
    QString ld2file;
    QByteArray ld2ByteArray;
    QTextCodec* xmlc;//XML Encoding
    QTextCodec* wordc;//Words Encoding
    void readDictionary(int offsetWithIndex, QString&);
    void inflateData(QVector<int> &, QByteArray *);
    void decompress(QByteArray *, int, quint32);
    void extract(QByteArray&, int, int, QString&);
    void detectEncodings(QByteArray&, int, int, const int, const int, int a[]);
    void readDefinitionData(QByteArray&, int, int, const int, int a[], QString s[], int);
    void getIdxData(QByteArray&, int, int a[]);
    QString strip(QString);

    const static QVector<QByteArray> availableEncodings;
};
#endif // lingoes_H
