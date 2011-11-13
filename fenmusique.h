#ifndef FENMUSIQUE_H
#define FENMUSIQUE_H

#include <QMainWindow>
#include <phonon/audiooutput.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>
#include <QList>
#include <time.h>
#include <stdlib.h>

class QAction;
class QTableWidget;
class QLCDNumber;

class fenMusique : public QMainWindow
{
    Q_OBJECT

public:
    fenMusique();

    QSize sizeHint() const
    {
             return QSize(600, 350);//(default(500,300)
    }

 private slots:
     void addFiles();
     void stateChanged(Phonon::State newState, Phonon::State oldState);
     void tick(qint64 time);
     void sourceChanged(const Phonon::MediaSource &source);
     void metaStateChanged(Phonon::State newState, Phonon::State oldState);
     void aboutToFinish();
     void finished();
     void tableDoubleClicked(int row, int column);
     void removeFiles();
     void previous();
     void next();
     void random();

 private:
     void setupActions();
     void setupMenus();
     void setupUi();

     Phonon::SeekSlider *seekSlider;
     Phonon::MediaObject *mediaObject;
     Phonon::MediaObject *metaInformationResolver;
     Phonon::AudioOutput *audioOutput;
     Phonon::VolumeSlider *volumeSlider;
     QList<Phonon::MediaSource> sources;

     QAction *playAction;
     QAction *pauseAction;
     QAction *stopAction;
     QAction *nextAction;
     QAction *previousAction;
     QAction *addFilesAction;
     QAction *randomAction;
     QAction *removeFilesAction;
     QAction *exitAction;
     QLCDNumber *timeLcd;
     QTableWidget *musicTable;
};

#endif // FENMUSIQUE_H
