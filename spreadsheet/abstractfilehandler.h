#ifndef SPREADSHEETABSTRACTFILEHANDLER_H
#define SPREADSHEETABSTRACTFILEHANDLER_H

#include <QObject>

class SpreadsheetWorkbook;
class SpreadsheetAbstractFileHandler : public QObject
{
    Q_OBJECT
public:
    explicit SpreadsheetAbstractFileHandler(SpreadsheetWorkbook *workbook, QObject *parent = 0) : QObject(parent), mWorkbook(workbook) {}
    virtual ~SpreadsheetAbstractFileHandler() {}

    virtual QStringList extension() = 0;
    virtual bool open(const QString &filename) = 0;
    virtual bool save(const QString &filename) = 0;

protected:
    SpreadsheetWorkbook *mWorkbook;
};

#endif // SPREADSHEETABSTRACTFILEHANDLER_H
