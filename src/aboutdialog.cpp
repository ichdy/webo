#include "aboutdialog.h"

#include <QLabel>
#include <QGridLayout>
#include <QTabWidget>
#include <QTextEdit>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint)
{
    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap(":/images/logo.png"));

    QLabel *appNameLabel = new QLabel(QString("\n%1 v%2").arg("Webo").arg("0.2.2"));
    QLabel *appDetailLabel = new QLabel("Web Automation\n");

    QFont titleFont;
    titleFont.setPixelSize(18);
    titleFont.setWeight(75);
    appNameLabel->setFont(titleFont);

    QFont detailFont;
    detailFont.setPixelSize(12);
    detailFont.setItalic(true);
    appDetailLabel->setFont(detailFont);

    QLabel *buildLabel = new QLabel(QString("Built on %1 at %2\n").arg(__DATE__, __TIME__));
    QLabel *authorLabel = new QLabel(QString("Dibuat Oleh:\n%1\n%2\n").arg("Ichdyan Thalasa", "ichdyan.thalasa@gmail.com"));
    authorLabel->setAlignment(Qt::AlignCenter);

    QGridLayout *aboutLayout = new QGridLayout;
    aboutLayout->setSpacing(0);
    aboutLayout->addWidget(logoLabel, 0, 0, Qt::AlignCenter);
    aboutLayout->addWidget(appNameLabel, 1, 0, Qt::AlignCenter);
    aboutLayout->addWidget(appDetailLabel, 2, 0, Qt::AlignCenter);
    aboutLayout->addWidget(buildLabel, 3, 0, Qt::AlignCenter);
    aboutLayout->addWidget(authorLabel, 4, 0, Qt::AlignCenter);

    QWidget *aboutWidget = new QWidget;
    aboutWidget->setLayout(aboutLayout);

    QTabWidget *tab = new QTabWidget;
    tab->addTab(aboutWidget, "Aplikasi");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(tab);

    setLayout(layout);
    setWindowTitle(QString("About"));
    setWindowIcon(QIcon(":/images/about.png"));
    resize(300, 250);
}
