#include "serverconnector.h"

#include <QCoreApplication>

int main( int argc, char *argv[] ) {
    QCoreApplication a{ argc, argv };
    ccServerConnector serverConnector;

    return a.exec();
}
