#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

class ConfirmDialog : public QDialog {

public:
    ConfirmDialog(const QString &title, const QString &details, QWidget *parent = nullptr) 
        : QDialog(parent) 
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setAttribute(Qt::WA_TranslucentBackground);

        QWidget *container = new QWidget(this);
        container->setObjectName("dialogContainer");
        container->setStyleSheet(
            "QWidget#dialogContainer { "
            "  background-color: #f9fafb; "
            "  border-radius: 15px; "
            "}"
        );
        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
        shadow->setBlurRadius(30);
        shadow->setXOffset(0);
        shadow->setYOffset(0);
        shadow->setColor(QColor(0, 0, 0, 80));
        container->setGraphicsEffect(shadow);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(container);

        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(20, 20, 20, 20);

        QLabel *lblTitle = new QLabel(title);
        lblTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #001833;");
        layout->addWidget(lblTitle);

        QLabel *lblDetails = new QLabel(details);
        lblDetails->setStyleSheet("font-size: 14px; color: #4b5563; margin-top: 10px;");
        lblDetails->setWordWrap(true);
        layout->addWidget(lblDetails);

        layout->addSpacing(20);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnCancel = new QPushButton("Cancel");
        QPushButton *btnOk = new QPushButton("Confirm");

        QString btnStyle = "QPushButton { padding: 10px; border-radius: 8px; font-size: 16px; font-weight: bold; min-width: 100px; }";
        btnCancel->setStyleSheet(btnStyle + "QPushButton { background-color: #e5e7eb; color: #374151; } QPushButton:hover { background-color: #d1d5db; }");
        btnOk->setStyleSheet(btnStyle + "QPushButton { background-color: #3b82f6; color: white; } QPushButton:hover { background-color: #2563eb; }");

        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnOk);
        layout->addLayout(btnLayout);

        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    }
};

#endif