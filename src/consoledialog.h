#ifndef CONSOLEDIALOG_H
#define CONSOLEDIALOG_H

#include <QDialog>

class QProcess;
class QTextEdit;
class NodeJs;
class ConsoleDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConsoleDialog(NodeJs *nodejs, QWidget *parent = nullptr);

private slots:
    void onReadyReadStdOut();
    void onReadyReadStdErr();

private:
    QProcess *mProcess;
    QTextEdit *mEdit;
};

#endif // CONSOLEDIALOG_H
