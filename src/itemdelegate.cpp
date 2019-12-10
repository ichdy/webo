#include "itemdelegate.h"

#include <QDateTime>
#include <QPainter>
#include <QDebug>

ItemDelegate::ItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent) {}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    int type = index.data(Qt::UserRole + 1).toInt();

    return QSize(0, type? 44 : 20);
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font = painter->font();
    int y = option.rect.y();
    int width = painter->device()->width();
    int height = option.rect.height();

    painter->save();

    QString name = index.data(Qt::DisplayRole).toString();
    int type = index.data(Qt::UserRole +1).toInt();

    if (type == 0) {
        painter->save();

        font.setBold(true);
        painter->setFont(font);
        painter->drawText(20, y, width - 52, height, Qt::AlignVCenter | Qt::AlignLeft, name);

        painter->restore();
    }
    else {
        QVariantMap dataMap = index.data(Qt::UserRole +2).toMap();
        QString version = dataMap["version"].toString();

        painter->save();
        QColor selectedColor;
        if (option.state & QStyle::State_Selected)
            selectedColor.setNamedColor("#efefef");
        else
            selectedColor.setNamedColor("#fff");

        painter->setPen(selectedColor);
        painter->setBrush(selectedColor);
        painter->drawRect(0, y, width, height);
        painter->restore();

        painter->save();
        if (!(option.state & QStyle::State_Selected))
            painter->setPen(Qt::lightGray);
        painter->drawRect(2, y + 2, width - 5, height - 5);
        painter->restore();

        QString statusImage = ":/images/script.png";
        painter->drawImage(7, y + 6, QImage(statusImage));

        font.setPixelSize(14);
        painter->setFont(font);
        painter->drawText(48, y + 6, width - 54, 18, Qt::AlignVCenter | Qt::AlignLeft, name);

        font.setPixelSize(12);
        painter->setPen(Qt::darkGray);
        painter->setFont(font);
        painter->drawText(48, y + 24, width - 54, 14, Qt::AlignVCenter | Qt::AlignRight, QString("v%1").arg(version));
    }

    painter->restore();
}

QString ItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(value)
    Q_UNUSED(locale)

    return QString();
}

QString ItemDelegate::formatNpwp(const QString &npwp) const
{
    return QString("%1.%2.%3.%4-%5.%6").arg(npwp.mid(0, 2))
            .arg(npwp.mid(2, 3))
            .arg(npwp.mid(5, 3))
            .arg(npwp.mid(8, 1))
            .arg(npwp.mid(9, 3))
            .arg(npwp.mid(12, 3));
}
