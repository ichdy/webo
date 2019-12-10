#include "nodejs.h"

#include <QCoreApplication>
#include <QProcess>

NodeJs::NodeJs(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_WIN
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value("PATH");
    QString nodePath = QString("%1/node").arg(qApp->applicationDirPath());
    env.insert("PATH", QString("%1%2").arg(nodePath).arg(path.isEmpty()? "" : path.prepend(";")));
    env.insert("NODE_PATH", QString("%1/node_modules").arg(nodePath));

    mProcess.setProcessEnvironment(env);
    mProcess.setProgram(qApp->applicationDirPath() + "/node/node.exe");
#endif
#ifdef Q_OS_LINUX
    mProcess.setWorkingDirectory(qApp->applicationDirPath());
    mProcess.setProgram("node");
#endif
}

QProcess *NodeJs::process()
{
    return &mProcess;
}

void NodeJs::setArguments(const QStringList &args)
{
    mProcess.setArguments(args);
}

void NodeJs::start()
{
    mProcess.start();
}

QByteArray NodeJs::startAndWaitForFinished()
{
    mProcess.start();
    mProcess.waitForFinished();
    return mProcess.readAll();
}
