#include "VentanaJuego.h"
#include <QPainter>
#include <cmath>

VentanaJuego::VentanaJuego(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFixedSize(800, 600); // Tamaño fijo de la ventana

    juego = new Juego();

    // QTimer se encarga del bucle principal del juego (Game Loop)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VentanaJuego::actualizarFrame);
    timer->start(16); // Aproximadamente 60 FPS (1000ms / 60 = 16.6ms)
}

VentanaJuego::~VentanaJuego()
{
    delete juego;
}

void VentanaJuego::actualizarFrame()
{
    // Si estamos presionando el click, la fuerza aumenta
    if (cargandoDisparo && juego->getEstado() == Juego::APUNTANDO) {
        fuerzaDisparo += 15.0;
        if (fuerzaDisparo > 1200.0) fuerzaDisparo = 1200.0;
    }

    juego->actualizarMundo(0.016);
    update();
}

void VentanaJuego::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Pintar fondo del cielo
    painter.fillRect(rect(), QColor(135, 206, 235));

    // Delegar todo el dibujo al controlador del juego
    juego->dibujarMundo(&painter);

    // --- DIBUJAR LA BARRA DE ENERGÍA ---
    if (cargandoDisparo) {
        painter.setBrush(Qt::red);
        painter.setPen(Qt::black);
        // Dibuja un rectángulo que crece según la fuerza (dividido por 5 para que quepa en pantalla)
        painter.drawRect(20, 25, fuerzaDisparo / 5, 15);

        // Texto de potencia
        painter.setPen(Qt::black);
        painter.drawText(20, 20, "Potencia de disparo:");
    }
}

void VentanaJuego::keyPressEvent(QKeyEvent *event)
{       //controles basicos del teclado
    if (event->key() == Qt::Key_R) {
        cargandoDisparo = false;
        juego->inicializarJuego();
    }
}

void VentanaJuego::mouseMoveEvent(QMouseEvent *event)
{
    const double dx = event->position().x() - juego->getXCanonActual();
    const double dy = event->position().y() - juego->getYCanonActual();
    juego->setAnguloCanon(-std::atan2(dy, dx) * (180.0 / M_PI));
    update();
}

void VentanaJuego::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && juego->getEstado() == Juego::APUNTANDO) {
        cargandoDisparo = true;
        fuerzaDisparo = 100.0;
    }
}

void VentanaJuego::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && cargandoDisparo) {
        cargandoDisparo = false;
        juego->disparar(fuerzaDisparo);
    }
}