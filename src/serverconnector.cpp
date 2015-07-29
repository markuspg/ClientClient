/*
 * Copyright 2015 Markus Prasser
 *
 * This file is part of ClientClient.
 *
 *  ClientClient is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ClientClient is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ClientClient.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "serverconnector.h"

ccServerConnector::ccServerConnector( QObject *argParent) :
    QObject{ argParent },
    connectionIntervalTimer{ this },
    env{ QProcessEnvironment::systemEnvironment() },
    settings{ "Economic Laboratory", "ClientClient", this },
    socket{ this }
{
    if ( !socket.bind( QHostAddress{ settings.value( "host_ip", "127.0.0.1" ).toString() },
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

    connect( &startzLeafProcess, &QProcess::started,
             this, &ccServerConnector::zleafStartedSuccessfully );
    connect( &startzLeafProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             this, SLOT( zleafClosed( int, QProcess::ExitStatus ) ) );
}

void ccServerConnector::KillzLeaf() {
    QProcess killzLeafProcess;
    killzLeafProcess.setProcessEnvironment( env );
#ifdef Q_OS_UNIX
    killzLeafProcess.startDetached( settings.value( "killall_command", "/usr/bin/killall" ).toString(),
                                    QStringList{ "zleaf.exe" } );
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

void ccServerConnector::SendMessage( const quint16 &argMessageID, QString *argMessage ) {
    QByteArray block;
    QDataStream out{ &block, QIODevice::WriteOnly };
    out.setVersion( QDataStream::Qt_5_2 );
    out << ( quint16 )0;
    out << ( quint16 )argMessageID;
    if ( argMessage ) {
        out << *argMessage;
        delete argMessage;
    }
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

    this->deleteLater();
}

void ccServerConnector::StartzLeaf( const QString &argzLeafSettings ) {
    startzLeafProcess.setProcessEnvironment( env );

    QStringList zleafSettings = argzLeafSettings.split( '|', QString::SkipEmptyParts );

    QString program;
    QStringList arguments;
#ifdef Q_OS_UNIX
    program = settings.value( "wine_command", "/usr/bin/wine" ).toString();
    arguments.append( settings.value( "ztree_installation_directory" ).toString()
                      + "/zTree_" + zleafSettings[ 0 ] + "/zleaf.exe" );
#else
    program = QString{ settings.value( "ztree_installation_directory" ).toString()
                       + "/zTree_" + zleafSettings[ 0 ] + "/zleaf.exe" };
#endif
    arguments << "/server" << zleafSettings[ 1 ]
              << "/channel" << QString::number( zleafSettings[ 2 ].toUInt() - 7000 );

    if ( zleafSettings.count() == 4 ) {
        arguments << "/name" << zleafSettings[ 3 ];
    }

    startzLeafProcess.start( program, arguments );
}

void ccServerConnector::TryConnect() {
    if ( socket.state() != QAbstractSocket::HostLookupState ||
         socket.state() != QAbstractSocket::ConnectingState ||
         socket.state() != QAbstractSocket::ConnectedState ) {
        socket.connectToHost( QHostAddress{ settings.value( "server_ip", "127.0.0.1" ).toString() },
                              settings.value( "server_port", "19870" ).toUInt() );
    }
}

void ccServerConnector::zleafClosed( const int &argExitCode, const QProcess::ExitStatus &argExitStatus ) {
    QString *message = new QString{ tr( "z-Leaf closed with exit code '%1' and exit status '%2" )
                                    .arg( argExitCode ).arg( argExitStatus ) };
    qDebug() << *message;
    SendMessage( 1, message );
}

void ccServerConnector::zleafStartedSuccessfully() {
    SendMessage( 0 );
}
