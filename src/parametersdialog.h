#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>
#include <QVariant>

class ParametersDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParametersDialog(const QByteArray &json, QWidget *parent = nullptr);
    ~ParametersDialog();

    struct WidgetData {
        QWidget *widget;
        QVariantMap dataMap;
    };

    QByteArray arguments();

private:
    QList<WidgetData *> mWidgetDataList;
};

#endif // PARAMETERSDIALOG_H
