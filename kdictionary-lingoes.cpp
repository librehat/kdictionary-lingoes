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
#include <QDataStream>
#include <QTextStream>

kdictionary_lingoes::kdictionary_lingoes(QString openFile)
{
    ld2file = openFile;
    QFile file(ld2file);
    file.open(QIODevice::ReadOnly);
    ld2ByteArray = file.readAll();
    file.close();
    position = 0;
    inflated_pos = 0;
    AVAIL_ENCODINGS<<"UTF-8"<<"UTF-16LE"<<"UTF-16BE"<<"EUC-JP";
}

void kdictionary_lingoes::main()
{
    std::cout<<"File: "<<ld2file.toStdString()<<"\n";
    std::cout<<"Type: "<<QString::fromAscii(ld2ByteArray.mid(1, 3)).toStdString()<<"\n";
    std::cout<<"Version: "<<getShort(0x18)<<"."<<getShort(0x1A)<<"\n";
    std::cout<<"ID: 0x"<<toHexString(getLong(0x1C)).toStdString()<<"\n";//Wrong!
    int offsetData = getInt(0x5C) + 0x60;
    if(ld2ByteArray.size() > offsetData) {
        std::cout<<"Summary Addr: "<<toHexString(offsetData).toStdString()<<"\n";
        int dtype = getInt(offsetData);
        std::cout<<"Summary Type: "<<toHexString(dtype).toStdString()<<"\n";
        int offsetWithInfo = getInt(offsetData + 4) + offsetData + 12;
        if(dtype == 3) {
            readDictionary(offsetData);
        }
        else if(ld2ByteArray.size() > offsetWithInfo - 0x1C) {
            readDictionary(offsetWithInfo);
        }
        else {
            std::cerr<<"This File Doesn't Contain Dictionary."<<"\n";
        }
    }
    else {
        std::cerr<<"This File Doesn't Contain Dictionary."<<"\n";
    }
    QTextStream s(stdin);
    while(true) {
        std::cerr<<"Input the word you wish to define: ";//use cerr so that you can see this output even trans output to a file
        QString input = s.readLine();
        std::cout<<getDef(input).toStdString()<<std::endl;
    }
}

kdictionary_lingoes::~kdictionary_lingoes()
{}

void kdictionary_lingoes::readDictionary(int offsetWithIndex)
{
    //analyze dictionary file's header
    std::cout<<"Dictionary Type: 0x"<<getInt(offsetWithIndex)<<std::endl;
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
    std::cout<<"Index Numbers: "<<definitions<<std::endl;
    std::cout<<"Index Address/Size: 0x" <<toHexString(offsetIndex).toStdString()<<" / "<<(offsetCompressedDataHeader - offsetIndex)<<" B"<<std::endl;
    std::cout<<"Compressed Data Address/Size: 0x"<<toHexString(offsetCompressedData).toStdString()<<" / "<<(limit - offsetCompressedData)<<" B"<<std::endl;
    std::cout<<"Phrases Index Address/Size(Decompressed): 0x0 / "<<inflatedWordsIndexLength<<" B"<<std::endl;
    std::cout<<"Phrases Address/Size(Decompressed): 0x"<<toHexString(inflatedWordsIndexLength).toStdString()<<" / "<<inflatedWordsLength<<" B"<<std::endl;
    std::cout<<"XML Address/Size(Decompressed): 0x"<<toHexString(inflatedWordsIndexLength + inflatedWordsLength).toStdString()<<" / "<<inflatedXmlLength<<" B"<<std::endl;
    std::cout<<"File Size(Decompressed): "<< ((inflatedWordsIndexLength + inflatedWordsLength + inflatedXmlLength) / 1024)<<" KB"<<std::endl;
    QByteArray inflatedData;
    inflateData(deflateStreams, inflatedData);
    if(!inflatedData.isEmpty()) {
        position = offsetIndex;
        int idxArray[definitions];
        for (int i = 0; i < definitions; i++) {
            idxArray[i] = getInt(position);
            position += sizeof(int);
        }
        extract(idxArray, inflatedData, inflatedWordsIndexLength, inflatedWordsIndexLength + inflatedWordsLength);
    } else {
        std::cerr<<"ERROR: Inflated Data is Empty."<<std::endl;
    }
}

void kdictionary_lingoes::inflateData(QList<int>& deflateStreams, QByteArray& inflatedData)
{
    std::cout<<"Decompressing "<<deflateStreams.size()<<" data streams."<<"\n";
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
        std::cerr<<"Failed decompressing "<<offset<<": "<<e.what()<<std::endl;
    } catch (const char* e2) {
        std::cerr<<e2<<std::endl;
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
        std::cerr<<e.what()<<std::endl;
    } catch (const char* e2) {
        std::cerr<<e2<<std::endl;
    }
}

void kdictionary_lingoes::extract(int idxArray[], QByteArray& inflatedBytes, int offsetDefs, int offsetXml)
{
    //QByteArray indexBytes;
    //QByteArray defsBytes;
    //QByteArray xmlBytes;
    //QByteArray outputBytes;
    int counter = 0;
    const int dataLen = 10;
    const int defTotal = (offsetDefs / dataLen) - 1;
    //QString words[defTotal];
    int idxData[6];
    QString defData[2];
    QStringList encodings = detectEncodings(inflatedBytes, offsetDefs, offsetXml, defTotal, dataLen, idxData, defData);
    
    inflated_pos = 8;
    for (int i = 0; i < defTotal; i++) {
        readDefinitionData(inflatedBytes, offsetDefs, offsetXml, dataLen, encodings.at(0), encodings.at(1), idxData, defData, i);
        //words[i] = defData[0];
        words << defData[0];
        defs << defData[1];
        /*defsBytes.append(defData[0]);
        defsBytes.append("\n");
        
        xmlBytes.append(defData[1]);
        xmlBytes.append("\n");
        
        outputBytes.append(defData[0]);
        outputBytes.append("=");
        outputBytes.append(defData[1]);
        outputBytes.append("\n");
        */
        
        //std::cout<<defData[0].toStdString()<<" = "<<defData[1].toStdString()<<std::endl;
        counter++;
    }
    /*for (int i = 0; i < sizeof(idxArray); i++) {
        const int idx = idxArray[i];
        indexBytes.append(words[idx]);
        indexBytes.append(", ");
        indexBytes.append(idx);
        indexBytes.append("\n");
    }*/
    std::cout<<"Extracted "<<counter<<" entries."<<std::endl;
}

QStringList kdictionary_lingoes::detectEncodings(QByteArray& inflatedBytes, int offsetWords, int offsetXml, const int defTotal, const int dataLen, int idxData[], QString defData[])
{
    /*
     * Try to detect the Encodings.
     * Currently support:
     * UTF-8, UTF-16LE, UTF-16BE, EUC-JP (see constructor function)
     */
    const int test = std::min(defTotal ,10);
    for (int j=0; j< AVAIL_ENCODINGS.size(); j++) {
        for (int k=0; k< AVAIL_ENCODINGS.size(); k++) {
            try {
                for (int i=0; i< test; i++) {
                    readDefinitionData(inflatedBytes, offsetWords, offsetXml, dataLen, AVAIL_ENCODINGS.at(j), AVAIL_ENCODINGS.at(k), idxData, defData, i);
                }
                std::cout<<"Phrases Encoding: "<<AVAIL_ENCODINGS.at(j).toStdString()<<std::endl;
                std::cout<<"XML Encoding: "<<AVAIL_ENCODINGS.at(k).toStdString()<<std::endl;
                return (QStringList() << AVAIL_ENCODINGS.at(j) << AVAIL_ENCODINGS.at(k));
            } catch (...){
                //ignore
            }
        }
    }
    std::cerr<<"Detecting encodings failed. Choose UTF-16LE to continue."<<std::endl;
    return (QStringList() << AVAIL_ENCODINGS.at(1) << AVAIL_ENCODINGS.at(1));
}

void kdictionary_lingoes::readDefinitionData(QByteArray& inflatedBytes, int offsetWords, int offsetXml, const int dataLen, QString wordStringcodec, QString xmlStringcodec, int idxData[], QString defData[], int i)
{
    //get all data from definition area
    getIdxData(inflatedBytes, dataLen * i, idxData);
    int lastWordPos = idxData[0];
    int lastXmlPos = idxData[1];
    int refs = idxData[3];
    const int currentWordOffset = idxData[4];
    int currenXmlOffset = idxData[5];
    lddecoder.setName(xmlStringcodec);
    QString xml = strip(lddecoder.decode(inflatedBytes, offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos));
    while (refs-- > 0) {
        int ref = getInt(inflatedBytes, offsetWords + lastWordPos);
        getIdxData(inflatedBytes, dataLen * ref, idxData);
        lastXmlPos = idxData[1];
        currenXmlOffset = idxData[5];
        if (xml.isEmpty()) {
            xml = strip(lddecoder.decode(inflatedBytes, offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos));
        } else {
            xml = strip(lddecoder.decode(inflatedBytes, offsetXml + lastXmlPos, currenXmlOffset - lastXmlPos)) + ", " + xml;
        }
        lastWordPos += 4;
    }
    defData[1] = xml;
    lddecoder.setName(wordStringcodec);
    QString word = lddecoder.decode(inflatedBytes, offsetWords + lastWordPos, currentWordOffset - lastWordPos);
    defData[0] = word;
}

/*
void kdictionary_lingoes::readWordsData(QByteArray& inflatedBytes, int offsetWords, const int dataLen, int idxData[])
{
    getIdxData(inflatedBytes, dataLen * i, idxData);
    int lastWordPos = idxData[0];
    const int currentWordOffset = idxData[4];
    lastWordPos += idxData[3] * 4;
    words<<lddecoder.decode(inflatedBytes, offsetWords + lastWordPos, currentWordOffset - lastWordPos);
}
*/
QString kdictionary_lingoes::strip(QString xml)
{
    /*
     * Strip some formats characters.
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

QString kdictionary_lingoes::getDef(QString& word)
{
    if(words.isEmpty()) {
        std::cerr<<"NO DATA!"<<std::endl;
        return QString("");
    }
    else {
        int i = words.indexOf(word);
        if(i != -1)     return defs.at(i);
        else    return QString("No result.");
    }
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

// class of SensitiveStringDecoder
SensitiveStringDecoder::SensitiveStringDecoder()
{}

SensitiveStringDecoder::~SensitiveStringDecoder()
{}

void SensitiveStringDecoder::setName(QString& nm)
{
    //Name must be one of codec that Qt support. i.e. UTF-8
    name = nm;
}

QString SensitiveStringDecoder::decode(QByteArray& ba, const int off, const int len)
{
    QString result;
    try {
        QTextDecoder td(QTextCodec::codecForName(name.toLocal8Bit()));
        result = td.toUnicode(ba.mid(off, len));
    } catch (std::exception e) {
        std::cerr<<e.what()<<std::endl;
    }
    return result;
}

#include "kdictionary-lingoes.moc"
