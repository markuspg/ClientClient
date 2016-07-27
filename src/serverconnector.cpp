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

#include <QDebug>

#include "serverconnector.h"

cc::ServerConnector::ServerConnector( QObject *argParent) :
    QObject{ argParent },
    connectionIntervalTimer{ this },
    env{ QProcessEnvironment::systemEnvironment() },
    settings{ "Economic Laboratory", "ClientClient", this },
    webSocket{ QStringLiteral( "client" ),
               QWebSocketProtocol::Version13, this }
{
    qDebug() << "Initializing 'ccServerConnector'";
    connect( &webSocket, &QWebSocket::connected,
             this, &ServerConnector::OnWebSocketConnected );
    connect( &webSocket, &QWebSocket::textMessageReceived,
             this, &ServerConnector::OnTextMessageReceived );
    typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect( &webSocket, static_cast<sslErrorsSignal>(&QWebSocket::sslErrors),
             this, &ServerConnector::OnSSLErrors );
    connect( &webSocket, SIGNAL( connected() ),
             &connectionIntervalTimer, SLOT( stop() ) );
    connect( &webSocket, SIGNAL( disconnected() ),
             &connectionIntervalTimer, SLOT( start() ) );

    connectionIntervalTimer.setInterval( 3000 );
    connectionIntervalTimer.start();
    connect( &connectionIntervalTimer, &QTimer::timeout,
             this, &ServerConnector::TryConnect );

    connect( &startzLeafProcess, &QProcess::started,
             this, &ServerConnector::zleafStartedSuccessfully );
    connect( &startzLeafProcess, SIGNAL( finished( int, QProcess::ExitStatus ) ),
             this, SLOT( zleafClosed( int, QProcess::ExitStatus ) ) );
    qDebug() << "Finished initializing 'ServerConnector'";
}

void cc::ServerConnector::KillzLeaf() {
    QProcess killzLeafProcess;
    killzLeafProcess.setProcessEnvironment( env );
    qDebug() << settings.value( "killall_command", "/usr/bin/killall" ).toString()
             << "zleaf.exe";
    killzLeafProcess.startDetached( settings.value( "killall_command",
                                                    "/usr/bin/killall" ).toString(),
                                    QStringList{ "zleaf.exe" } );
}

void cc::ServerConnector::OnSSLErrors( const QList<QSslError> &argErrors ) {
    Q_UNUSED( argErrors );

    for ( auto s: argErrors ) {
        qWarning() << "SSL error: " << s.errorString();
    }

    webSocket.ignoreSslErrors();
}

void cc::ServerConnector::OnTextMessageReceived( QString argMessage ) {
    qDebug() << "Received message: " <<  argMessage;

    QStringList tempMessageSplit = argMessage.split( '|', QString::SkipEmptyParts,
                                                     Qt::CaseSensitive );

    if ( tempMessageSplit[ 0 ] == "StartzLeaf" ) {
        StartzLeaf( tempMessageSplit );
        return;
    } else if ( tempMessageSplit[ 0 ] == "KillzLeaf" ) {
        KillzLeaf();
        return;
    } else if ( tempMessageSplit[ 0 ] == "Shutdown" ) {
        Shutdown();
        return;
    }

    qWarning() << "Unhandled message received from server";
}

void cc::ServerConnector::OnWebSocketConnected() {
    qDebug() << "Web socket connection attempt was successful, sending password";
    webSocket.sendTextMessage( settings.value( "server_connection_password",
                                               "password" ).toString() );
}

void cc::ServerConnector::SendMessage( const quint16 &argMessageID,
                                       const QString *argMessage ) {
    QString message;
    if ( argMessage ) {
        message = QString::number( argMessageID ) + "|" + *argMessage;
    } else {
        message = QString::number( argMessageID );
    }
    webSocket.sendTextMessage( message );
    qDebug() << "Sent message" << message << "to server";
    delete argMessage;
}

void cc::ServerConnector::Shutdown() {
    qDebug() << "Attempting to shut down";
    webSocket.close( QWebSocketProtocol::CloseCodeNormal, "Shutting down" );

    QProcess shutdownProcess;
    shutdownProcess.setProcessEnvironment( env );

    shutdownProcess.startDetached( "sudo shutdown -hP now" );

    this->deleteLater();
}

void cc::ServerConnector::StartzLeaf( const QStringList &argzLeafSettings ) {
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

    // The 'taskset' workaround is needed because of random crashes on startup of 'z-Leaf',
    // more details can be found in the winehq bug reports 35041 and 39404
    program = settings.value( "taskset_command", "/usr/bin/taskset" ).toString();
    arguments << "-c"  << "0" << settings.value( "wine_command", "/usr/bin/wine" ).toString()
              << pathTozLeaf ;
    arguments << "/server" << argzLeafSettings[ 2 ]
              << "/channel" << QString::number( argzLeafSettings[ 3 ].toUInt() - 7000 );

    if ( argzLeafSettings.count() == 5 ) {
        arguments << "/name" << argzLeafSettings[ 4 ];
    }

    qDebug() << program << arguments;
    startzLeafProcess.start( program, arguments );
}

void cc::ServerConnector::TryConnect() {
    qDebug() << "Attempting to connect to server"
             << settings.value( "server_ip", "127.0.0.1" ).toString()
             << "using port" << settings.value( "server_port", "0" ).toString();
    if ( webSocket.state() != QAbstractSocket::UnconnectedState ) {
        qDebug() << "The websocket is not in state 'UnconnectedState',"
                    " skipping connection attempt";
        return;
    }

    webSocket.open( QUrl{ QString{ "wss://"
                                   + settings.value( "server_ip", "127.0.0.1" ).toString()
                                   + ":"
                                   + settings.value( "server_port", "0" ).toString() } } );
}

void cc::ServerConnector::zleafClosed( const int &argExitCode,
                                       const QProcess::ExitStatus &argExitStatus ) {
    QString *message = new QString{ tr( "z-Leaf closed with exit code '%1' and exit status '%2" )
                                    .arg( argExitCode ).arg( argExitStatus ) };
    qDebug() << *message;
    SendMessage( 1, message );
}

void cc::ServerConnector::zleafStartedSuccessfully() {
    qDebug() << "z-Leaf started successfully";
    SendMessage( 0 );
}
