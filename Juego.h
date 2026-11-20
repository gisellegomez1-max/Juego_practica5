#ifndef JUEGO_H
#define JUEGO_H

#include <vector>
#include <QPainter>
#include "Bola.h"
#include "Canon.h"
#include "Jugador.h"
#include "Pared.h"
#include "Obstaculo.h"
#include "Rival.h"
#include "MotorFisica.h"
#include "ResolutorColisiones.h"

class Juego
{
public:
    enum Estado { APUNTANDO, EN_VUELO, TERMINADO };

    Juego();
    ~Juego();

    void inicializarJuego();             // también sirve para reiniciar
    void actualizarMundo(double dt);
    void dibujarMundo(QPainter *painter);

    void setAnguloCanon(double grados);  // cañón del jugador en turno
    void disparar(double fuerza);

    Estado getEstado() const { return estado; }
    int getTurno() const { return turno; }
    double getXCanonActual() const { return canones[turno]->getX(); }
    double getYCanonActual() const { return canones[turno]->getY(); }

    const std::vector<Obstaculo *> &getObstaculos(int i) const { return obstaculos[i]; }
    const Canon *getCanon(int i) const { return canones[i]; }
    const Rival *getRival(int i) const { return rivales[i]; }
    const Jugador *getJugador(int i) const { return jugadores[i]; }
    const Bola *getProyectil() const { return proyectil; }
    int getGanador() const { return ganador; }

private:
    void limpiar();
    void crearBando(int i);              // 0 = izquierda, 1 = derecha
    void pasoProyectil(double h);
    void terminarTurno();

    Estado estado = APUNTANDO;
    int turno = 0;                       // 0 o 1
    int ganador = -1;
    double tiempoVuelo = 0.0;
    int rebotes = 0;

    // Un elemento por jugador
    Jugador *jugadores[2] = { nullptr, nullptr };
    Canon *canones[2] = { nullptr, nullptr };
    Rival *rivales[2] = { nullptr, nullptr };
    std::vector<Obstaculo *> obstaculos[2];   // obstaculos[i] = infraestructura del jugador i

    Bola *proyectil = nullptr;
    std::vector<Pared *> paredes;

    MotorFisica motor;
    ResolutorColisiones resolutor;
};

#endif // JUEGO_H