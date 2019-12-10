#include "consoledialog.h"

#include <QProcess>
#include <QGridLayout>
#include <QTextEdit>
#include <QIcon>
#include <QDateTime>

#include "nodejs.h"

ConsoleDialog::ConsoleDialog(NodeJs *nodejs, QWidget *parent) :
    QDialog(parent),
    mProcess(nodejs->process())
{
    mEdit = new QTextEdit;
    mEdit->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mEdit);
    setLayout(layout);

    setWindowIcon(QIcon(":/images/console.png"));
    setWindowTitle("Console");
    resize(500, 300);

    connect(mProcess, &QProcess::readyReadStandardOutput, this, &ConsoleDialog::onReadyReadStdOut);
    connect(mProcess, &QProcess::readyReadStandardError, this, &ConsoleDialog::onReadyReadStdErr);
}

void ConsoleDialog::onReadyReadStdOut()
{
    QDateTime now = QDateTime::currentDateTime();

    QStringList line = QString(mProcess->readAllStandardOutput()).split("\n");
    foreach (const QString &msg, line) {
        if (msg.isEmpty())
            continue;

        mEdit->append(QString("[%1] <font color='black'>%2</color>").arg(now.toString("dd/MM/yyyy HH:mm")).arg(msg));
    }
}

void ConsoleDialog::onReadyReadStdErr()
{
    QDateTime now = QDateTime::currentDateTime();
    QStringList line = QString(mProcess->readAllStandardError()).split("\n");
    foreach (const QString &msg, line) {
        if (msg.isEmpty())
            continue;

        mEdit->append(QString("[%1] <font color='red'>%2</color>").arg(now.toString("dd/MM/yyyy HH:mm")).arg(msg));
    }
}
