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
        qDebug() << s.errorString();
    }

    webSocket.ignoreSslErrors();
}

void ccServerConnector::OnTextMessageReceived( QString argMessage ) {
    qDebug() << argMessage;

    QStringList tempMessageSplit = argMessage.split( '|', QString::SkipEmptyParts, Qt::CaseSensitive );
    bool conversionSucceeded = false;
    int messageID = tempMessageSplit[ 0 ].toInt( &conversionSucceeded );
    if ( !conversionSucceeded ) {
        throw "Conversion to int failed";
    }

    switch ( messageID ) {
    case 0:
        Shutdown();
        break;
    case 1:
        StartzLeaf( argMessage );
        break;
    case 2:
        KillzLeaf();
        break;
    default:
        true;
    }
}

void ccServerConnector::OnWebSocketConnected() {
    connect( &webSocket, &QWebSocket::textMessageReceived,
             this, &ccServerConnector::OnTextMessageReceived );
    webSocket.sendTextMessage( settings.value( "server_connection_password", "password" ).toString() );
}

void ccServerConnector::SendMessage( const quint16 &argMessageID, const QString *argMessage ) {
    QString message;
    if ( argMessage ) {
        message = QString::number( argMessageID ) + "|" + *argMessage;
    } else {
        message = QString::number( argMessageID );
    }
    qint64 bytesSent = webSocket.sendTextMessage( message );
    delete argMessage;
}

void ccServerConnector::Shutdown() {
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

void ccServerConnector::StartzLeaf( const QString &argzLeafSettings ) {
    startzLeafProcess.setProcessEnvironment( env );

    QStringList zleafSettings = argzLeafSettings.split( '|', QString::SkipEmptyParts );

    QString program;
    QStringList arguments;
#ifdef Q_OS_UNIX
    program = settings.value( "wine_command", "/usr/bin/wine" ).toString();
    arguments.append( settings.value( "ztree_installation_directory" ).toString()
                      + "/zTree_" + zleafSettings[ 1 ] + "/zleaf.exe" );
#else
    program = QString{ settings.value( "ztree_installation_directory" ).toString()
                       + "/zTree_" + zleafSettings[ 1 ] + "/zleaf.exe" };
#endif
    arguments << "/server" << zleafSettings[ 2 ]
              << "/channel" << QString::number( zleafSettings[ 3 ].toUInt() - 7000 );

    if ( zleafSettings.count() == 5 ) {
        arguments << "/name" << zleafSettings[ 4 ];
    }

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
