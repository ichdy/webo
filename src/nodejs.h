#ifndef NODEJS_H
#define NODEJS_H

#include <QObject>
#include <QProcess>

class NodeJs : public QObject
{
    Q_OBJECT
public:
    explicit NodeJs(QObject *parent = nullptr);

    void setArguments(const QStringList &args);

    QProcess *process();

public slots:
    void start();
    QByteArray startAndWaitForFinished();

private:
    QProcess mProcess;
};

#endif // NODEJS_H
