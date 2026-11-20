#include "Juego.h"
#include <cmath>

namespace {
const double ANCHO = 800.0;
const double PISO = 520.0;              // límite inferior de la caja
const int SUBPASOS = 4;                 // evita que el proyectil atraviese bloques
const double TIEMPO_MAX_VUELO = 8.0;    // s
const int MAX_REBOTES = 4;              // rebotes en los límites de la caja
}

Juego::Juego()
{
    motor.setGravedad(800.0);
    resolutor = ResolutorColisiones(0.6, 0.1);   // restitución y factor de daño
    inicializarJuego();
}

Juego::~Juego()
{
    limpiar();
}

void Juego::limpiar()
{
    delete proyectil;
    proyectil = nullptr;

    for (int i = 0; i < 2; ++i) {
        delete jugadores[i];  jugadores[i] = nullptr;
        delete canones[i];    canones[i] = nullptr;
        delete rivales[i];    rivales[i] = nullptr;
        for (Obstaculo *o : obstaculos[i]) delete o;
        obstaculos[i].clear();
    }
    for (Pared *p : paredes) delete p;
    paredes.clear();
}

void Juego::inicializarJuego()
{
    limpiar();

    estado = APUNTANDO;
    turno = 0;
    ganador = -1;
    tiempoVuelo = 0.0;
    rebotes = 0;

    paredes.push_back(new Pared(0, PISO, ANCHO, 80, Qt::darkGray));

    crearBando(0);
    crearBando(1);
}

// El bando derecho es el reflejo del izquierdo respecto al centro de la caja
void Juego::crearBando(int i)
{
    auto X = [i](double x, double ancho) { return i == 0 ? x : ANCHO - x - ancho; };
    const QColor colorBloque = (i == 0) ? QColor(200, 160, 40) : QColor(180, 100, 60);

    jugadores[i] = new Jugador(i == 0 ? 40 : ANCHO - 40, 500, 40,
                               i == 0 ? "Jugador 1" : "Jugador 2",
                               i == 0 ? QColor(Qt::blue) : QColor(Qt::darkMagenta));

    canones[i] = new Canon(i == 0 ? 60 : ANCHO - 60, 470, 50, Qt::darkGreen);
    canones[i]->setAngulo(i == 0 ? 45.0 : 135.0);

    // El rival queda protegido entre el techo y las dos columnas
    rivales[i] = new Rival(X(140, 40), 440, 40, 80, Qt::red);

    obstaculos[i].push_back(new Obstaculo(X(110, 100), 410, 100, 30, 100.0, colorBloque)); // techo
    obstaculos[i].push_back(new Obstaculo(X(110, 30), 440, 30, 80, 200.0, colorBloque));   // columna
    obstaculos[i].push_back(new Obstaculo(X(180, 30), 440, 30, 80, 200.0, colorBloque));   // columna
}

void Juego::setAnguloCanon(double grados)
{
    if (estado != APUNTANDO) return;

    // Solo se apunta hacia arriba (0° a 180°)
    if (grados < 0.0)
        grados = (grados > -90.0) ? 0.0 : 180.0;

    canones[turno]->setAngulo(grados);
}

void Juego::disparar(double fuerza)
{
    if (estado != APUNTANDO) return;

    Canon *c = canones[turno];
    const double a = c->getAngulo() * M_PI / 180.0;

    delete proyectil;
    proyectil = new Bola(c->getX(), c->getY(), 5.0, 1.0,
                         turno == 0 ? QColor(Qt::blue) : QColor(Qt::magenta));
    proyectil->establecerVelocidad(Vector2D(fuerza * std::cos(a), -fuerza * std::sin(a)));

    tiempoVuelo = 0.0;
    rebotes = 0;
    estado = EN_VUELO;
}

void Juego::actualizarMundo(double dt)
{
    if (estado != EN_VUELO || !proyectil) return;

    const double h = dt / SUBPASOS;
    for (int k = 0; k < SUBPASOS && estado == EN_VUELO; ++k)
        pasoProyectil(h);

    tiempoVuelo += dt;
    if (estado == EN_VUELO && (tiempoVuelo > TIEMPO_MAX_VUELO || rebotes >= MAX_REBOTES))
        terminarTurno();
}

void Juego::pasoProyectil(double h)
{
    // 1. Gravedad y movimiento
    motor.actualizarMovimiento(proyectil, h);

    // 2. Límites de la caja: perfectamente elásticos (el piso es el límite inferior)
    if (resolutor.resolverPared(proyectil, ANCHO, PISO))
        ++rebotes;

    // 3. Infraestructura: rebota contra los dos bandos, pero solo daña la del rival
    for (int b = 0; b < 2; ++b)
        for (Obstaculo *o : obstaculos[b])
            resolutor.resolverProyectilObstaculo(proyectil, o, b != turno);

    // 4. ¿Alcanzó al representante enemigo?
    if (resolutor.tocaRival(proyectil, rivales[1 - turno])) {
        ganador = turno;
        estado = TERMINADO;
    }
}

void Juego::terminarTurno()
{
    delete proyectil;
    proyectil = nullptr;

    turno = 1 - turno;
    tiempoVuelo = 0.0;
    rebotes = 0;
    estado = APUNTANDO;
}

void Juego::dibujarMundo(QPainter *painter)
{
    for (Pared *p : paredes) p->dibujar(painter);

    for (int i = 0; i < 2; ++i) {
        for (Obstaculo *o : obstaculos[i]) o->dibujar(painter);
        rivales[i]->dibujar(painter);
        jugadores[i]->dibujar(painter);
        canones[i]->dibujar(painter);
    }
    if (proyectil) proyectil->dibujar(painter);

    // Texto de estado
    painter->setPen(Qt::black);
    QFont font = painter->font();

    if (estado == TERMINADO) {
        font.setPointSize(24);
        painter->setFont(font);
        painter->drawText(220, 200, QString("¡VICTORIA DEL JUGADOR %1!").arg(ganador + 1));
        font.setPointSize(10);
        painter->setFont(font);
        painter->drawText(300, 230, "Presiona R para reiniciar");
    } else {
        font.setPointSize(12);
        painter->setFont(font);
        painter->drawText(330, 25, QString("Turno: Jugador %1").arg(turno + 1));
    }
}