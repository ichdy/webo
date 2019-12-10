#include "parametersdialog.h"

#include <QJsonDocument>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QDebug>

ParametersDialog::ParametersDialog(const QByteArray &json, QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;

    QFont wFont = font();
    wFont.setPixelSize(14);

    QVariantList parameterList = QJsonDocument::fromJson(json).toVariant().toList();
    foreach (const QVariant &parameter, parameterList) {
        WidgetData *widgetData = new WidgetData;
        widgetData->dataMap = parameter.toMap();

        QString title = widgetData->dataMap["title"].toString();
        QString type = widgetData->dataMap["type"].toString();
        QVariant value = widgetData->dataMap["value"];

        QLabel *titleLabel = new QLabel(title);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);

        QWidget *widget = 0;
        if (type == "text") {
            QLineEdit *tempWidget = new QLineEdit;
            tempWidget->setMinimumWidth(220);
            tempWidget->setAlignment(Qt::AlignCenter);
            tempWidget->setText(value.toString());
            widget = tempWidget;
        }
        else if (type == "password") {
            QLineEdit *tempWidget = new QLineEdit;
            tempWidget->setMinimumWidth(220);
            tempWidget->setEchoMode(QLineEdit::Password);
            tempWidget->setAlignment(Qt::AlignCenter);
            tempWidget->setText(value.toString());
            widget = tempWidget;
        }
        else if (type == "number") {
            QSpinBox *tempWidget = new QSpinBox;
            tempWidget->setMinimumWidth(100);
            tempWidget->setAlignment(Qt::AlignCenter);
            tempWidget->setMaximum(999999);
            tempWidget->setValue(value.toInt());
            widget = tempWidget;
        }

        if (widget) {
            widget->setFont(wFont);
            widgetData->widget = widget;
            layout->addWidget(widget, 0, Qt::AlignCenter);
        }

        mWidgetDataList << widgetData;
    }

    QPushButton *okButton = new QPushButton("OK");
    QPushButton *cancelButton = new QPushButton("Batal");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addSpacing(20);
    layout->addStretch();
    layout->addLayout(buttonLayout);
    setLayout(layout);
    setWindowIcon(QIcon(":/images/parameters.png"));

    resize(400, 100);

    connect(okButton, &QPushButton::clicked, this, &ParametersDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &ParametersDialog::reject);
}

ParametersDialog::~ParametersDialog()
{
    foreach (WidgetData *data, mWidgetDataList)
        delete data;
    mWidgetDataList.clear();
}

QByteArray ParametersDialog::arguments()
{
    QVariantMap args;
    foreach (WidgetData *data, mWidgetDataList) {
        if (data->widget) {
            QString name = data->dataMap["name"].toString();
            QString type = data->dataMap["type"].toString();
            QVariant value;

            if (type == "text" || type == "password")
                value = static_cast<QLineEdit *>(data->widget)->text();
            else if (type == "number")
                value = static_cast<QSpinBox *>(data->widget)->value();

            args[name] = value;
        }
    }

    return QJsonDocument::fromVariant(args).toJson().simplified();
}
