#ifndef SIMPLENAM_H
#define SIMPLENAM_H

#include <QObject>
#include <QNetworkProxy>

class QNetworkAccessManager;
class SimpleNAM : public QObject
{
    Q_OBJECT
public:
    explicit SimpleNAM(QObject *parent = nullptr);

    void setProxy(const QNetworkProxy &proxy);
    QByteArray get(const QUrl &url, bool *ok = nullptr);
    QByteArray post(const QUrl &url, const QByteArray &data, bool *ok = nullptr);

private slots:
    void onFinished();

private:
    QByteArray send(const QString &method, const QUrl &url, bool *ok = nullptr);

private:
    QNetworkAccessManager *mNam;
    QByteArray mTempData;
    bool mRunning;
};

#endif // SIMPLENAM_H
