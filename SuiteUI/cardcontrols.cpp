#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

/**
 * @file CardControls.h
 * @brief Widget pequeño con controles (favorito, info, eliminar) para mostrar en cada tarjeta.
 *
 * CardControls es un contenedor horizontal con tres QPushButton estilizados en círculo:
 * - btnFav : botón de favorito (★)
 * - btnInfo : botón de información (i)
 * - btnDel : botón de eliminar (X)
 *
 * Este widget está pensado para usarse dentro de un delegado o como parte de la UI
 * donde se muestran tarjetas de imagen. No gestiona la lógica de negocio; solo
 * expone los botones para conectar sus señales (clicked) desde quien lo instancie.
 *
 * Comentarios en estilo Doxygen en español para facilitar la generación de documentación.
 */
class CardControls : public QWidget {
    Q_OBJECT
public:
    /// Botón para marcar/desmarcar favorito (ícono: ★)
    QPushButton *btnFav;

    /// Botón de información (ícono: i)
    QPushButton *btnInfo;

    /// Botón de eliminar (ícono: X)
    QPushButton *btnDel;

    /**
     * @brief Constructor.
     *
     * Crea los tres botones, aplica un estilo visual consistente y los coloca en
     * un QHBoxLayout sin márgenes para que ocupen el mínimo espacio requerido.
     *
     * @param parent Widget padre (por defecto nullptr).
     */
    explicit CardControls(QWidget *parent = nullptr) : QWidget(parent) {
        // Layout horizontal compacto
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(5);

        // Crear botones con texto/icónicos simples
        btnFav = new QPushButton("★", this);
        btnInfo = new QPushButton("i", this);
        btnDel = new QPushButton("X", this);

        // Estilo compartido para obtener botones circulares y un look uniforme.
        // - border-radius: redondea para dar aspecto circular (asumiendo size 30x30)
        // - border: línea gris clara
        // - background: color de fondo por defecto
        // - min/max width/height para forzar tamaño fijo
        QString style = "QPushButton { "
                        "border-radius: 15px; "
                        "border: 1px solid #ccc; "
                        "background: #f0f0f0; "
                        "min-width: 30px; max-width: 30px; "
                        "min-height: 30px; } "
                        "QPushButton:hover { background: #e0e0e0; }";

        // Aplicar estilos
        btnFav->setStyleSheet(style);
        btnInfo->setStyleSheet(style);
        // Botón eliminar con color de texto en rojo para destacar acción destructiva
        btnDel->setStyleSheet(style + " QPushButton { color: red; }");

        // Añadir al layout
        layout->addWidget(btnFav);
        layout->addWidget(btnInfo);
        layout->addWidget(btnDel);

        // Hacer el fondo transparente para que el widget se integre con la tarjeta.
        setAttribute(Qt::WA_TranslucentBackground);
    }
};
