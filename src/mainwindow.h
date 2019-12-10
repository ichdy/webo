#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QProcess>

class QTreeView;
class QComboBox;
class QPushButton;
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void download();
    void refresh();
    void about();
    void openData();
    void run();

private:
    void onItemCurrentIndexChanged();

private:
    QVariantList scanFile(const QString &dirPath);
    QVariantMap parseScriptInfo(const QString &filename);
    void enableControls(bool value);

private:
    QString mAppDirPath;
    QTreeView *mView;
    QComboBox *mBrowserComboBox;
    QPushButton *mDataButton;
    QPushButton *mRunButton;
    bool mAutomationRunning;

};

#endif // MAINWINDOW_H
