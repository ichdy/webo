#include "simplenam.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>

SimpleNAM::SimpleNAM(QObject *parent) :
    QObject(parent),
    mRunning(false)
{
    mNam = new QNetworkAccessManager(this);
}

void SimpleNAM::setProxy(const QNetworkProxy &proxy)
{
    mNam->setProxy(proxy);
}

QByteArray SimpleNAM::send(const QString &method, const QUrl &url, bool *ok)
{
    if (mRunning)
        return QByteArray();

    mRunning = true;
    QNetworkRequest request;
    request.setUrl(url);

    QNetworkReply *reply = 0;
    if (method == "GET")
        reply = mNam->get(request);
    else if (method == "POST")
        reply = mNam->post(request, mTempData);
    mTempData.clear();

    if (!reply)
        return QByteArray();

    connect(reply, &QNetworkReply::finished, this, &SimpleNAM::onFinished);

    while (mRunning)
        qApp->processEvents();

    bool success = reply->error() == QNetworkReply::NoError;
    if (ok)
        *ok = success;

    if (!success)
        return QByteArray();

    return reply->readAll();
}

void SimpleNAM::onFinished()
{
    mRunning = false;
}

QByteArray SimpleNAM::get(const QUrl &url, bool *ok)
{
    return send("GET", url, ok);
}

QByteArray SimpleNAM::post(const QUrl &url, const QByteArray &data, bool *ok)
{
    mTempData = data;
    return send("POST", url, ok);
}

