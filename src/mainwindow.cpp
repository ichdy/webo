#include "mainwindow.h"

#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QApplication>
#include <QGridLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include <QDebug>
#include <QToolButton>
#include <QJsonDocument>
#include <QTimer>

#include "aboutdialog.h"
#include "itemdelegate.h"
#include "parametersdialog.h"
#include "consoledialog.h"
#include "downloaddialog.h"
#include "nodejs.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    mAutomationRunning(false)
{
    mAppDirPath = qApp->applicationDirPath();

    QToolButton *downloadButton = new QToolButton;
    downloadButton->setAutoRaise(true);
    downloadButton->setIcon(QIcon(":/images/download.png"));
    downloadButton->setIconSize(QSize(16, 16));

    QToolButton *refreshButton = new QToolButton;
    refreshButton->setAutoRaise(true);
    refreshButton->setIcon(QIcon(":/images/refresh.png"));
    refreshButton->setIconSize(QSize(16, 16));

    QToolButton *aboutButton = new QToolButton;
    aboutButton->setAutoRaise(true);
    aboutButton->setIcon(QIcon(":/images/about.png"));
    aboutButton->setIconSize(QSize(16, 16));

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->addStretch();
    toolLayout->addWidget(downloadButton);
    toolLayout->addWidget(refreshButton);
    toolLayout->addWidget(aboutButton);

    ItemDelegate *itemDelegate = new ItemDelegate(this);

    mView = new QTreeView;
    mView->setItemDelegate(itemDelegate);
    mView->header()->hide();
    mView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mBrowserComboBox = new QComboBox;
    mBrowserComboBox->addItem("Google Chrome", "chrome");
    mBrowserComboBox->addItem("Mozilla Firefox", "firefox");

    mDataButton = new QPushButton("Data");
    mRunButton = new QPushButton("Run");

    mDataButton->setIcon(QIcon(":/images/doc.png"));
    mRunButton->setIcon(QIcon(":/images/play.png"));

    mDataButton->setIconSize(QSize(12, 12));
    mRunButton->setIconSize(QSize(12, 12));

    mDataButton->setEnabled(false);
    mRunButton->setEnabled(false);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(new QLabel("Browser"));
    buttonLayout->addWidget(mBrowserComboBox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(mDataButton);
    buttonLayout->addWidget(mRunButton);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(toolLayout);
    layout->addWidget(mView);
    layout->addLayout(buttonLayout);
    setLayout(layout);
    resize(300, 500);

    // connect(mProcess, (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished, this, &MainWindow::onProcessFinished);
    connect(downloadButton, &QToolButton::clicked, this, &MainWindow::download);
    connect(refreshButton, &QToolButton::clicked, this, &MainWindow::refresh);
    connect(aboutButton, &QToolButton::clicked, this, &MainWindow::about);
    connect(mDataButton, &QPushButton::clicked, this, &MainWindow::openData);
    connect(mRunButton, &QPushButton::clicked, this, &MainWindow::run);

    refresh();
}

MainWindow::~MainWindow() {}

void MainWindow::onItemCurrentIndexChanged()
{
    QModelIndex index = mView->currentIndex();
    QVariantMap dataMap = index.data(Qt::UserRole +2).toMap();
    mDataButton->setEnabled(!dataMap["data"].isNull());
    mRunButton->setEnabled(!dataMap["filename"].isNull());
}

void MainWindow::enableControls(bool value)
{
    if (!value) {
        mBrowserComboBox->setEnabled(false);
        mDataButton->setEnabled(false);
        mRunButton->setEnabled(false);
    }
    else {
        mBrowserComboBox->setEnabled(true);
        onItemCurrentIndexChanged();
    }
}

void MainWindow::openData()
{
    QModelIndex index = mView->currentIndex();
    QVariantMap dataMap = index.data(Qt::UserRole +2).toMap();
    QString path = dataMap["path"].toString();
    QString data = dataMap["data"].toString();

    QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + data));
}

void MainWindow::run()
{
    enableControls(false);

    NodeJs nodejs;

    QModelIndex index = mView->currentIndex();
    QVariantMap dataMap = index.data(Qt::UserRole +2).toMap();
    QString name = dataMap["name"].toString();
    QString path = dataMap["path"].toString();
    QString filename = dataMap["filename"].toString();
    QString browser = mBrowserComboBox->currentData().toString();

    QByteArray json;
    {
        QStringList args;
        args << path + "/" + filename << "--parameters";
        nodejs.setArguments(args);
        json = nodejs.startAndWaitForFinished();
    }

    QStringList args;
    QVariantList parameters = QJsonDocument::fromJson(json).toVariant().toList();
    if (parameters.size()) {
        ParametersDialog dialog(json, this);
        dialog.setWindowTitle(name);
        if (dialog.exec() == QDialog::Accepted) {
            args << path + "/" + filename << browser << dialog.arguments();
        }
        else {
            enableControls(true);
            return;
        }
    }
    else
        args << path + "/" + filename << browser;

    mAutomationRunning = true;
    ConsoleDialog consoleDialog(&nodejs, this);

    nodejs.setArguments(args);
    QTimer::singleShot(0, &nodejs, &NodeJs::start);
    consoleDialog.exec();
    enableControls(true);
}

void MainWindow::download()
{
    DownloadDialog dialog(this);
    dialog.exec();

    if (dialog.isScriptNeedRefresh())
        refresh();
}

void MainWindow::refresh()
{
    delete mView->model();
    QStandardItemModel *model = new QStandardItemModel(0, 1, mView);

    QMap<QString, QStandardItem *> groupMap;

    QVariantList infoList = scanFile(mAppDirPath + "/scripts");
    foreach (const QVariant &file, infoList) {
        const QVariantMap infoMap = file.toMap();
        QString group = infoMap["group"].toString();

        if (!groupMap.contains(group)) {
            QStandardItem *gItem = new QStandardItem(group);
            gItem->setData(0, Qt::UserRole +1);
            groupMap[group] = gItem;
        }

        QStandardItem *item = new QStandardItem(infoMap["name"].toString());
        item->setData(1, Qt::UserRole +1);
        item->setData(infoMap, Qt::UserRole +2);

        groupMap[group]->appendRow(item);
    }

    QMapIterator<QString, QStandardItem *> iterator(groupMap);
    while (iterator.hasNext()) {
        iterator.next();
        model->appendRow(iterator.value());
    }

    mView->setModel(model);
    mView->expandAll();

    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onItemCurrentIndexChanged);
}

void MainWindow::about()
{
    AboutDialog dialog(this);
    dialog.exec();
}

QVariantList MainWindow::scanFile(const QString &dirPath)
{
    QVariantList list;

    QFileInfoList infoList = QDir(dirPath).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &info, infoList) {
        if (info.isDir())
            list << scanFile(info.canonicalFilePath());
        else {
            if (info.fileName().section(".", -1) != "js")
                continue;

            QVariantMap fileMap = parseScriptInfo(info.canonicalFilePath());
            list << fileMap;
        }
    }

    return list;
}

QVariantMap MainWindow::parseScriptInfo(const QString &filename)
{
    QStringList args;
    args << filename << "--info";

    NodeJs nodejs;
    nodejs.setArguments(args);
    QString stdOut = nodejs.startAndWaitForFinished();
    QString group, name, version, data;
    {
        QStringList infoList = stdOut.split("\n");
        QString key, value;
        foreach (const QString &info, infoList) {
            QStringList tmpList = info.split(":");
            key = tmpList[0];
            value = tmpList.size() > 1? tmpList[1] : QString();

            if (key == "Group")
                group = value;
            else if (key == "Name")
                name = value;
            else if (key == "Version")
                version = value;
            else if (key == "Data")
                data = value;
        }
    }

    QFileInfo info(filename);

    QVariantMap fileMap;
    fileMap["path"] = info.canonicalPath();
    fileMap["filename"] = info.fileName();
    fileMap["group"] = group;
    fileMap["name"] = name;
    fileMap["version"] = version;
    if (!data.isEmpty())
        fileMap["data"] = data;

    return fileMap;
}
