#include "downloaddialog.h"

#include <QCoreApplication>
#include <QTimer>
#include <QGridLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QSettings>

#include "simplenam.h"
#include "nodejs.h"

const QString host = "http://web-o.duckdns.org:8080";

DownloadDialog::DownloadDialog(QWidget *parent) :
    QDialog(parent),
    mUpdated(false)
{
    mClient = new SimpleNAM(this);

    QSettings settings(qApp->applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    {
        QString host = settings.value("Proxy/host").toString();
        quint16 port = quint16(settings.value("Proxy/port").toUInt());
        QString username = settings.value("Proxy/username").toString();
        QString password = settings.value("Proxy/password").toString();

        if (!host.isEmpty() && port) {
            QNetworkProxy proxy;
            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(host);
            proxy.setPort(port);
            proxy.setUser(username);
            proxy.setPassword(password);

            mClient->setProxy(proxy);
        }
    }

    mView = new QTreeView;
    // mView->setItemDelegate(itemDelegate);
    // mView->header()->hide();
    mView->header()->setStretchLastSection(true);
    mView->header()->setDefaultAlignment(Qt::AlignCenter);
    mView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mView->setAlternatingRowColors(true);

    mDownloadButton = new QPushButton("Download");

    mDownloadButton->setIcon(QIcon(":/images/download.png"));
    mDownloadButton->setIconSize(QSize(12, 12));
    mDownloadButton->setEnabled(false);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(mDownloadButton);
    buttonLayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mView);
    layout->addLayout(buttonLayout);
    setLayout(layout);
    resize(500, 300);

    connect(mDownloadButton, &QPushButton::clicked, this, &DownloadDialog::onDownloadClicked);

    QTimer::singleShot(0, this, &DownloadDialog::refresh);
}

bool DownloadDialog::isScriptNeedRefresh()
{
    return mUpdated;
}

bool DownloadDialog::updateWebo()
{
    bool ok(false);
    QByteArray json = mClient->get(QUrl(host + "/webo"), &ok);
    if (!ok)
        return false;

    QVariantMap updateMap = QJsonDocument::fromJson(json).toVariant().toMap();
    QString updateVersion = updateMap["version"].toString();


    QStringList args;
    args << qApp->applicationDirPath() + "/node/version.js";

    NodeJs nodejs;
    nodejs.setArguments(args);
    QString stdOut = nodejs.startAndWaitForFinished();
    QString version;
    {
        QStringList infoList = stdOut.split("\n");
        QString key, value;
        foreach (const QString &info, infoList) {
            QStringList tmpList = info.split(":");
            key = tmpList[0];
            value = tmpList.size() > 1? tmpList[1] : QString();

            if (key == "Version")
                version = value;
        }
    }

    if (updateVersion.toFloat() <= version.toFloat())
        return true;

    mUpdated = true;
    QFile o(qApp->applicationDirPath() + "/node/node_modules/webo.js");
    if (o.open(QIODevice::WriteOnly)) {
        o.write(QByteArray::fromBase64(QByteArray::fromPercentEncoding(updateMap["data"].toByteArray())));
        o.close();
    }

    return true;
}

void DownloadDialog::refresh()
{
    updateWebo();

    bool ok(false);
    QByteArray json = mClient->get(QUrl(host + "/list"), &ok);
    if (ok) {
        if (json.isEmpty()) {
            QMessageBox::warning(this, "Error", "Server sedang bermasalah.");
            return;
        }

        delete mView->model();
        QStandardItemModel *model = new QStandardItemModel(0, 2, mView);
        model->setHeaderData(0, Qt::Horizontal, "Name");
        model->setHeaderData(1, Qt::Horizontal, "Version");

        QMap<QString, QStandardItem *> groupMap;
        foreach (const QVariant &file, QJsonDocument::fromJson(json).toVariant().toMap()["data"].toList()) {
            const QVariantMap infoMap = file.toMap();
            QString group = infoMap["group"].toString();
            QString type = infoMap["type"].toString();

            if (type == "script") {
                if (!groupMap.contains(group)) {
                    QStandardItem *gItem = new QStandardItem(group);
                    gItem->setData(0, Qt::UserRole +1);
                    groupMap[group] = gItem;
                }

                QStandardItem *nameitem = new QStandardItem;
                nameitem->setText(infoMap["name"].toString());
                nameitem->setData(1, Qt::UserRole +1);
                nameitem->setData(infoMap, Qt::UserRole +2);

                QStandardItem *versionItem = new QStandardItem;
                versionItem->setTextAlignment(Qt::AlignCenter);
                versionItem->setText(infoMap["version"].toString());
                versionItem->setData(1, Qt::UserRole +1);
                versionItem->setData(infoMap, Qt::UserRole +2);

                QList<QStandardItem *> itemList;
                itemList << nameitem << versionItem;

                groupMap[group]->appendRow(itemList);
            }
            else if (type == "data")
                mDataMap[infoMap["filename"].toString()] = infoMap["version"].toString();
        }

        QMapIterator<QString, QStandardItem *> iterator(groupMap);
        while (iterator.hasNext()) {
            iterator.next();
            model->appendRow(iterator.value());
        }

        mView->setModel(model);
        mView->setColumnWidth(0, 330);
        mView->setColumnWidth(1, 50);
        mView->expandAll();

        connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DownloadDialog::onItemCurrentIndexChanged);
    }
    else
        QMessageBox::warning(this, "Error", "Gagal mendapatkan list, pastikan Anda terhubung dengan Internet.");
}

void DownloadDialog::onItemCurrentIndexChanged()
{
    QModelIndex index = mView->currentIndex();
    mDownloadButton->setEnabled(!index.data(Qt::UserRole +2).toMap()["filename"].isNull());
}

bool DownloadDialog::downloadToScripts(const QUrl &url)
{
    bool ok(false);
    QByteArray json = mClient->get(url, &ok);
    if (ok) {
        if (json.isEmpty()) {
            QMessageBox::warning(this, "Error", "Server sedang bermasalah.");
            return false;
        }

        QVariantMap fileMap = QJsonDocument::fromJson(json).toVariant().toMap();
        QString filename = fileMap["filename"].toString();

        if (filename.isEmpty()) {
            QMessageBox::warning(this, "Error", "Gagal saat mengambil data.");
            return false;
        }

        QDir scriptsDir(qApp->applicationDirPath() + "/scripts");
        if (!scriptsDir.exists())
            scriptsDir.mkpath(".");

        mUpdated = true;
        QFile o(qApp->applicationDirPath() + "/scripts/" + filename);
        if (o.open(QIODevice::WriteOnly)) {
            o.write(QByteArray::fromBase64(QByteArray::fromPercentEncoding(fileMap["data"].toByteArray())));
            o.close();
        }
        else {
            QMessageBox::warning(this, "Error", "Gagal menyimpan file.");
            return false;
        }
    }
    else {
        QMessageBox::warning(this, "Error", "Gagal menyimpan file, pastikan Anda terhubung dengan Internet atau coba lagi.");
        return false;
    }

    return true;
}

bool DownloadDialog::checkCurrentDataNeedUpdate(const QString &data)
{
    QString filepath = qApp->applicationDirPath() + "/scripts/" + data;
    QString  version;
    if (QFile::exists(filepath)) {
        QStringList args;
        args  << qApp->applicationDirPath() + "/node/spreadsheet.js" << filepath;

        NodeJs nodejs;
        nodejs.setArguments(args);
        QString stdOut = nodejs.startAndWaitForFinished();

        {
            QStringList infoList = stdOut.split("\n");
            QString key, value;
            foreach (const QString &info, infoList) {
                QStringList tmpList = info.split(":");
                key = tmpList[0];
                value = tmpList.size() > 1? tmpList[1] : QString();

                if (key == "Version")
                    version = value;
            }
        }
    }

    return (mDataMap[data].toFloat() > version.toFloat());
}

void DownloadDialog::onDownloadClicked()
{
    QModelIndex index = mView->currentIndex();
    QVariantMap infoMap = index.data(Qt::UserRole +2).toMap();
    QString type = infoMap["type"].toString();
    QString filename = infoMap["filename"].toString();
    QString filepath = qApp->applicationDirPath() + "/scripts/" + infoMap["data"].toString();

    if (!downloadToScripts(QUrl(host + "/download/" + filename)))
        return;

    if (type != "script")
        goto done;

    if (infoMap["data"].isNull())
        goto done;

    if (!checkCurrentDataNeedUpdate(infoMap["data"].toString()))
        goto done;

    if (QFile::exists(filepath)) {
        int res = QMessageBox::question(this, "Backup Data", "Data untuk script ini harus diupdate, data sebelumnya akan hilang. Backup data terlebih dahulu?");
        if (res == QMessageBox::Yes) {
            QString backuppath = QFileDialog::getSaveFileName(this, "Backup File", infoMap["data"].toString(), "Excel Files (*.xlsx)");
            if (!backuppath.isEmpty())
                QFile::copy(filepath, backuppath);
        }
    }

    downloadToScripts(QUrl(host + "/download/" + infoMap["data"].toString()));

done:
    QMessageBox::information(this, "Download", "Process Download Selesai");
}
