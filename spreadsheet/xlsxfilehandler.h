#ifndef SPREADSHEETXLSXFILEHANDLER_H
#define SPREADSHEETXLSXFILEHANDLER_H

#include "cell.h"
#include "abstractfilehandler.h"

class SpreadsheetXlsxFileHandler : public SpreadsheetAbstractFileHandler
{
    Q_OBJECT
public:
    explicit SpreadsheetXlsxFileHandler(SpreadsheetWorkbook *workbook, QObject *parent=0);

    QStringList extension();
    bool open(const QString &filename);
    bool save(const QString &filename);

private:
    Spreadsheet::BorderType toBorderType(const QString &style);
};

#endif // SPREADSHEETXLSXFILEHANDLER_H
