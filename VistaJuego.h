#ifndef VISTAJUEGO_H
#define VISTAJUEGO_H

#include <QGraphicsView>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>
#include <vector>
#include "Juego.h"

class VistaJuego : public QGraphicsView
{
    Q_OBJECT

public:
    explicit VistaJuego(QWidget *parent = nullptr);
    ~VistaJuego();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void actualizarFrame();

private:
    void construirEscena();   // crea los ítems (se repite al reiniciar)
    void sincronizar();       // copia el estado del modelo a los ítems

    Juego *juego;
    QGraphicsScene *escena;
    QTimer *timer;

    // Ítems que cambian durante la partida
    std::vector<QGraphicsRectItem *> itemsObstaculos[2];
    std::vector<QGraphicsSimpleTextItem *> textosObstaculos[2];
    QGraphicsLineItem *itemsCanon[2] = { nullptr, nullptr };
    QGraphicsEllipseItem *itemProyectil = nullptr;
    QGraphicsPathItem *rastro = nullptr;
    QPainterPath trayectoria;

    // HUD
    QGraphicsSimpleTextItem *textoTurno = nullptr;
    QGraphicsSimpleTextItem *textoVictoria = nullptr;
    QGraphicsSimpleTextItem *textoPotencia = nullptr;
    QGraphicsRectItem *barraFondo = nullptr;
    QGraphicsRectItem *barraPotencia = nullptr;

    // Carga del disparo con el mouse
    bool cargandoDisparo = false;
    double fuerzaDisparo = 100.0;
};

#endif // VISTAJUEGO_H