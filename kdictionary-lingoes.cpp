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
#include "kdictionary-lingoes.h"
#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QTextStream>

kdictionary_lingoes::kdictionary_lingoes(const QString &openFile)
{
    ld2file = openFile;
    QFile file(ld2file);
    file.open(QIODevice::ReadOnly);
    ld2ByteArray = file.readAll();
    file.close();
    position = 0;
    inflated_pos = 0;
}

const QList<QByteArray> kdictionary_lingoes::available_encodings = QList<QByteArray>() << "UTF-8" << "UTF-16LE" << "UTF-16BE" << "EUC-JP";

void kdictionary_lingoes::main(QString& outputfile)
{
    qDebug() << QString("File: ").append(ld2file);
    qDebug() << QString("Type: ").append(QString::fromLatin1(ld2ByteArray.mid(1, 3)));
    qDebug() << QString("Version: %1.%2").arg(QString::number(getShort(0x18)), QString::number(getShort(0x1A)));
    qDebug() << QString("ID: 0x").append(toHexString(getLong(0x1C)));
    int offsetData = getInt(0x5C) + 0x60;
    if(ld2ByteArray.size() > offsetData) {
        qDebug() << QString("Summary Addr: ").append(toHexString(offsetData));
        int dtype = getInt(offsetData);
        qDebug() << QString("Summary Type: ").append(toHexString(dtype));
        int offsetWithInfo = getInt(offsetData + 4) + offsetData + 12;
        if(dtype == 3) {
            readDictionary(offsetData, outputfile);
        }
        else if(ld2ByteArray.size() > offsetWithInfo - 0x1C) {
            readDictionary(offsetWithInfo, outputfile);
        }
        else {
            qWarning() << "This File Doesn't Contain Dictionary.";
        }
    }
    else {
        qWarning() <<"This File Doesn't Contain Dictionary.";
    }
}

kdictionary_lingoes::~kdictionary_lingoes()
{}

void kdictionary_lingoes::readDictionary(int offsetWithIndex, QString& outputfile)
{
    //analyze dictionary file's header
    qDebug() << QString("Dictionary Type: 0x").append(QString::number(getInt(offsetWithIndex)));
    const int limit = getInt(offsetWithIndex + 4) + offsetWithIndex + 8;
    const int offsetIndex = offsetWithIndex + 0x1C;
    const int offsetCompressedDataHeader = getInt(offsetWithIndex + 8) + offsetIndex;
    const int inflatedWordsIndexLength = getInt(offsetWithIndex + 12);
    const int inflatedWordsLength = getInt(offsetWithIndex + 16);
    const int inflatedXmlLength = getInt(offsetWithIndex + 20);
    const int definitions = (offsetCompressedDataHeader - offsetIndex) / 4;
    QList<int> deflateStreams;
    position = offsetCompressedDataHeader + 8;//position here represents ByteBuffer's position() in Java
    int offset = getInt(position);//In java, ByteBuffer's getInt() will increase the position by four(an Integer size).
    position += sizeof(int);//Hence, we need to add it to position manually.
    while (offset + position < limit) {
        offset = getInt(position);
        deflateStreams.append(offset);
        position += sizeof(int);
    }
    int offsetCompressedData = position;
    qDebug() << QString("Index Numbers: ").append(QString::number(definitions));
    qDebug() << QString("Index Address/Size: 0x%1 / %2B").arg(toHexString(offsetIndex), QString::number(offsetCompressedDataHeader - offsetIndex));
    qDebug() << QString("Compressed Data Address/Size: 0x%1 / %2B").arg(toHexString(offsetCompressedData), QString::number(limit - offsetCompressedData));
    qDebug() << QString("Phrases Index Address/Size(Decompressed): 0x0 / %1B").arg(QString::number(inflatedWordsIndexLength));
    qDebug() << QString("Phrases Address/Size(Decompressed): 0x%1 / %2B").arg(toHexString(inflatedWordsIndexLength), QString::number(inflatedWordsLength));
    qDebug() << QString("XML Address/Size(Decompressed): 0x%1 / %2").arg(toHexString(inflatedWordsIndexLength + inflatedWordsLength), QString::number(inflatedXmlLength));
    qDebug() << QString("File Size(Decompressed): %1KB").arg(QString::number((inflatedWordsIndexLength + inflatedWordsLength + inflatedXmlLength) / 1024));
    QByteArray inflatedData;
    inflateData(deflateStreams, inflatedData);
    if(!inflatedData.isEmpty()) {
        position = offsetIndex;
        int idxArray[definitions];
        for (int i = 0; i < definitions; i++) {
            idxArray[i] = getInt(position);
            position += sizeof(int);
        }
        extract(idxArray, inflatedData, inflatedWordsIndexLength, inflatedWordsIndexLength + inflatedWordsLength, outputfile);
    } else {
        qWarning() << "ERROR: Inflated Data is Empty.";
    }
}

void kdictionary_lingoes::inflateData(QList<int>& deflateStreams, QByteArray& inflatedData)
{
    qDebug() << QString("Decompressing %1 data streams.").arg(QString::number(deflateStreams.size()));
    int startOffset = position;
    int offset = -1;
    int lastOffset = startOffset;
    try {
        foreach(int offsetRelative, deflateStreams) {
            offset = startOffset + offsetRelative;
            decompress(inflatedData, lastOffset, offset - lastOffset);
            lastOffset = offset;
        }
    } catch (std::exception e) {
        qWarning() << QString("Failed decompressing %1: %2").arg(QString::number(offset), QString::fromStdString(e.what()));
    } catch (const char* e2) {
        qWarning() << e2;
    }
}

inline void kdictionary_lingoes::decompress(QByteArray& inflatedData, int offset, int length)
{
    //uncompress deflate datastream
    try {
        QByteArray data = ld2ByteArray.mid(offset, length);
        data.prepend(QByteArray::number(length));
        inflatedData.append(qUncompress(data));
    } catch (std::exception e) {
        qWarning() << e.what();
    } catch (const char* e2) {
        qWarning() << e2;
    }
}

void kdictionary_lingoes::extract(int idxArray[], QByteArray& inflatedBytes, int offsetDefs, int offsetXml, QString& outputfile)
{
    QFile fileout(outputfile);
    fileout.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&fileout);
    int counter = 0;
    const int dataLen = 10;
    const int defTotal = (offsetDefs / dataLen) - 1;
    QString line;
    int idxData[6];
    QString defData[2];
    detectEncodings(inflatedBytes, offsetDefs, offsetXml, defTotal, dataLen, idxData);
    
    inflated_pos = 8;
    for (int i = 0; i < defTotal; i++) {
        readDefinitionData(inflatedBytes, offsetDefs, offsetXml, dataLen, idxData, defData, i);
        line.append(defData[0]);
        line.append("=");
        line.append(defData[1]);
        out<<line<<endl;
        out.flush();
        line.clear();
        counter++;
    }

    fileout.close();
    qDebug() << QString("Extracted %1 entries.").arg(QString::number(counter));
}

void kdictionary_lingoes::detectEncodings(QByteArray& inflatedBytes, int offsetWords, int offsetXml, const int defTotal, const int dataLen, int idxData[])
{
    /*
     * Try to detect the Encodings.
     * TODO: Its accuracy needs to be improved! Or find the codec definition in LD2/LDX header.
     */
    int wordc_id = 0, xmlc_id = 0;
    int testIdx = qMin(defTotal, 10);

    while (--testIdx > 0) {
        getIdxData(inflatedBytes, dataLen * testIdx, idxData);
        const int lastXmlPos = idxData[1];
        const int lastWordPos = idxData[0] + 4 * idxData[3];
        const int currentWordOffset = idxData[4];
        const int currenXmlOffset = idxData[5];
        QByteArray wordbytes = inflatedBytes.mid(offsetWords + lastWordPos, currentWordOffset - lastWordPos);
        QByteArray xmlbytes = inflatedBytes.mid(offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos);

        wordc = QTextCodec::codecForName(available_encodings.at(wordc_id));
        try {
            wordc->toUnicode(wordbytes);
        }
        catch (std::exception) {
            if (wordc_id < available_encodings.size() - 1) {
                ++wordc_id;
            }
            else {
                qWarning() << "Cannot detect a valid encoding for Words. Output may be corrupted.";
            }
        }

        xmlc = QTextCodec::codecForName(available_encodings.at(xmlc_id));
        try {
            xmlc->toUnicode(xmlbytes);
        }
        catch (std::exception) {
            if (xmlc_id < available_encodings.size() - 1) {
                ++xmlc_id;
            }
            else {
                qWarning() << "Cannot detect a valid encoding for XML. Output may be corrupted.";
            }
        }
    }

    qDebug() << QString("Phrases Encoding: %1").arg(QString(wordc->name()));
    qDebug() << QString("XML Encoding: %1").arg(QString(xmlc->name()));
}

void kdictionary_lingoes::readDefinitionData(QByteArray& inflatedBytes, int offsetWords, int offsetXml, const int dataLen, int idxData[], QString defData[], int i)
{
    //get all data from definition area
    getIdxData(inflatedBytes, dataLen * i, idxData);
    int lastWordPos = idxData[0];
    int lastXmlPos = idxData[1];
    int refs = idxData[3];
    const int currentWordOffset = idxData[4];
    int currenXmlOffset = idxData[5];
    QString xml = strip(xmlc->toUnicode(inflatedBytes.mid(offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos)));
    while (refs-- > 0) {
        int ref = getInt(inflatedBytes, offsetWords + lastWordPos);
        getIdxData(inflatedBytes, dataLen * ref, idxData);
        lastXmlPos = idxData[1];
        currenXmlOffset = idxData[5];
        if (xml.isEmpty()) {
            xml = strip(xmlc->toUnicode(inflatedBytes.mid(offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos)));
        } else {
            xml = strip(xmlc->toUnicode(inflatedBytes.mid(offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos))) + ", " + xml;
        }
        lastWordPos += 4;
    }
    defData[1] = xml;
    QString word = wordc->toUnicode(inflatedBytes.mid(offsetWords + lastWordPos, currentWordOffset - lastWordPos));
    defData[0] = word;
}

QString kdictionary_lingoes::strip(QString xml)
{
    /*
     * Strip some formats characters.
     * TODO: strip HTML tags such as <TD> <TR>
     */
    int open = 0;
    int end = 0;
    const int SZ = QString("<![CDATA[").length();
    if ((open = xml.indexOf("<![CDATA[")) != -1) {
        if ((end = xml.indexOf("]]>", open)) != -1) {
            return xml.mid(open + SZ, end - SZ - open).replace('\t', ' ').replace('\n', ' ').replace('\u001e', ' ').replace('\u001f', ' ');
        }
    } else if ((open = xml.indexOf("<Ô")) != -1) {
        if ((end = xml.indexOf("</Ô", open)) != -1) {
            open = xml.indexOf(">", open + 1);
            return xml.mid(open + 1, end - open - 1).replace('\t', ' ').replace('\n', ' ').replace('\u001e', ' ').replace('\u001f', ' ');
        }
    } else {
        QString sb;
        end = 0;
        open = xml.indexOf('<');
        do {
            if ((open - end) > 1) {
                sb.append(xml.mid(end + 1, open - end - 1));
            }
            open = xml.indexOf('<', open + 1);
            end = xml.indexOf('>', end + 1);
        } while ((open != -1) && (end != -1));
        return sb.replace('\t', ' ').replace('\n', ' ').replace('\u001e', ' ').replace('\u001f', ' ');
    }
    return QString("");
}

void kdictionary_lingoes::getIdxData(QByteArray& inflatedBytes, int pos, int wordIdxData[])
{
    inflated_pos = pos;
    wordIdxData[0] = getInt(inflatedBytes ,inflated_pos);
    inflated_pos += sizeof(int);
    wordIdxData[1] = getInt(inflatedBytes ,inflated_pos);
    inflated_pos += sizeof(int);
    wordIdxData[2] = inflatedBytes[inflated_pos] & 0xff;
    inflated_pos ++;
    wordIdxData[3] = inflatedBytes[inflated_pos] & 0xff;
    inflated_pos ++;
    wordIdxData[4] = getInt(inflatedBytes ,inflated_pos);
    inflated_pos += sizeof(int);
    wordIdxData[5] = getInt(inflatedBytes ,inflated_pos);
    inflated_pos += sizeof(int);
}

//Inspired by https://github.com/Dasister/Game-Server-Query/blob/master/sourcequery.cpp
inline int kdictionary_lingoes::getInt(int index)
{
    return *((int*)ld2ByteArray.mid(index, sizeof(int)).data());
}

inline int kdictionary_lingoes::getInt(QByteArray& ba, int index)
{
    return *((int*)ba.mid(index, sizeof(int)).data());
}

inline short int kdictionary_lingoes::getShort(int index)
{
    return *((short*)ld2ByteArray.mid(index, sizeof(short)).data());
}

inline long int kdictionary_lingoes::getLong(int index)
{
    return *((long*)ld2ByteArray.mid(index, sizeof(long)).data());
}

inline QString kdictionary_lingoes::toHexString(long int num)
{
    return QString::number(num, 16).toUpper();
}

inline QString kdictionary_lingoes::toHexString(int num)
{
    return QString::number(num, 16).toUpper();
}
