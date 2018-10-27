/*!
 * \file main.cpp
 * \brief Brief
 *
 * Long
 * Description
 *
 * \date 26.10.2018
 * \author Королев Дмитрий <d.v.korolev@inbox.ru>
 */
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
