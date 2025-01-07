/*
 * Copyright (c) 2024 FilmTool
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
