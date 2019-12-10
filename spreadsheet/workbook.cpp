#include "workbook.h"

#include "worksheet.h"
#include "xlsxfilehandler.h"

SpreadsheetWorkbook::SpreadsheetWorkbook(QObject *parent) :
    QObject(parent),
    mActiveIndex(-1)
{
}

SpreadsheetWorksheet *SpreadsheetWorkbook::createWorksheet(const QString &name)
{
    SpreadsheetWorksheet *worksheet = new SpreadsheetWorksheet(this);
    worksheet->setName(name);
    mWorksheetList << worksheet;
    connect(worksheet, SIGNAL(cellChanged(SpreadsheetCell*)), SLOT(onCellChanged(SpreadsheetCell*)));
    connect(worksheet, SIGNAL(rowHeightChanged(quint32)), SLOT(onRowHeightChanged(quint32)));
    connect(worksheet, SIGNAL(columnWidthChanged(quint32)), SLOT(onColumnChanged(quint32)));

    if (mActiveIndex == -1)
        mActiveIndex = 0;

    emit worksheetCreated(worksheet);
    return worksheet;
}

void SpreadsheetWorkbook::clear()
{
    QMutableListIterator<SpreadsheetWorksheet *> iterator(mWorksheetList);
    while (iterator.hasNext()) {
        iterator.next();

        delete iterator.value();
        iterator.remove();
    }

    mActiveIndex = -1;
    emit cleared();
}

bool SpreadsheetWorkbook::load(const QString &filename)
{
    clear();
    return append(filename);
}

bool SpreadsheetWorkbook::append(const QString &filename)
{
    emit loadStarted();

    bool result(false);
    QList<SpreadsheetAbstractFileHandler *> handlerList;
    handlerList << new SpreadsheetXlsxFileHandler(this);

    QString fileExtension = filename.section(".", -1).toLower();
    SpreadsheetAbstractFileHandler *handler(0);

    QMutableListIterator<SpreadsheetAbstractFileHandler *> iterator(handlerList);
    while (iterator.hasNext()) {
        iterator.next();

        SpreadsheetAbstractFileHandler *handlerTemp = iterator.value();
        if (handlerTemp->extension().contains(fileExtension))
            handler = handlerTemp;
        else
            delete iterator.value();

        iterator.remove();
    }

    if (handler) {
        QString workingPath = QDir::toNativeSeparators(filename);
        result = handler->open(workingPath);
        delete handler;
    }

    emit loadFinished();
    return result;
}

bool SpreadsheetWorkbook::save(const QString &filename)
{
    bool result(false);

    QList<SpreadsheetAbstractFileHandler *> handlerList;
    handlerList << new SpreadsheetXlsxFileHandler(this);

    QString fileExtension = filename.section(".", -1).toLower();
    SpreadsheetAbstractFileHandler *handler(0);

    QMutableListIterator<SpreadsheetAbstractFileHandler *> iterator(handlerList);
    while (iterator.hasNext()) {
        iterator.next();

        SpreadsheetAbstractFileHandler *handlerTemp = iterator.value();
        if (handlerTemp->extension().contains(fileExtension))
            handler = handlerTemp;
        else
            delete iterator.value();

        iterator.remove();
    }

    if (handler) {
        result = handler->save(filename);
        delete handler;
    }

    return result;
}

SpreadsheetWorksheet *SpreadsheetWorkbook::activeWorksheet()
{
    return mWorksheetList[mActiveIndex];
}

int SpreadsheetWorkbook::numWorksheets()
{
    return mWorksheetList.size();
}

void SpreadsheetWorkbook::setActiveWorksheet(int index)
{
    if (index >= mWorksheetList.size())
        return;

    mActiveIndex = index;
    emit activeWorksheetChanged(activeWorksheet());
}

SpreadsheetWorksheet *SpreadsheetWorkbook::worksheet(int index)
{
    return mWorksheetList[index];
}

quint64 SpreadsheetWorkbook::height()
{
    return activeWorksheet()->height();
}

quint64 SpreadsheetWorkbook::width()
{
    return activeWorksheet()->width();
}

void SpreadsheetWorkbook::onCellChanged(SpreadsheetCell *cell)
{
    SpreadsheetWorksheet *worksheet = static_cast<SpreadsheetWorksheet *>(QObject::sender());
    emit cellChanged(worksheet, cell);
}

void SpreadsheetWorkbook::onRowHeightChanged(quint32 row)
{
    SpreadsheetWorksheet *worksheet = static_cast<SpreadsheetWorksheet *>(QObject::sender());
    emit rowHeightChanged(worksheet, row);
}

void SpreadsheetWorkbook::onColumnChanged(quint32 column)
{
    SpreadsheetWorksheet *worksheet = static_cast<SpreadsheetWorksheet *>(QObject::sender());
    emit columnWidthChanged(worksheet, column);
}
