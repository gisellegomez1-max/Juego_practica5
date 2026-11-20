#include "VistaJuego.h"
#include <QBrush>
#include <QPen>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace {
const double ANCHO = 800.0;
const double ALTO = 600.0;
const double PISO = 520.0;           // debe coincidir con PISO de Juego.cpp
const int INTERVALO_MS = 16;
const double FUERZA_MIN = 100.0;
const double FUERZA_MAX = 1200.0;
const double FUERZA_PASO = 15.0;

const QColor COLOR_BLOQUE[2]  = { QColor(200, 160, 40), QColor(180, 100, 60) };
const QColor COLOR_JUGADOR[2] = { QColor(Qt::blue), QColor(Qt::darkMagenta) };
}

VistaJuego::VistaJuego(QWidget *parent)
    : QGraphicsView(parent)
{
    juego = new Juego();
    escena = new QGraphicsScene(0, 0, ANCHO, ALTO, this);
    setScene(escena);

    setFixedSize(int(ANCHO), int(ALTO));
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    setBackgroundBrush(QColor(135, 206, 235));      // cielo
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    escena = new QGraphicsScene(0, 0, ANCHO, ALTO, this);
    escena->setItemIndexMethod(QGraphicsScene::NoIndex);
    setScene(escena);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    construirEscena();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &VistaJuego::actualizarFrame);
    timer->start(INTERVALO_MS);
}

VistaJuego::~VistaJuego()
{
    delete juego;     // la escena y sus ítems los libera Qt (son hijos de la vista)
}

void VistaJuego::construirEscena()
{
    escena->clear();                 // borra todos los ítems anteriores
    itemProyectil = nullptr;
    for (int i = 0; i < 2; ++i) {
        itemsObstaculos[i].clear();
        textosObstaculos[i].clear();
    }

    // Piso
    escena->addRect(0, PISO, ANCHO, ALTO - PISO, QPen(Qt::NoPen), QBrush(Qt::darkGray));

    QFont fuenteBloque;
    fuenteBloque.setPointSize(9);
    fuenteBloque.setBold(true);
    rastro = escena->addPath(QPainterPath(), QPen(QColor(0, 0, 0, 140), 1.5, Qt::DotLine));
    rastro->setZValue(5);

    for (int i = 0; i < 2; ++i) {
        // --- Jugador (base) con su nombre ---
        const Jugador *j = juego->getJugador(i);
        const QRectF rj = j->obtenerLimites();
        escena->addRect(rj, QPen(Qt::black, 2), QBrush(COLOR_JUGADOR[i]));
        QGraphicsSimpleTextItem *nombre = escena->addSimpleText(j->getNombre());
        nombre->setPos(rj.center().x() - nombre->boundingRect().width() / 2.0, rj.top() - 22);

        // --- Rival: zona de impacto + muñequito ---
        const QRectF rr = juego->getRival(i)->obtenerLimites();
        const double cx = rr.center().x();
        escena->addRect(rr, QPen(Qt::NoPen), QBrush(QColor(255, 255, 255, 120)));
        const QPen lapiz(Qt::black, 2);
        escena->addEllipse(cx - 8, rr.top() + 6, 16, 16, lapiz, QBrush(Qt::white));      // cabeza
        escena->addLine(cx, rr.top() + 22, cx, rr.top() + 50, lapiz);                    // cuerpo
        escena->addLine(cx - 12, rr.top() + 30, cx + 12, rr.top() + 30, lapiz);          // brazos
        escena->addLine(cx, rr.top() + 50, cx - 10, rr.top() + 76, lapiz);               // pierna
        escena->addLine(cx, rr.top() + 50, cx + 10, rr.top() + 76, lapiz);               // pierna

        // --- Bloques de la infraestructura ---
        for (const Obstaculo *o : juego->getObstaculos(i)) {
            QGraphicsRectItem *r = escena->addRect(o->obtenerLimites(), QPen(Qt::black, 1.5),
                                                   QBrush(COLOR_BLOQUE[i]));
            QGraphicsSimpleTextItem *t = escena->addSimpleText(
                QString::number(qRound(o->getResistencia())), fuenteBloque);
            itemsObstaculos[i].push_back(r);
            textosObstaculos[i].push_back(t);
        }

        // --- Cañón: tubo que gira + base ---
        const Canon *c = juego->getCanon(i);
        itemsCanon[i] = escena->addLine(0, 0, c->getLongitud(), 0,
                                        QPen(Qt::darkGray, 10.0, Qt::SolidLine, Qt::RoundCap));
        itemsCanon[i]->setPos(c->getX(), c->getY());
        escena->addEllipse(c->getX() - 15, c->getY() - 15, 30, 30,
                           QPen(Qt::black, 2), QBrush(Qt::darkGreen));
    }

    // --- HUD ---
    QFont fuenteGrande;
    fuenteGrande.setPointSize(24);
    fuenteGrande.setBold(true);

    QFont fuenteTurno;
    fuenteTurno.setPointSize(14);
    fuenteTurno.setBold(true);

    textoTurno = escena->addSimpleText("", fuenteTurno);
    textoTurno->setZValue(100);

    textoVictoria = escena->addSimpleText("", fuenteGrande);
    textoVictoria->setBrush(QBrush(Qt::black));
    textoVictoria->setZValue(100);
    textoVictoria->setVisible(false);

    textoPotencia = escena->addSimpleText("Potencia de disparo:");
    textoPotencia->setPos(20, 5);
    textoPotencia->setZValue(100);

    barraFondo = escena->addRect(20, 25, FUERZA_MAX / 5.0, 15, QPen(Qt::black), QBrush(Qt::NoBrush));
    barraPotencia = escena->addRect(20, 25, 0, 15, QPen(Qt::black), QBrush(Qt::red));
    barraFondo->setZValue(100);
    barraPotencia->setZValue(100);

    QGraphicsSimpleTextItem *ayuda = escena->addSimpleText(
        "Mueve el mouse para apuntar  |  Mantén clic para cargar y suelta para disparar  |  R reinicia");
    ayuda->setBrush(QBrush(Qt::white));
    ayuda->setPos(15, ALTO - 28);
    ayuda->setZValue(100);
}

void VistaJuego::sincronizar()
{
    for (int i = 0; i < 2; ++i) {
        // Bloques: se ocultan al destruirse y se desvanecen al perder resistencia
        const std::vector<Obstaculo *> &obs = juego->getObstaculos(i);
        for (size_t k = 0; k < obs.size() && k < itemsObstaculos[i].size(); ++k) {
            const Obstaculo *o = obs[k];
            QGraphicsRectItem *r = itemsObstaculos[i][k];
            QGraphicsSimpleTextItem *t = textosObstaculos[i][k];

            const bool vivo = !o->estaDestruido();
            r->setVisible(vivo);
            t->setVisible(vivo);
            if (!vivo)
                continue;

            const double ratio = o->getResistencia() / o->getResistenciaInicial();
            r->setOpacity(0.35 + 0.65 * ratio);
            t->setText(QString::number(qRound(o->getResistencia())));
            const QRectF lim = o->obtenerLimites();
            t->setPos(lim.center().x() - t->boundingRect().width() / 2.0,
                      lim.center().y() - t->boundingRect().height() / 2.0);
        }

        // Cañón: solo cambia el ángulo
        itemsCanon[i]->setRotation(-juego->getCanon(i)->getAngulo());
    }

    // Proyectil: aparece al disparar y se elimina al terminar el turno
    const Bola *p = juego->getProyectil();
    if (p) {
        if (!itemProyectil) {
            const double r = p->getRadio();
            itemProyectil = escena->addEllipse(-r, -r, 2 * r, 2 * r, QPen(Qt::black, 1.5),
                                               QBrush(juego->getTurno() == 0 ? Qt::blue : Qt::magenta));
            itemProyectil->setZValue(10);
            trayectoria = QPainterPath(QPointF(p->getX(), p->getY()));   // nuevo disparo: rastro nuevo
        }
        itemProyectil->setPos(p->getX(), p->getY());
        trayectoria.lineTo(p->getX(), p->getY());
        rastro->setPath(trayectoria);
    } else if (itemProyectil) {
        escena->removeItem(itemProyectil);
        delete itemProyectil;
        itemProyectil = nullptr;     // el rastro se queda hasta el próximo disparo
    }

    // Turno / victoria
    const bool fin = (juego->getEstado() == Juego::TERMINADO);

    const QString textoT = QString("Turno: Jugador %1").arg(juego->getTurno() + 1);
    if (textoTurno->text() != textoT) {
        textoTurno->setText(textoT);
        textoTurno->setBrush(QBrush(COLOR_JUGADOR[juego->getTurno()]));
        textoTurno->setPos((ANCHO - textoTurno->boundingRect().width()) / 2.0, 8);
    }
    textoTurno->setVisible(!fin);

    textoVictoria->setVisible(fin);
    if (fin) {
        const QString textoV = QString("¡VICTORIA DEL JUGADOR %1!\nPresiona R para reiniciar")
                                   .arg(juego->getGanador() + 1);
        if (textoVictoria->text() != textoV) {
            textoVictoria->setText(textoV);
            textoVictoria->setPos((ANCHO - textoVictoria->boundingRect().width()) / 2.0, 180);
        }
    }

    // Barra de potencia: solo mientras se carga el disparo
    const bool cargando = cargandoDisparo && juego->getEstado() == Juego::APUNTANDO;
    barraFondo->setVisible(cargando);
    barraPotencia->setVisible(cargando);
    textoPotencia->setVisible(cargando);
    if (cargando)
        barraPotencia->setRect(20, 25, fuerzaDisparo / 5.0, 15);
}

void VistaJuego::actualizarFrame()
{
    if (cargandoDisparo && juego->getEstado() == Juego::APUNTANDO)
        fuerzaDisparo = std::min(fuerzaDisparo + FUERZA_PASO, FUERZA_MAX);

    juego->actualizarMundo(INTERVALO_MS / 1000.0);   // la física sigue en el modelo
    sincronizar();                                   // la vista solo refleja el estado
}

void VistaJuego::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_R) {
        cargandoDisparo = false;
        juego->inicializarJuego();
        construirEscena();           // el modelo creó objetos nuevos: se rehace la escena
    } else {
        QGraphicsView::keyPressEvent(event);
    }
}

void VistaJuego::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF p = mapToScene(event->pos());
    const double dx = p.x() - juego->getXCanonActual();
    const double dy = p.y() - juego->getYCanonActual();
    juego->setAnguloCanon(-std::atan2(dy, dx) * (180.0 / M_PI));
}

void VistaJuego::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && juego->getEstado() == Juego::APUNTANDO) {
        cargandoDisparo = true;
        fuerzaDisparo = FUERZA_MIN;
    }
}

void VistaJuego::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && cargandoDisparo) {
        cargandoDisparo = false;
        juego->disparar(fuerzaDisparo);
    }
}