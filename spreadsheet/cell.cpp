#include "cell.h"

#include "worksheet.h"

void SpreadsheetCellBorder::set(Spreadsheet::BorderType type)
{
    mType = type;
}

quint32 SpreadsheetCellPosition::letterToInt(const QString &letter)
{
    quint32 number(0);

    for (int i=0; i < letter.length(); i++)
        number = number * 26 + (letter.at(i).toLatin1() - ('A' - 1));

    return number - 1;
}

QString SpreadsheetCellPosition::intToLetter(quint32 _number)
{
    quint32 number = _number + 1;
    QString result;

    while (number-- > 0) {
        result.prepend((QChar)('A' + char(number % 26)));
        number /= 26;
    }

    return result;
}

SpreadsheetCellPosition SpreadsheetCellPosition::fromCellAddress(const QString &address)
{
    QString row, column;
    for (int i=0; i < address.length(); i++) {
        QChar r = address.at(i);
        if (r.isLetter())
            column.append(r);
        else
            row.append(r);
    }

    return SpreadsheetCellPosition(row.toInt() - 1, letterToInt(column));
}

SpreadsheetCell::SpreadsheetCell(SpreadsheetWorksheet *worksheet, quint32 row, quint32 column, QObject *parent) :
    QObject(parent),
    mWorksheet(worksheet),
    mPosition(row, column),
    mMergedCell(0),
    mMergePosition(row, column),
    mRotation(0),
    mAlignment(Qt::AlignLeft),
    mWordWarp(false),
    mForeground(Qt::black),
    mBackground(Qt::transparent)
{
    mFont.setFamily("Calibri");
    mFont.setPointSize(11);
}

void SpreadsheetCell::setRotation(float value)
{
    mRotation = value;
    emit changed();
}

void SpreadsheetCell::setValue(const QVariant &value)
{
    if (mMergedCell && mMergedCell != this)
        return;

    mValue = value;
    emit changed();
}

void SpreadsheetCell::setFont(const QString &name, int point)
{
    QFont font = mFont;
    font.setFamily(name);
    font.setPointSize(point);

    mFont = font;
    emit changed();
}

void SpreadsheetCell::setBold(bool value)
{
    QFont font = mFont;
    font.setBold(value);

    setFont(font);
}

void SpreadsheetCell::setItalic(bool value)
{
    QFont font = mFont;
    font.setItalic(value);

    setFont(font);
}

void SpreadsheetCell::setUnderline(bool value)
{
    QFont font = mFont;
    font.setUnderline(value);

    setFont(font);
}

void SpreadsheetCell::setFont(const QFont &value)
{
    if (mMergedCell && mMergedCell != this)
        return;

    mFont = value;
    emit changed();
}

void SpreadsheetCell::setAlignment(const QString &hAlign, const QString &vAlign)
{
    int alignment(0);

    if (hAlign == "left")
        alignment |= Qt::AlignLeft;
    else if (hAlign == "right")
        alignment |= Qt::AlignRight;
    else if (hAlign == "center")
        alignment |= Qt::AlignHCenter;

    if (vAlign == "top")
        alignment |= Qt::AlignTop;
    else if (vAlign == "bottom")
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignVCenter;

    setAlignment((Qt::Alignment) alignment);
}

void SpreadsheetCell::setAlignment(Qt::Alignment align)
{
    if (mMergedCell && mMergedCell != this)
        return;

    mAlignment = align;
    emit changed();
}

void SpreadsheetCell::setWordWarp(bool value)
{
    mWordWarp = value;
    emit changed();
}

Spreadsheet::BorderPosition SpreadsheetCell::toBorderPos(const QString &pos)
{
    if (pos == "top")
        return Spreadsheet::Top;
    else if (pos == "bottom")
        return Spreadsheet::Bottom;
    else if (pos == "left")
        return Spreadsheet::Left;
    else if (pos == "right")
        return Spreadsheet::Right;

    return Spreadsheet::Unknown;
}

Spreadsheet::BorderType SpreadsheetCell::toBorderType(const QString &type)
{
    if (type == "thin")
        return Spreadsheet::Thin;
    else if (type == "medium")
        return Spreadsheet::Medium;
    else if (type == "thick")
        return Spreadsheet::Thick;
    else if (type == "dot")
        return Spreadsheet::Dot;
    else if (type == "dash")
        return Spreadsheet::Dash;
    else if (type == "dashDot")
        return Spreadsheet::DashDot;
    else if (type == "dashDotDot")
        return Spreadsheet::DashDotDot;

    return Spreadsheet::None;
}

void SpreadsheetCell::setBorder(const QString &pos, const QString &type)
{
    setBorder(toBorderPos(pos), toBorderType(type));
}

QString SpreadsheetCell::border(const QString &pos)
{
    Spreadsheet::BorderType type = border(toBorderPos(pos));
    if (type == Spreadsheet::Thin)
        return "thin";
    else if (type == Spreadsheet::Medium)
        return "medium";
    else if (type == Spreadsheet::Thick)
        return "thick";
    else if (type == Spreadsheet::Dot)
        return "dot";
    else if (type == Spreadsheet::Dash)
        return "dash";
    else if (type == Spreadsheet::DashDot)
        return "dashDot";
    else if (type == Spreadsheet::DashDotDot)
        return "dashDotDot";

    return QString();
}

Spreadsheet::BorderType SpreadsheetCell::border(Spreadsheet::BorderPosition pos)
{
    if (mMergedCell && mMergedCell != this)
        return mMergedCell->border(pos);

    quint32 endRow = row();
    quint32 endColumn = column();

    if (mMergedCell) {
        endRow = mMergePosition.row();
        endColumn = mMergePosition.column();
    }

    Spreadsheet::BorderType bordered(Spreadsheet::None);

    switch (pos) {
    case Spreadsheet::Top:
    {
        for (quint32 _column=column(); _column <= endColumn; _column++) {
            Spreadsheet::BorderType type = mWorksheet->borderData(row(), _column)->top()->type();
            if (type > 0) {
                bordered = type;
                break;
            }
        }
    }
        break;
    case Spreadsheet::Left:
    {
        for (quint32 _row=row(); _row <= endRow; _row++) {
            Spreadsheet::BorderType type = mWorksheet->borderData(_row, column())->left()->type();
            if (type > 0) {
                bordered = type;
                break;
            }
        }
    }
        break;
    case Spreadsheet::Right:
        for (quint32 _row=row(); _row <= endRow; _row++) {
            Spreadsheet::BorderType type = mWorksheet->borderData(_row, endColumn + 1)->left()->type();
            if (type > 0) {
                bordered = type;
                break;
            }
        }
        break;
    case Spreadsheet::Bottom:
        for (quint32 _column=column(); _column <= endColumn; _column++) {
            Spreadsheet::BorderType type = mWorksheet->borderData(endRow + 1, _column)->top()->type();
            if (type > 0) {
                bordered = type;
                break;
            }
        }
        break;
    default:
        break;
    }

    return bordered;
}

void SpreadsheetCell::setBorder(Spreadsheet::BorderPosition pos, Spreadsheet::BorderType type)
{
    if (mMergedCell && mMergedCell != this)
        return;

    quint32 endRow = row();
    quint32 endColumn = column();

    if (mMergedCell) {
        endRow = mMergePosition.row();
        endColumn = mMergePosition.column();
    }

    switch (pos) {
    case Spreadsheet::Top:
        for (quint32 _column=column(); _column <= endColumn; _column++)
            mWorksheet->borderData(row(), _column)->top()->set(type);
        break;
    case Spreadsheet::Left:
        for (quint32 _row=row(); _row <= endRow; _row++)
            mWorksheet->borderData(_row, column())->left()->set(type);
        break;
    case Spreadsheet::Right:
        for (quint32 _row=row(); _row <= endRow; _row++)
            mWorksheet->borderData(_row, endColumn + 1)->left()->set(type);
        break;
    case Spreadsheet::Bottom:
        for (quint32 _column=column(); _column <= endColumn; _column++)
            mWorksheet->borderData(endRow + 1, _column)->top()->set(type);
        break;
    default:
        break;
    }

    emit changed();
}

void SpreadsheetCell::setForegroundColor(const QColor &value)
{
    mForeground = value;
    emit changed();
}

void SpreadsheetCell::setBackgroundColor(const QColor &value)
{
    mBackground = value;
    emit changed();
}

