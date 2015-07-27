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
    QTimer connectionIntervalTimer;
    const QProcessEnvironment env;
    quint16 messageID = 0;
    QSettings settings;
    QTcpSocket socket;
    QProcess startzLeafProcess;

    void KillzLeaf();
    void Shutdown();
    void StartzLeaf( const QString &argzLeafSettings );

private slots:
    void ReadMessage();
    void SendMessage( const quint16 &argMessageID, QString *argMessage = nullptr );
    void TryConnect();
    void zleafClosed( const int &argExitCode, const QProcess::ExitStatus &argExitStatus );
};

#endif // SERVERCONNECTOR_H
