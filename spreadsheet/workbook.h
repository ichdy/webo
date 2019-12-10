#ifndef SPREADSHEETWORKBOOK_H
#define SPREADSHEETWORKBOOK_H

#include <QObject>
#include <QRectF>
#include <QList>
#include <QDir>
#include "cell.h"

class QPainter;
class SpreadsheetWorksheet;
class SpreadsheetWorkbook : public QObject
{
    Q_OBJECT
public:
    explicit SpreadsheetWorkbook(QObject *parent = 0);

    SpreadsheetWorksheet *createWorksheet(const QString &name = QString());
    void setWorkingDirectory(const QString &path);

    Q_INVOKABLE bool load(const QString &filename);
    Q_INVOKABLE bool append(const QString &filename);
    Q_INVOKABLE bool save(const QString &filename);

    Q_INVOKABLE SpreadsheetWorksheet *activeWorksheet();
    Q_INVOKABLE void setActiveWorksheet(int index);
    Q_INVOKABLE int numWorksheets();
    Q_INVOKABLE SpreadsheetWorksheet *worksheet(int index);

    Q_INVOKABLE quint64 height();
    Q_INVOKABLE quint64 width();

    Q_INVOKABLE void clear();

signals:
    void loadStarted();
    void loadFinished();
    void cleared();
    void worksheetCreated(SpreadsheetWorksheet *worksheet);
    void activeWorksheetChanged(SpreadsheetWorksheet *worksheet);
    void cellChanged(SpreadsheetWorksheet *worksheet, SpreadsheetCell *cell);
    void rowHeightChanged(SpreadsheetWorksheet *worksheet, quint32 row);
    void columnWidthChanged(SpreadsheetWorksheet *worksheet, quint32 column);

private slots:
    void onCellChanged(SpreadsheetCell *cell);
    void onRowHeightChanged(quint32 row);
    void onColumnChanged(quint32 column);

private:
    QList<SpreadsheetWorksheet *> mWorksheetList;
    int mActiveIndex;
    QDir mWorkingDirectory;
};


#endif // SPREADSHEETWORKBOOK_H
