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
#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <QCommandLineParser>
#include <QDebug>
#include "lingoes.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("KDictionary-Lingoes");
    app.setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Lingoes dictionary file (LD2/LDX) reader/extracter.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption ldxfile("i", "Input Lingoes dictionary file (default: input.ld2).", "input", "input.ld2");
    QCommandLineOption outfile("o", "Output extracted text file (default: output.txt).", "output", "output.txt");
    QCommandLineOption notrim("disable-trim", "Disable HTML tag trimming.");

    parser.addOption(ldxfile);
    parser.addOption(outfile);
    parser.addOption(notrim);

    parser.process(app);

    const QString inputFile = parser.value(ldxfile);
    QFileInfo ld2FileInfo(inputFile);
    if (!ld2FileInfo.exists()) {
        qCritical()<<"Error: Input file" << inputFile << "doesn't exist.";
        exit(1);
    }

    QString ld2file = ld2FileInfo.canonicalFilePath();
    bool trim = !parser.isSet(notrim);
    Lingoes ldx(ld2file, trim);
    ldx.extractToFile(parser.value(outfile));

    return 0;
}
