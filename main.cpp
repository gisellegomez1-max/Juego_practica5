#include <QApplication>
#include "VistaJuego.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VistaJuego vista;
    vista.setWindowTitle("Simulador de Física - Práctica 5");
    vista.show();

    return app.exec();
}