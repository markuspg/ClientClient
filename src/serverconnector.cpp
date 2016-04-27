/*
 * Copyright 2015-2016 Markus Prasser
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
#ifdef Q_OS_UNIX
    settings{ "Economic Laboratory", "ClientClient", this },
#endif
#ifdef Q_OS_WIN
    settings{ "C:\\EcoLabLib\\EcoLabLib.conf", QSettings::IniFormat, this },
#endif
    webSocket{ QStringLiteral( "client" ),
               QWebSocketProtocol::Version13, this }
{
    connect( &webSocket, &QWebSocket::connected,
             this, &ccServerConnector::OnWebSocketConnected );
    connect( &webSocket, &QWebSocket::textMessageReceived,
             this, &ccServerConnector::OnTextMessageReceived );
    typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect( &webSocket, static_cast<sslErrorsSignal>(&QWebSocket::sslErrors),
             this, &ccServerConnector::OnSSLErrors );
    connect( &webSocket, SIGNAL( connected() ),
             &connectionIntervalTimer, SLOT( stop() ) );
    connect( &webSocket, SIGNAL( disconnected() ),
             &connectionIntervalTimer, SLOT( start() ) );

    connectionIntervalTimer.setInterval( 3000 );
    connectionIntervalTimer.start();
    connect( &connectionIntervalTimer, &QTimer::timeout,
             this, &ccServerConnector::TryConnect );

    connect( &startzLeafProcess, &QProcess::started,
             this, &ccServerConnector::zleafStartedSuccessfully );
    connect( &startzLeafProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             this, SLOT( zleafClosed( int, QProcess::ExitStatus ) ) );
}

void ccServerConnector::KillzLeaf() {
    qDebug() << "Killing z-Leaf";
    QProcess killzLeafProcess;
    killzLeafProcess.setProcessEnvironment( env );
#ifdef Q_OS_UNIX
    killzLeafProcess.startDetached( settings.value( "killall_command", "/usr/bin/killall" ).toString(),
                                    QStringList{ "zleaf.exe" } );
#endif
#ifdef Q_OS_WIN
    killzLeafProcess.startDetached( "taskkill",
                                    QStringList{} << "/IM" << "zleaf.exe" );
#endif
}

void ccServerConnector::OnSSLErrors( const QList<QSslError> &argErrors ) {
    Q_UNUSED( argErrors );

    for ( auto s: argErrors ) {
        qWarning() << "SSL error: " << s.errorString();
    }

    webSocket.ignoreSslErrors();
}

void ccServerConnector::OnTextMessageReceived( QString argMessage ) {
    qDebug() << "Received message: " <<  argMessage;

    QStringList tempMessageSplit = argMessage.split( '|', QString::SkipEmptyParts, Qt::CaseSensitive );

    if ( tempMessageSplit[ 0 ] == "StartzLeaf" ) {
        StartzLeaf( tempMessageSplit );
        return;
    }
    if ( tempMessageSplit[ 0 ] == "KillzLeaf" ) {
        KillzLeaf();
        return;
    }
    if ( tempMessageSplit[ 0 ] == "Shutdown" ) {
        Shutdown();
        return;
    }

    qWarning() << "Unhandled message received from server";
}

void ccServerConnector::OnWebSocketConnected() {
    webSocket.sendTextMessage( settings.value( "server_connection_password", "password" ).toString() );
}

void ccServerConnector::SendMessage( const quint16 &argMessageID, const QString *argMessage ) {
    QString message;
    if ( argMessage ) {
        message = QString::number( argMessageID ) + "|" + *argMessage;
    } else {
        message = QString::number( argMessageID );
    }
    webSocket.sendTextMessage( message );
    delete argMessage;
}

void ccServerConnector::Shutdown() {
    qDebug() << "Attempting to shut down";
    webSocket.close( QWebSocketProtocol::CloseCodeNormal, "Shutting down" );

    QProcess shutdownProcess;
    shutdownProcess.setProcessEnvironment( env );

#ifdef Q_OS_UNIX
    shutdownProcess.startDetached( "sudo shutdown -hP now" );
#endif
#ifdef Q_OS_WIN
    shutdownProcess.startDetached( "shutdown",
                                   QStringList{} << "/s" << "/t" << "0" );
#endif

    this->deleteLater();
}

void ccServerConnector::StartzLeaf( const QStringList &argzLeafSettings ) {
    qDebug() << "Starting z-Leaf";
    startzLeafProcess.setProcessEnvironment( env );

    QString program;
    QStringList arguments;
    QString pathTozLeaf;
    if ( settings.value( "ztree_installation_directory" ).toString().endsWith( '/' ) ) {
        pathTozLeaf = QString{ settings.value( "ztree_installation_directory" ).toString()
                               + "zTree_" + argzLeafSettings[ 1 ] + "/zleaf.exe" };
    } else {
        pathTozLeaf = QString{ settings.value( "ztree_installation_directory" ).toString()
                               + "/zTree_" + argzLeafSettings[ 1 ] + "/zleaf.exe" };
    }
#ifdef Q_OS_UNIX
    // The 'taskset' workaround is needed because of random crashes on startup of 'z-Leaf',
    // more details can be found in the winehq bug reports 35041 and 39404
    program = settings.value( "taskset_command", "/usr/bin/taskset" ).toString();
    arguments << "-c"  << "0" << settings.value( "wine_command", "/usr/bin/wine" ).toString()
              << pathTozLeaf ;
#else
    program = pathTozLeaf;
#endif
    arguments << "/server" << argzLeafSettings[ 2 ]
              << "/channel" << QString::number( argzLeafSettings[ 3 ].toUInt() - 7000 );

    if ( argzLeafSettings.count() == 5 ) {
        arguments << "/name" << argzLeafSettings[ 4 ];
    }

    qDebug() << program << arguments.join( ' ' );
    startzLeafProcess.start( program, arguments );
}

void ccServerConnector::TryConnect() {
    if ( webSocket.state() != QAbstractSocket::UnconnectedState ) {
        return;
    }

    webSocket.open( QUrl{ QString{ "wss://"
                                   + settings.value( "server_ip", "127.0.0.1" ).toString()
                                   + ":"
                                   + settings.value( "server_port", "0" ).toString() } } );
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
