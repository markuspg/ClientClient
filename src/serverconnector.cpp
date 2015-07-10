#include "serverconnector.h"

ccServerConnector::ccServerConnector( QObject *argParent) :
    QObject{ argParent },
    env{ QProcessEnvironment::systemEnvironment() },
    socket{ this }
{
    if ( !socket.bind( QHostAddress{ "192.168.53.100" }, 19871 ) ) {
        throw 20;
    }
    socket.connectToHost( QHostAddress{ "192.168.53.100" }, 19870 );
    connect( &socket, &QTcpSocket::readyRead, this, &ccServerConnector::ReadMessage );
}

void ccServerConnector::KillzLeaf() {
    QProcess killzLeafProcess;
    killzLeafProcess.setProcessEnvironment( env );
    killzLeafProcess.startDetached( "/usr/bin/killall", QStringList{ "zleaf.exe" } );
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

    qDebug() << QString::number( blockSize );
    qDebug() << QString::number( messageID );

    QString serverAnswer;
    in >> serverAnswer;

    qDebug() << serverAnswer;

    switch ( messageID ) {
    case 0:
        Shutdown();
        break;
    case 1:
        StartzLeaf( serverAnswer );
        break;
    case 2:
        KillzLeaf();
        break;
    default:
        true;
    }
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

void ccServerConnector::Shutdown() {
    socket.disconnectFromHost();

    QProcess shutdownProcess;
    shutdownProcess.setProcessEnvironment( env );
    shutdownProcess.startDetached( "sudo shutdown -hP now" );
}

void ccServerConnector::StartzLeaf( const QString &argzLeafSettings ) {
    QProcess startzLeafProcess;
    startzLeafProcess.setProcessEnvironment( env );

    QStringList zleafSettings = argzLeafSettings.split( '|', QString::SkipEmptyParts );
    QStringList startzLeafarguments = QStringList{} << "/opt/zTree_" + zleafSettings[ 0 ] + "/zleaf.exe"
                                                    << "/server" << zleafSettings[ 1 ];
    startzLeafProcess.startDetached( "/usr/bin/wine", startzLeafarguments );
}
