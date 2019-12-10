#ifndef SPREADSHEETCELL_H
#define SPREADSHEETCELL_H

#include <QObject>
#include <QVariant>
#include <QFont>
#include <QColor>

namespace Spreadsheet {
    enum BorderPosition {
        Unknown = -1,
        Top = 0,
        Bottom,
        Left,
        Right,
    };

    enum BorderType {
        None = 0,
        Thin,
        Medium,
        Thick,
        Dot,
        Dash,
        DashDot,
        DashDotDot
    };
}

class SpreadsheetWorksheet;
class SpreadsheetCellBorder {
public:
    SpreadsheetCellBorder() : mType(Spreadsheet::None) {}

    void set(Spreadsheet::BorderType type);
    Spreadsheet::BorderType type() const { return mType; }

private:
    Spreadsheet::BorderType mType;
};

class SpreadsheetCellBorderData {
public:
    SpreadsheetCellBorderData() {}

    SpreadsheetCellBorder *top() { return &mTop; }
    SpreadsheetCellBorder *left() { return &mLeft; }

private:
    SpreadsheetCellBorder mTop;
    SpreadsheetCellBorder mLeft;
};

class SpreadsheetCellPosition {
public:
    SpreadsheetCellPosition(quint32 row=0, quint32 column=0) : mRow(row), mColumn(column) {}
    SpreadsheetCellPosition(const SpreadsheetCellPosition &other) : mRow(other.mRow), mColumn(other.mColumn) {}
    SpreadsheetCellPosition operator=(const SpreadsheetCellPosition &other) { mRow = other.mRow; mColumn = other.mColumn; return *this; }

    Q_INVOKABLE quint32 row() const { return mRow; }
    Q_INVOKABLE quint32 column() const { return mColumn; }

    void clear() { mRow = 0; mColumn = 0; }

    static QString intToLetter(quint32 number);
    static quint32 letterToInt(const QString &letter);
    static SpreadsheetCellPosition fromCellAddress(const QString &address);

private:
    quint32 mRow;
    quint32 mColumn;
};

class SpreadsheetCell : public QObject
{
    Q_OBJECT

    friend class SpreadsheetWorksheet;

public:
    explicit SpreadsheetCell(SpreadsheetWorksheet *worksheet, quint32 row, quint32 column, QObject *parent = 0);

    Q_INVOKABLE const SpreadsheetCellPosition &position() { return mPosition; }
    Q_INVOKABLE quint32 row() { return mPosition.row(); }
    Q_INVOKABLE quint32 column() { return mPosition.column(); }

    Q_INVOKABLE void setRotation(float value);
    Q_INVOKABLE float rotation() { return mRotation; }

    Q_INVOKABLE void setValue(const QVariant &value);
    Q_INVOKABLE QVariant value() { return mValue; }

    Q_INVOKABLE void setFont(const QString &name, int point);
    Q_INVOKABLE void setBold(bool value);
    Q_INVOKABLE void setItalic(bool value);
    Q_INVOKABLE void setUnderline(bool value);
    void setFont(const QFont &value);
    QFont font() { return mFont; }

    Q_INVOKABLE void setAlignment(const QString &hAlign, const QString &vAlign = QString());
    void setAlignment(Qt::Alignment align);
    Q_INVOKABLE Qt::Alignment alignment() { return mAlignment; }

    Q_INVOKABLE void setWordWarp(bool value);
    Q_INVOKABLE bool wordWarp() { return mWordWarp; }

    Q_INVOKABLE void setBorder(const QString &pos, const QString &type);
    Q_INVOKABLE QString border(const QString &pos);
    void setBorder(Spreadsheet::BorderPosition pos, Spreadsheet::BorderType type);
    Spreadsheet::BorderType border(Spreadsheet::BorderPosition pos);

    Q_INVOKABLE void setForegroundColor(const QColor &value);
    Q_INVOKABLE void setBackgroundColor(const QColor &value);
    Q_INVOKABLE QColor foregroundColor() { return mForeground; }
    Q_INVOKABLE QColor backgroundColor() { return mBackground; }

signals:
    void changed();

private:
    Spreadsheet::BorderPosition toBorderPos(const QString &pos);
    Spreadsheet::BorderType toBorderType(const QString &type);

private:
    SpreadsheetWorksheet *mWorksheet;

    SpreadsheetCellPosition mPosition;
    SpreadsheetCell *mMergedCell;
    SpreadsheetCellPosition mMergePosition;
    float mRotation;
    QVariant mValue;
    QFont mFont;
    Qt::Alignment mAlignment;
    bool mWordWarp;
    QColor mForeground;
    QColor mBackground;
};

#endif // SPREADSHEETCELL_H
