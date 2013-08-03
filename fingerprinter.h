#ifndef FINGERPRINTER_H
#define FINGERPRINTER_H

# include <QObject>
# include <QString>
# include <QStringList>
# include <QNetworkAccessManager>
# include <QNetworkReply>
# include <QScriptEngine>

# include <chromaprint.h>

# if defined(_WIN32) && !defined(__unix__)
#  include <stdint.h>
# endif

class Fingerprinter : public QObject {

    Q_OBJECT

    /* methods =================================== */
public:
    Fingerprinter(QObject *parent = 0);

    void getMusicBrainzData(const QString &filename);

private:
    static QString getFingerprint(const QString &filename, int &duration);
    static int decode_audio_file(ChromaprintContext *chromaprint_ctx,
                                 const char *file_name, int max_length, int *duration);

    /* signals & slots =========================== */
signals:
    void receivedGoodAnswer(QString, QStringList, QStringList, QStringList);
    void status(QString, int);

private slots:
    void receiveNetAnswer(QNetworkReply *reply);
    void networkAccessChanged(QNetworkAccessManager::NetworkAccessibility na);

    /* attributes ================================ */
private:
    QHash<QNetworkReply*,QString> replyFilelocationHash;

    QNetworkAccessManager *nwaManager;
    QScriptEngine          scriptEngine;
};


#endif // FINGERPRINTER_H
