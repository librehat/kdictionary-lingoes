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
#include <QCoreApplication>
#include "kdictionary-lingoes.h"


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    if(argc <= 2) {
        std::cerr<<"No input file. \n"
                 <<"Usage:\n"
                 <<"./kdictionary-lingoes <LD2/LDX FILE> <OUTPUT FILE>\n"
                 <<"Control/Ctrl + C to exit.\n\n";
        exit(1);
    }
    QString ld2file(argv[1]);
    QString outputfile(argv[2]);
    kdictionary_lingoes foo(ld2file);
    foo.main(outputfile);
    //return app.exec();
}
