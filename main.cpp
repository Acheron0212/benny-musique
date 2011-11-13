#include <QtGui>

#include "fenmusique.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);


    a.setApplicationName("Lecteur Multimedia");//necessaire a Phonon
    fenMusique w;
    w.show();

    return a.exec();
}
