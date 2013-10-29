/*
 *   Read Lingoes Dictionary Files (*.ld2 or *.ldx)
 *   Copyright (C) 2013 by William Wong <librehat@outlook.com>
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
#ifndef kdictionary_lingoes_H
#define kdictionary_lingoes_H

#include <QtCore/QObject>
#include <QTextDecoder>
#include <QTextCodec>
#include <QStringList>
#include <iostream>

class SensitiveStringDecoder 
{
public:
    SensitiveStringDecoder();
    virtual ~SensitiveStringDecoder();
    void setName(QString&);
    QString name;
    QString decode(QByteArray&, const int, const int);
};

class kdictionary_lingoes : public QObject
{
Q_OBJECT
public:
    kdictionary_lingoes(QString);
    virtual ~kdictionary_lingoes();
    void main();
    void readDictionary(int offsetWithIndex);
    int getInt(int);
    int getInt(QByteArray&, int);
    short getShort(int);
    long getLong(int);
    QString toHexString(long int);
    QString toHexString(int);
    QString getDef(QString&);
private:
    int position;
    int inflated_pos;
    QString ld2file;
    QByteArray ld2ByteArray;
    QStringList AVAIL_ENCODINGS;//Define the encodings determined later.
    SensitiveStringDecoder lddecoder;
    QStringList defs;//Store all definitioins
    QStringList words;//Store all words
    void inflateData(QList<int>&, QByteArray&);
    void decompress(QByteArray&, int, int);
    void extract(int a[], QByteArray&, int, int);
    QStringList detectEncodings(QByteArray&, int, int, const int, const int, int a[], QString s[]);
    void readDefinitionData(QByteArray&, int, int, const int, QString, QString, int a[], QString s[], int);
    //void readWordsData(QByteArray& inflatedBytes, int offsetWords, const int dataLen, int idxData[]);
    void getIdxData(QByteArray&, int, int a[]);
    QString strip(QString);
};
#endif // kdictionary_lingoes_H
