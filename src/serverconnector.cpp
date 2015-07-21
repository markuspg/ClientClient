#include "serverconnector.h"

ccServerConnector::ccServerConnector( QObject *argParent) :
    QObject{ argParent },
    connectionIntervalTimer{ this },
    env{ QProcessEnvironment::systemEnvironment() },
    settings{ "Economic Laboratory", "EcoLabLib", this },
    socket{ this }
{
    if ( !socket.bind( QHostAddress{ settings.value( "server_ip", "127.0.0.1" ).toString() },
                       settings.value( "server_port", "19870" ).toUInt() + 1 ) ) {
        throw 20;
    }
    connect( &socket, &QTcpSocket::readyRead, this, &ccServerConnector::ReadMessage );
    connect( &socket, SIGNAL( connected() ),
             &connectionIntervalTimer, SLOT( stop() ) );
    connect( &socket, SIGNAL( disconnected() ),
             &connectionIntervalTimer, SLOT( start() ) );

    connect( &connectionIntervalTimer, &QTimer::timeout,
             this, &ccServerConnector::TryConnect );
    connectionIntervalTimer.setInterval( 3000 );
    connectionIntervalTimer.start();
}

void ccServerConnector::KillzLeaf() {
    QProcess killzLeafProcess;
    killzLeafProcess.setProcessEnvironment( env );
#ifdef Q_OS_UNIX
    killzLeafProcess.startDetached( "/usr/bin/killall", QStringList{ "zleaf.exe" } );
#endif
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

#ifdef Q_OS_UNIX
    shutdownProcess.startDetached( "sudo shutdown -hP now" );
#endif
}

void ccServerConnector::StartzLeaf( const QString &argzLeafSettings ) {
    QProcess startzLeafProcess;
    startzLeafProcess.setProcessEnvironment( env );

    QStringList zleafSettings = argzLeafSettings.split( '|', QString::SkipEmptyParts );

    QString program;
    QStringList arguments;
#ifdef Q_OS_UNIX
    program = "/usr/bin/wine";
    arguments.append( "/opt/zTree_" + zleafSettings[ 0 ] + "/zleaf.exe" );
#else
    program = QString{ "/opt/zTree_" + zleafSettings[ 0 ] + "/zleaf.exe" };
#endif
    arguments << "/server" << zleafSettings[ 1 ];
    startzLeafProcess.startDetached( program, arguments );
}

void ccServerConnector::TryConnect() {
    if ( socket.state() != QAbstractSocket::HostLookupState ||
         socket.state() != QAbstractSocket::ConnectingState ||
         socket.state() != QAbstractSocket::ConnectedState ) {
        socket.connectToHost( QHostAddress{ settings.value( "server_ip", "127.0.0.1" ).toString() },
                              settings.value( "server_port", "19870" ).toUInt() );
    }
}
