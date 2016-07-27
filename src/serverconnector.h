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

#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <QHostInfo>
#include <QObject>
#include <QProcess>
#include <QWebSocket>

#include <QtNetwork>

namespace cc {

class ServerConnector final : public QObject {
    Q_OBJECT
public:
    explicit ServerConnector( QObject *argParent = nullptr );
    ServerConnector( const ServerConnector& ) = delete;
    ServerConnector& operator=( const ServerConnector& ) = delete;
    ServerConnector( ServerConnector&& ) = delete;
    ServerConnector& operator=( ServerConnector&& ) = delete;

signals:

public slots:

private:
    QTimer connectionIntervalTimer;
    const QProcessEnvironment env;
    QSettings settings;
    QProcess startzLeafProcess;
    QWebSocket webSocket;

    void KillzLeaf();
    void Shutdown();
    void StartzLeaf( const QStringList &argzLeafSettings );

private slots:
    void OnSSLErrors( const QList< QSslError > &argErrors );
    void OnTextMessageReceived( QString argMessage );
    void OnWebSocketConnected();
    void SendMessage( const quint16 &argMessageID, const QString *argMessage = nullptr );
    void TryConnect();
    void zleafClosed( const int &argExitCode, const QProcess::ExitStatus &argExitStatus );
    void zleafStartedSuccessfully();
};

}

#endif // SERVERCONNECTOR_H
