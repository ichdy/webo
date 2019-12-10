#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>

class SimpleNAM;
class QTreeView;
class DownloadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DownloadDialog(QWidget *parent = nullptr);

    bool isScriptNeedRefresh();

public slots:
    void refresh();

private slots:
    void onItemCurrentIndexChanged();
    void onDownloadClicked();

private:
    bool updateWebo();
    bool downloadToScripts(const QUrl &url);
    bool checkCurrentDataNeedUpdate(const QString &data);

private:
    QHash<QString, QString> mDataMap;
    QTreeView *mView;
    SimpleNAM *mClient;
    QPushButton *mDownloadButton;
    bool mUpdated;
};

#endif // DOWNLOADDIALOG_H
