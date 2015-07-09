#include "serverconnector.h"

ccServerConnector::ccServerConnector( QObject *argParent) :
    QObject{ argParent },
    socket{ this }
{
    if ( !socket.bind( QHostAddress{ "192.168.53.100" }, 19871 ) ) {
        throw 20;
    }
    socket.connectToHost( QHostAddress{ "192.168.53.100" }, 19870 );
    connect( &socket, &QTcpSocket::readyRead, this, &ccServerConnector::ReadMessage );
}

void ccServerConnector::ReadMessage() {
    QDataStream in( &socket );
    in.setVersion( QDataStream::Qt_5_2 );

    blockSize = 0;
    if ( blockSize == 0 ) {
        if ( socket.bytesAvailable() < ( int )sizeof( quint16 ) ) {
            return;
        }
        in >> blockSize;
        in >> messageID;
    }

    if ( socket.bytesAvailable() < blockSize ) {
        return;
    }

    QString serverAnswer;
    in >> serverAnswer;

    qDebug() << serverAnswer;
}

void ccServerConnector::SendMessage( const QString &argMessage, const quint16 &argMessageID ) {
    QByteArray block;
    QDataStream out{ &block, QIODevice::WriteOnly };
    out.setVersion( QDataStream::Qt_5_2 );
    out << ( quint16 )0;
    out << ( quint16 )argMessageID;
    out << argMessage;
    out.device()->seek( 0 );
    out << ( quint16 )( block.size() - sizeof( quint16 ) * 2 );

    socket.write( block );
}