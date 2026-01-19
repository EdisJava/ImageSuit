#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

class CardControls : public QWidget {
    Q_OBJECT
public:
    QPushButton *btnFav, *btnInfo, *btnDel;

    explicit CardControls(QWidget *parent = nullptr) : QWidget(parent) {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(5);

        btnFav = new QPushButton("★", this);
        btnInfo = new QPushButton("i", this);
        btnDel = new QPushButton("X", this);

        // Estilo para que parezcan circulares y limpios
        QString style = "QPushButton { border-radius: 15px; border: 1px solid #ccc; background: #f0f0f0; min-width: 30px; max-width: 30px; min-height: 30px; } "
                        "QPushButton:hover { background: #e0e0e0; }";

        btnFav->setStyleSheet(style);
        btnInfo->setStyleSheet(style);
        btnDel->setStyleSheet(style + "QPushButton { color: red; }");

        layout->addWidget(btnFav);
        layout->addWidget(btnInfo);
        layout->addWidget(btnDel);

        setAttribute(Qt::WA_TranslucentBackground);
    }
};
