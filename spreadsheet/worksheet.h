#ifndef SPREADSHEETWORKSHEET_H
#define SPREADSHEETWORKSHEET_H

#include <QObject>
#include <QHash>
#include <QImage>

#include "cell.h"

class QPainter;
class SpreadsheetDrawing {
public:
    SpreadsheetDrawing() {}

    void setPosition(const SpreadsheetCellPosition &pos);
    void setOffset(const QPoint &offset);
    void setSize(const QSize &size);
    void setImage(const QImage &image);

    SpreadsheetCellPosition position() const { return mPosition; }
    QPoint offset() const { return mOffset; }
    QSize size() const { return mSize; }
    QImage image() const { return mImage; }

private:
    SpreadsheetCellPosition mPosition;
    QPoint mOffset;
    QSize mSize;
    QImage mImage;
};

class SpreadsheetWorksheet : public QObject
{
    Q_OBJECT

    friend class SpreadsheetCell;

public:
    explicit SpreadsheetWorksheet(QObject *parent = 0);
    ~SpreadsheetWorksheet();

    Q_INVOKABLE void setName(const QString &name);
    Q_INVOKABLE QString name();

    Q_INVOKABLE void setDefaultFont(const QString &name, int point);

    Q_INVOKABLE SpreadsheetCell *cell(quint32 row, quint32 column);
    Q_INVOKABLE SpreadsheetCell *cell(const SpreadsheetCellPosition &pos);
    Q_INVOKABLE SpreadsheetCell *cell(const QString &address);
    SpreadsheetCell *cell(const QPoint &pos);

    bool mergeCell(const SpreadsheetCellPosition &start, const SpreadsheetCellPosition &end);
    Q_INVOKABLE bool mergeCell(const QString &start, const QString &end);
    Q_INVOKABLE bool mergeCell(quint32 startRow, quint32 startColumn, quint32 endRow, quint32 endColumn);

    Q_INVOKABLE quint32 maxRow() { return mMaxRow; }
    Q_INVOKABLE quint32 maxColumn() { return mMaxColumn; }

    Q_INVOKABLE void setColumnWidth(quint32 column, qint64 width);
    Q_INVOKABLE void setRowHeight(quint32 row, qint64 height);

    Q_INVOKABLE void fitContent(quint32 row, quint32 column);
    Q_INVOKABLE void fitContent(const QString &address);

    Q_INVOKABLE quint64 columnWidth(quint32 column);
    Q_INVOKABLE quint64 rowHeight(quint32 row);

    Q_INVOKABLE quint64 width();
    Q_INVOKABLE quint64 height();

    Q_INVOKABLE void insertRow(quint32 row);

    Q_INVOKABLE void findAndReplace(const QString &from, const QString &to);
    Q_INVOKABLE void findAndReplace(const QString &from, const QString &to, quint32 row, quint32 column);
    void findAndReplace(const QString &from, const QString &to, const SpreadsheetCellPosition &pos);
    Q_INVOKABLE void findAndReplace(const QString &from, const QString &to, const QString &address);

    void addDrawing(SpreadsheetDrawing *drawing);

    void render(QPainter *painter, double scale = 1.0);
    void render(QPainter *painter, quint32 rowStart, quint32 columStart, quint32 rowEnd, quint32 columnEnd, double scale = 1.0);
    void render(QPainter *painter, const SpreadsheetCellPosition &start, const SpreadsheetCellPosition &end, double scale = 1.0);

signals:
    void cellChanged(SpreadsheetCell *cell);
    void rowHeightChanged(quint32 row);
    void columnWidthChanged(quint32 column);

private slots:
    void onCellChanged();

private:
    SpreadsheetCellBorderData *borderData(quint32 row, quint32 column);
    QPen borderPen(int type);

private:
    QString mName;
    QHash<quint32, QHash<quint32, SpreadsheetCellBorderData *> > mCellBorderDataHash;
    QHash<quint32, QHash<quint32, SpreadsheetCell *> > mCellHash;
    QList<SpreadsheetDrawing *> mDrawingList;
    QHash<quint32, qint64> mWidthHash;
    QHash<quint32, qint64> mHeightHash;
    quint32 mMaxRow;
    quint32 mMaxColumn;
    QFont mDefaultFont;
};

#endif // SPREADSHEETWORKSHEET_H
