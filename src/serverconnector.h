#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <QObject>
#include <QProcess>

#include <QtNetwork>

class ccServerConnector : public QObject {
    Q_OBJECT
public:
    explicit ccServerConnector( QObject *argParent = nullptr );

signals:

public slots:

private:
    quint16 blockSize = 0;
    const QProcessEnvironment env;
    quint16 messageID = 0;
    QTcpSocket socket;

    void Shutdown();

private slots:
    void ReadMessage();
    void SendMessage( const QString &argMessage, const quint16 &argMessageID );
};

#endif // SERVERCONNECTOR_H
