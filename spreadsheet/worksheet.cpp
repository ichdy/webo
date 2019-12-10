#include "worksheet.h"

#include <QPainter>
#include <QFontMetrics>
#include "cell.h"

int paddingX(2), paddingY(1);

void SpreadsheetDrawing::setPosition(const SpreadsheetCellPosition &pos)
{
    mPosition = pos;
}

void SpreadsheetDrawing::setOffset(const QPoint &offset)
{
    mOffset = offset;
}

void SpreadsheetDrawing::setSize(const QSize &size)
{
    mSize = size;
}

void SpreadsheetDrawing::setImage(const QImage &image)
{
    mImage = image;
}

SpreadsheetWorksheet::SpreadsheetWorksheet(QObject *parent) :
    QObject(parent),
    mMaxRow(0),
    mMaxColumn(0)
{
}

SpreadsheetWorksheet::~SpreadsheetWorksheet()
{
    {
        QMutableHashIterator<quint32, QHash<quint32, SpreadsheetCellBorderData *> > rowIterator(mCellBorderDataHash);
        while (rowIterator.hasNext()) {
            rowIterator.next();

            QMutableHashIterator<quint32, SpreadsheetCellBorderData *> columnIterator(rowIterator.value());
            while (columnIterator.hasNext()) {
                columnIterator.next();

                delete columnIterator.value();
                columnIterator.remove();
            }
            rowIterator.remove();
        }
    }

    {
        QMutableHashIterator<quint32, QHash<quint32, SpreadsheetCell *> > rowIterator(mCellHash);
        while (rowIterator.hasNext()) {
            rowIterator.next();

            QMutableHashIterator<quint32, SpreadsheetCell *> columnIterator(rowIterator.value());
            while (columnIterator.hasNext()) {
                columnIterator.next();

                delete columnIterator.value();
                columnIterator.remove();
            }
            rowIterator.remove();
        }
    }

    {
        QMutableListIterator<SpreadsheetDrawing *> iterator(mDrawingList);
        while (iterator.hasNext()) {
            iterator.next();

            delete iterator.value();
            iterator.remove();
        }
    }
}

void SpreadsheetWorksheet::setName(const QString &name)
{
    mName = name;
}

QString SpreadsheetWorksheet::name()
{
    return mName;
}

void SpreadsheetWorksheet::setDefaultFont(const QString &name, int point)
{
    QFont font;
    font.setFamily(name);
    font.setPointSize(point);

    mDefaultFont = font;
}

SpreadsheetCell *SpreadsheetWorksheet::cell(quint32 row, quint32 column)
{
    SpreadsheetCell *cell = mCellHash[row][column];
    if (!cell) {
        cell = new SpreadsheetCell(this, row, column);
        cell->setFont(mDefaultFont);
        mCellHash[row][column] = cell;
        connect(cell, &SpreadsheetCell::changed, this, &SpreadsheetWorksheet::onCellChanged);
    }

    mMaxRow = mMaxRow > row? mMaxRow : row;
    mMaxColumn = mMaxColumn > column? mMaxColumn : column;

    return cell;
}

SpreadsheetCellBorderData *SpreadsheetWorksheet::borderData(quint32 row, quint32 column)
{
    SpreadsheetCellBorderData *borderData = mCellBorderDataHash[row][column];
    if (!borderData) {
        borderData = new SpreadsheetCellBorderData;
        mCellBorderDataHash[row][column] = borderData;
    }

    return borderData;
}

SpreadsheetCell *SpreadsheetWorksheet::cell(const SpreadsheetCellPosition &pos)
{
    return cell(pos.row(), pos.column());
}

SpreadsheetCell *SpreadsheetWorksheet::cell(const QString &address)
{
    return cell(SpreadsheetCellPosition::fromCellAddress(address));
}

SpreadsheetCell *SpreadsheetWorksheet::cell(const QPoint &pos)
{
    qint64 x1(0), x2(0), y1(0), y2(0);

    SpreadsheetCell *result = 0;
    for (quint32 column=0; column<=mMaxColumn; column++) {
        x2 += columnWidth(column);
        if (pos.x() > x1 && pos.x() < x2) {
            for (quint32 row=0; row<=mMaxRow; row++) {
                y2 += rowHeight(row);
                if (pos.y() > y1 && pos.y() < y2)
                    result = cell(row, column);
                y1 = y2;
                if (result) break;
            }
        }
        x1 = x2;
        if (result) break;
    }

    if (!result)
        return 0;

    if (result->mMergedCell)
        return result->mMergedCell;

    return result;
}

bool SpreadsheetWorksheet::mergeCell(quint32 startRow, quint32 startColumn, quint32 endRow, quint32 endColumn)
{
    for (quint32 row=startRow; row <= endRow; row++) {
        for (quint32 column=startColumn; column <= endColumn; column++) {
            SpreadsheetCell *cell = mCellHash[row][column];
            if (!cell)
                continue;

            if (cell->mMergedCell)
                return false;
        }
    }

    SpreadsheetCell *startCell = cell(startRow, startColumn);
    startCell->mMergePosition = SpreadsheetCellPosition(endRow, endColumn);

    for (quint32 row=startRow; row <= endRow; row++) {
        for (quint32 column=startColumn; column <= endColumn; column++) {
            SpreadsheetCell *mergeCell = cell(row, column);

            if (mergeCell->row() != startRow)
                mergeCell->setBorder(Spreadsheet::Top, Spreadsheet::None);

            if (mergeCell->column() != startColumn)
                mergeCell->setBorder(Spreadsheet::Left, Spreadsheet::None);

            mergeCell->mMergedCell = startCell;
            if (mergeCell != startCell) {
                mergeCell->mValue = QVariant();
                mergeCell->mFont = QFont();
                mergeCell->mAlignment = Qt::AlignLeft | Qt::AlignBottom;
            }
        }
    }

    return true;
}

bool SpreadsheetWorksheet::mergeCell(const SpreadsheetCellPosition &start, const SpreadsheetCellPosition &end)
{
    return mergeCell(start.row(), start.column(), end.row(), end.column());
}

bool SpreadsheetWorksheet::mergeCell(const QString &start, const QString &end)
{
    return mergeCell(SpreadsheetCellPosition::fromCellAddress(start), SpreadsheetCellPosition::fromCellAddress(end));
}

void SpreadsheetWorksheet::setColumnWidth(quint32 column, qint64 width)
{
    mWidthHash[column] = width;
    emit columnWidthChanged(column);
}

void SpreadsheetWorksheet::setRowHeight(quint32 row, qint64 height)
{
    mHeightHash[row] = height;
    emit rowHeightChanged(row);
}

void SpreadsheetWorksheet::fitContent(quint32 _row, quint32 _column)
{
    SpreadsheetCell *cell = mCellHash[_row][_column];
    if (!cell)
        return;

    int flags = cell->alignment() == 0? (Qt::AlignLeft | Qt::AlignBottom) : cell->alignment();
    QString value = cell->value().toString();

    if (cell->mWordWarp)
        flags |= Qt::TextWordWrap;

    quint64 mergeWidth = columnWidth(_column) - (paddingX * 2);
    if (cell->mMergedCell) {
        for (quint32 column=cell->column() + 1; column <= cell->mMergePosition.column(); column++)
            mergeWidth += columnWidth(column);
    }

    QFontMetrics fontMetrics(cell->font());
    QRect boundingRect = fontMetrics.boundingRect(0, 0, mergeWidth, rowHeight(_row), flags, value);

    setRowHeight(_row, boundingRect.height() + (paddingY * 2));
}

void SpreadsheetWorksheet::fitContent(const QString &address)
{
    SpreadsheetCellPosition pos = SpreadsheetCellPosition::fromCellAddress(address);
    fitContent(pos.row(), pos.column());
}

quint64 SpreadsheetWorksheet::columnWidth(quint32 column)
{
    qint64 width = mWidthHash[column];
    if (width == -1)
        return 0;
    else if (width == 0)
        return 64;
    else
        return width;
}

quint64 SpreadsheetWorksheet::rowHeight(quint32 row)
{
    qint64 height = mHeightHash[row];
    if (height == -1)
        return 0;
    else if (height == 0)
        return 20;
    else
        return height;
}

quint64 SpreadsheetWorksheet::width()
{
    quint64 result(0);
    for (quint32 column=0; column <= mMaxColumn; column++)
        result += columnWidth(column);

    return result + 1;
}

quint64 SpreadsheetWorksheet::height()
{
    quint64 result(0);
    for (quint32 row=0; row <= mMaxRow; row++)
        result += rowHeight(row);

    return result + 1;
}

void SpreadsheetWorksheet::insertRow(quint32 row)
{
    if (row > mMaxRow)
        return;

    for (quint32 i=mMaxRow; i>=row; i--) {
        mCellBorderDataHash[i+1] = mCellBorderDataHash[i];
        mCellBorderDataHash[i].clear();

        if (i == row) {
            QHash<quint32, SpreadsheetCellBorderData *> &borderHashNew = mCellBorderDataHash[i+1];
            for (quint32 column=0; column<=mMaxColumn; column++) {
                SpreadsheetCellBorderData *borderDataNew = borderHashNew[column];
                if (!borderDataNew)
                    continue;

                Spreadsheet::BorderType type = borderDataNew->top()->type();
                borderDataNew->top()->set(Spreadsheet::None);

                if (type == Spreadsheet::None)
                    continue;

                SpreadsheetCellBorderData *borderDataOld = borderData(i, column);
                borderDataOld->top()->set(type);
            }
        }

        mCellHash[i+1] = mCellHash[i];
        mCellHash[i].clear();

        QHash<quint32, SpreadsheetCell *> &cellHash = mCellHash[i+1];
        for (quint32 column=0; column<=mMaxColumn; column++) {
            SpreadsheetCell *cell = cellHash[column];
            if (!cell)
                continue;

            cell->mPosition = SpreadsheetCellPosition(cell->mPosition.row() + 1, cell->mPosition.column());
            cell->mMergePosition = SpreadsheetCellPosition(cell->mMergePosition.row() + 1, cell->mMergePosition.column());
        }

        quint64 height = mHeightHash[i];
        mHeightHash[i] = 0;
        mHeightHash[i+1] = height;

        if (i == 0)
            break;
    }

    foreach (SpreadsheetDrawing *drawing, mDrawingList) {
        SpreadsheetCellPosition pos = drawing->position();
        if (pos.row() < row)
            continue;

        drawing->setPosition(SpreadsheetCellPosition(pos.row() + 1, pos.column()));
    }

    mMaxRow += 1;
    emit rowHeightChanged(row);
}

void SpreadsheetWorksheet::addDrawing(SpreadsheetDrawing *drawing)
{
    mDrawingList << drawing;
}

void SpreadsheetWorksheet::findAndReplace(const QString &from, const QString &to)
{
    for (quint32 row=0; row <= mMaxRow; row++) {
        for (quint32 column=0; column <= mMaxColumn; column++) {
            findAndReplace(from, to, row, column);
        }
    }
}

void SpreadsheetWorksheet::findAndReplace(const QString &from, const QString &to, quint32 row, quint32 column)
{
    SpreadsheetCell *cell = mCellHash[row][column];
    if (!cell)
        return;

    cell->setValue(cell->value().toString().replace(from, to));
}

void SpreadsheetWorksheet::findAndReplace(const QString &from, const QString &to, const SpreadsheetCellPosition &pos)
{
    findAndReplace(from, to, pos.row(), pos.column());
}

void SpreadsheetWorksheet::findAndReplace(const QString &from, const QString &to, const QString &address)
{
    findAndReplace(from, to, SpreadsheetCellPosition::fromCellAddress(address));
}

void SpreadsheetWorksheet::onCellChanged()
{
    SpreadsheetCell *cell = static_cast<SpreadsheetCell *>(QObject::sender());
    emit cellChanged(cell);
}

void SpreadsheetWorksheet::render(QPainter *painter, const SpreadsheetCellPosition &start, const SpreadsheetCellPosition &end, double scale)
{
    qreal startX(0), startY(0), availableWidth(0), availableHeight(0);
    {
        quint32 tempX(0), tempY(0);
        for (quint32 column=0; column<=end.column(); column++) {
            quint32 tempWidth = columnWidth(column);
            if (column == start.column())
                startX = tempX;

            if (column >= start.column() && column <= end.column())
                availableWidth += tempWidth;

            tempX += tempWidth;
        }

        for (quint32 row=0; row<=end.row(); row++) {
            quint32 tempHeight = rowHeight(row);
            if (row == start.row())
                startY = tempY;

            if (row >= start.row() && row <= end.row())
                availableHeight += tempHeight;

            tempY += tempHeight;
        }
    }

    painter->save();
    painter->scale(scale, scale);
    painter->setClipRect(0, 0, availableWidth + 1, availableHeight + 1);
    painter->translate(-startX, - startY);

    // draw border
    painter->save();

    {
        int y(0);
        for (quint32 row=0; row <= (mMaxRow + 1); row++) {
            quint64 height = rowHeight(row);

            int x(0);
            for (quint32 column=0; column <= (mMaxColumn + 1); column++) {
                quint64 width = columnWidth(column);

                SpreadsheetCellBorderData *borderData = mCellBorderDataHash[row][column];
                if (borderData) {
                    if (borderData && width != 0 && height != 0) {
                        SpreadsheetCellBorder *topBorder = borderData->top();
                        if (column != (mMaxColumn + 1) && topBorder->type() != Spreadsheet::None) {
                            painter->save();
                            painter->setPen(borderPen(topBorder->type()));
                            QPoint startPoint, endPoint;
                            startPoint.setX(x); startPoint.setY(y);
                            endPoint.setX(x + width); endPoint.setY(y);
                            painter->drawLine(startPoint, endPoint);
                            painter->restore();
                        }

                        SpreadsheetCellBorder *leftBorder = borderData->left();
                        if (row != (mMaxRow + 1) && leftBorder->type() != Spreadsheet::None) {
                            painter->save();
                            painter->setPen(borderPen(leftBorder->type()));
                            QPoint startPoint, endPoint;
                            startPoint.setX(x); startPoint.setY(y);
                            endPoint.setX(x); endPoint.setY(y + height);
                            painter->drawLine(startPoint, endPoint);
                            painter->restore();
                        }
                    }
                }

                x += width;
            }

            y += height;
        }
    }
    painter->restore();

    // draw text
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    {
        int y(0);
        for (quint32 row=0; row <= mMaxRow; row++) {
            quint64 height = rowHeight(row);

            int x(0);
            for (quint32 column=0; column <= mMaxColumn; column++) {
                quint64 width = columnWidth(column);

                SpreadsheetCell *cell = mCellHash[row][column];
                if (cell && width != 0 && height != 0) {
                    painter->save();
                    painter->setFont(cell->font());

                    QPen pen = painter->pen();
                    pen.setColor(cell->mForeground);
                    painter->setPen(pen);

                    int flags = cell->alignment() == 0? (Qt::AlignLeft | Qt::AlignBottom) : cell->alignment();
                    QString value = cell->value().toString();

                    if (cell->mWordWarp)
                        flags |= Qt::TextWordWrap;

                    if (cell->mMergedCell == cell) {
                        quint64 mergeHeight(height), mergeWidth(width);
                        for (quint32 row=cell->row() + 1; row <= cell->mMergePosition.row(); row++)
                            mergeHeight += rowHeight(row);

                        for (quint32 column=cell->column() + 1; column <= cell->mMergePosition.column(); column++)
                            mergeWidth += columnWidth(column);

                        quint64 drawWidth = mergeWidth - (paddingX * 2);
                        quint64 drawHeight = mergeHeight - (paddingY * 2);

                        painter->translate(x, y);
                        painter->fillRect(0, 0, mergeWidth, mergeHeight, cell->mBackground);

                        QTransform transform;
                        transform.rotate(cell->mRotation);
                        QRect resultRect = transform.mapRect(QRect(paddingX, paddingY, drawWidth, drawHeight));

                        painter->rotate(-cell->mRotation);
                        painter->drawText(resultRect.x(), resultRect.y(),
                                          resultRect.width(), resultRect.height(),
                                          flags,
                                          value);
                    }
                    else if (!cell->mMergedCell) {
                        quint64 drawWidth = width - (paddingX * 2);
                        quint64 drawHeight = height - (paddingY * 2);

                        QRect boundingRect = painter->boundingRect(paddingX, paddingY,
                                                                   drawWidth, drawHeight,
                                                                   flags,
                                                                   value);
                        if (!cell->mWordWarp) {
                            int c(0);
                            qint64 defaultWidth = drawWidth;
                            while (boundingRect.width() > (qint32) drawWidth) {
                                if (flags & Qt::AlignLeft)
                                    c++;
                                else if (flags & Qt::AlignRight)
                                    c--;

                                quint32 checkColumn = column + c;
                                SpreadsheetCell *checkCell = mCellHash[row][checkColumn];
                                if (checkCell && !checkCell->value().toString().isEmpty())
                                    break;

                                drawWidth += columnWidth(checkColumn);
                            }

                            if (flags & Qt::AlignRight)
                                painter->translate(defaultWidth - qint64(drawWidth), 0);
                        }

                        painter->translate(x, y);
                        painter->fillRect(0, 0, width, height, cell->mBackground);

                        QTransform transform;
                        transform.rotate(cell->mRotation);
                        QRect resultRect = transform.mapRect(QRect(paddingX, paddingY, drawWidth, drawHeight));

                        painter->rotate(-cell->mRotation);
                        painter->drawText(resultRect.x(), resultRect.y(),
                                          resultRect.width(), resultRect.height(),
                                          flags,
                                          value);
                    }

                    painter->restore();
                }

                x += width;
            }

            y += height;
        }
    }

    painter->restore();

    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    // draw drawing
    foreach (SpreadsheetDrawing *drawing, mDrawingList) {
        if (drawing->image().isNull())
            continue;

        painter->save();

        SpreadsheetCellPosition pos = drawing->position();
        QPoint offset = drawing->offset();
        QSize size = drawing->size();

        quint64 fromX(0), fromY(0);
        for (quint32 column=0; column<pos.column(); column++)
            fromX += columnWidth(column);
        for (quint32 row=0; row<pos.row(); row++)
            fromY += rowHeight(row);

        fromX += offset.x();
        fromY += offset.y();

        painter->translate(fromX, fromY);
        painter->drawImage(QRect(0, 0, size.width(), size.height()), drawing->image());

        painter->restore();
    }

    painter->restore();

    painter->restore();
}

void SpreadsheetWorksheet::render(QPainter *painter, quint32 rowStart, quint32 columStart, quint32 rowEnd, quint32 columnEnd, double scale)
{
    render(painter, SpreadsheetCellPosition(rowStart, columStart), SpreadsheetCellPosition(rowEnd, columnEnd), scale);
}

void SpreadsheetWorksheet::render(QPainter *painter, double scale)
{
    render(painter, SpreadsheetCellPosition(0, 0), SpreadsheetCellPosition(mMaxRow, mMaxColumn), scale);
}

QPen SpreadsheetWorksheet::borderPen(int type)
{
    QPen pen;
    pen.setCosmetic(true);
    pen.setWidth(1);

    switch (type) {
    case Spreadsheet::Medium:
        pen.setWidth(2);
        break;
    case Spreadsheet::Thick:
        pen.setWidth(3);
        break;
    case Spreadsheet::Dot:
        pen.setStyle(Qt::DotLine);
        break;
    case Spreadsheet::Dash:
        pen.setStyle(Qt::DashLine);
        break;
    case Spreadsheet::DashDot:
        pen.setStyle(Qt::DashDotLine);
        break;
    case Spreadsheet::DashDotDot:
        pen.setStyle(Qt::DashDotDotLine);
        break;
    case Spreadsheet::Thin:
    default:
        break;
    }

    return pen;
}
