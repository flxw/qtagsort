#ifndef FINGERPRINTER_H
#define FINGERPRINTER_H

# include <QObject>
# include <QString>
# include <QStringList>
# include <QNetworkAccessManager>
# include <QNetworkReply>
# include <QScriptEngine>

# include <chromaprint.h>

class Fingerprinter : public QObject {

    Q_OBJECT

    /* methods =================================== */
public:
    Fingerprinter(QObject *parent = 0);

    void getMusicBrainzData(const QString &filename);

private:
    static QString getFingerprint(const QString &filename, int &duration);
    static int decode_audio_file(ChromaprintContext *chromaprint_ctx, int16_t *buffer1, int16_t *buffer2,
                                 const char *file_name, int max_length, int *duration);

    /* signals & slots =========================== */
signals:
    void receivedGoodAnswer(QStringList, QStringList, QStringList);
    void status(QString, int);

private slots:
    void receiveNetAnswer(QNetworkReply *reply);
    void networkAccessChanged(QNetworkAccessManager::NetworkAccessibility na);

    /* attributes ================================ */
public:
private:
    QNetworkAccessManager *nwaManager;
    QScriptEngine scriptEngine;
};


#endif // FINGERPRINTER_H
