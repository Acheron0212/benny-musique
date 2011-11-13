#include <QtGui>

#include "fenmusique.h"

fenMusique::fenMusique()//creation de la fenetre principale
{
    audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    mediaObject = new Phonon::MediaObject(this);
    metaInformationResolver = new Phonon::MediaObject(this);

    mediaObject->setTickInterval(1000);//defini l'interval où on envoi l'information du positionnement de la lecture dans le fichier/source lu
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(metaInformationResolver, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(metaStateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
    connect(mediaObject, SIGNAL(finished()), this, SLOT(finished()));

    Phonon::createPath(mediaObject, audioOutput);//la connexion qui fait tout

    setupActions();
    setupMenus();
    setupUi();
    timeLcd->display("00:00");
}

void fenMusique::addFiles()//declenché quand on clique sur "add a File"
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Selection Fichier Musique"),
        QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    if (files.isEmpty())
        return;

    int index = sources.size();
    foreach (QString string, files)
    {
        Phonon::MediaSource source(string);

        sources.append(source);
    }
    if (!sources.isEmpty())
    {
        metaInformationResolver->setCurrentSource(sources.at(index));
    }
}


void fenMusique::removeFiles()
{
   if (!sources.isEmpty())
    {
       if(mediaObject->currentSource() == sources[musicTable->currentRow()])
       {
           mediaObject->stop();
           mediaObject->clearQueue();
           mediaObject->clear();//erreur mais fonctionne normalement
       }
        sources.removeAt(musicTable->currentRow());
        musicTable->removeRow(musicTable->currentRow());
    }
}


void fenMusique::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    switch (newState)
    {
        case Phonon::ErrorState:
            if (mediaObject->errorType() == Phonon::FatalError) {
                QMessageBox::warning(this, tr("Erreur Fatale"),
                mediaObject->errorString());
            } else {
                QMessageBox::warning(this, tr("Erreur"),
                mediaObject->errorString());
            }
            break;
        case Phonon::PlayingState:
                playAction->setEnabled(false);
                pauseAction->setEnabled(true);
                stopAction->setEnabled(true);
                break;
        case Phonon::StoppedState:
                stopAction->setEnabled(false);
                playAction->setEnabled(true);
                pauseAction->setEnabled(false);
                previousAction->setEnabled(true);
                nextAction->setEnabled(true);
                timeLcd->display("00:00");
                break;
        case Phonon::PausedState:
                pauseAction->setEnabled(false);
                stopAction->setEnabled(true);
                playAction->setEnabled(true);
                break;
        case Phonon::BufferingState:
                break;
        default:
            ;
    }
}

void fenMusique::previous()//on implemente ces fonctions puisque pas automatisées par Phonon
{
    mediaObject->stop();
    int index = sources.indexOf(mediaObject->currentSource()) - 1;
    mediaObject->clearQueue();
    if (!index)
        mediaObject->setCurrentSource(sources.at(index));
    mediaObject->play();
}

void fenMusique::next()//on implemente ces fonctions puisque pas automatisées par Phonon
{
    mediaObject->stop();
    int index = sources.indexOf(mediaObject->currentSource()) + 1;
    mediaObject->clearQueue();
    if (sources.size() > index)
        mediaObject->setCurrentSource(sources.at(index));
    mediaObject->play();
}

//le temps est en milisecond, il convient de le convertir pour pouvoir ensuite l'afficher
void fenMusique::tick(qint64 time)
{
    QTime displayTime(0, (time / 60000) % 60, (time / 1000) % 60);

    timeLcd->display(displayTime.toString("mm:ss"));
}

void fenMusique::tableDoubleClicked(int row, int /* column */)
{

    mediaObject->stop();
    mediaObject->clearQueue();

    if (row >= sources.size())
        return;

    mediaObject->setCurrentSource(sources[row]);

    mediaObject->play();
}

void fenMusique::random()
{
    mediaObject->stop();
    mediaObject->clearQueue();
    srand (time(NULL));
    int i = rand()% sources.size() + 0;
    mediaObject->setCurrentSource(sources[i]);
    mediaObject->play();
}



void fenMusique::sourceChanged(const Phonon::MediaSource &source)
{
    musicTable->selectRow(sources.indexOf(source));
    timeLcd->display("00:00");
}

void fenMusique::metaStateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    if (newState == Phonon::ErrorState) {
        QMessageBox::warning(this, tr("Erreur de l'ouverture du fichier"),
            metaInformationResolver->errorString());
        while (!sources.isEmpty() &&
               !(sources.takeLast() == metaInformationResolver->currentSource())) {}  /* boucle */;
        return;
    }

    if (newState != Phonon::StoppedState && newState != Phonon::PausedState)
        return;

    if (metaInformationResolver->currentSource().type() == Phonon::MediaSource::Invalid)
            return;

    QMap<QString, QString> metaData = metaInformationResolver->metaData();//contient l'ensemble des données meta du fichier

    QString title = metaData.value("TITLE");
    if (title == "")
        title = metaInformationResolver->currentSource().fileName();

//Chacun des éléments meta sont décomposés puis stockés dans des QtableWidgetItem, il convient de connaître l'intitulé de chaque élément meta

    QTableWidgetItem *tNumberItem = new QTableWidgetItem(metaData.value("TRACKNUMBER"));//new
    tNumberItem->setFlags(tNumberItem->flags() ^ Qt::ItemIsEditable);//new
    QTableWidgetItem *titleItem = new QTableWidgetItem(title);
    titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *artistItem = new QTableWidgetItem(metaData.value("ARTIST"));
    artistItem->setFlags(artistItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *albumItem = new QTableWidgetItem(metaData.value("ALBUM"));
    albumItem->setFlags(albumItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *yearItem = new QTableWidgetItem(metaData.value("DATE"));
    yearItem->setFlags(yearItem->flags() ^ Qt::ItemIsEditable);


//Les QtableWidgetItem composés de chaque élément meta sont insérés dans le tableau d'affichage QtableWidget

    int currentRow = musicTable->rowCount();
    musicTable->insertRow(currentRow);
    musicTable->setItem(currentRow, 1, tNumberItem);//new
    musicTable->setItem(currentRow, 0, titleItem);
    musicTable->setItem(currentRow, 2, artistItem);
    musicTable->setItem(currentRow, 3, albumItem);
    musicTable->setItem(currentRow, 4, yearItem);


    if (musicTable->selectedItems().isEmpty())
    {
        musicTable->selectRow(0);
        mediaObject->setCurrentSource(metaInformationResolver->currentSource());
    }

    Phonon::MediaSource source = metaInformationResolver->currentSource();// pense bête
    int index = sources.indexOf(metaInformationResolver->currentSource()) + 1;
    if (sources.size() > index)
    {
        metaInformationResolver->setCurrentSource(sources.at(index));
    }
    else
    {
        musicTable->resizeColumnsToContents();
        if (musicTable->columnWidth(0) > 300)
        {
            musicTable->setColumnWidth(0, 300);
        }
        musicTable->horizontalHeader()->setStretchLastSection(true);//new
    }
}


void fenMusique::aboutToFinish()
{
    int index = sources.indexOf(mediaObject->currentSource()) + 1;
    if (sources.size() > index)
        mediaObject->enqueue(sources.at(index));
}

void fenMusique::finished()
{
    timeLcd->display("00:00");
}

void fenMusique::setupActions()
{
    playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    playAction->setShortcut(tr("Ctrl+P"));
    playAction->setDisabled(true);

    pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    pauseAction->setShortcut(tr("Ctrl+A"));
    pauseAction->setDisabled(true);

    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setShortcut(tr("Ctrl+S"));
    stopAction->setDisabled(true);

    nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
    nextAction->setShortcut(tr("Ctrl+N"));
    //nextAction->setDisabled(true);
    previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
    previousAction->setShortcut(tr("Ctrl+R"));
    //previousAction->setDisabled(true);
    //new
    randomAction = new QAction(style()->standardIcon(QStyle::SP_MessageBoxQuestion), tr("Random"), this);
    //pas de raccourci inutile
    // end of new
    addFilesAction = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Ajout &Fichier"), this);
    addFilesAction->setShortcut(tr("Ctrl+F"));

    removeFilesAction = new QAction(style()->standardIcon(QStyle::SP_DialogDiscardButton), tr("Enlever Fichier"), this);
    removeFilesAction->setShortcut(tr("Ctrl+E"));

    exitAction = new QAction(tr("Q&uitter"), this);
    exitAction->setShortcuts(QKeySequence::Quit);

    //on peut voire la difference des connexions avec certaines fonctions automatisés
    connect(playAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
    connect(pauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(previousAction, SIGNAL(triggered()), this, SLOT(previous()));
    connect(nextAction, SIGNAL(triggered()), this, SLOT(next()));
    connect(randomAction, SIGNAL(triggered()), this, SLOT(random()));
    connect(addFilesAction, SIGNAL(triggered()), this, SLOT(addFiles()));
    connect(removeFilesAction, SIGNAL(triggered()), this, SLOT(removeFiles()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

}

void fenMusique::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Fichier"));
    fileMenu->addAction(addFilesAction);
    fileMenu->addAction(removeFilesAction);//new
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);


}

void fenMusique::setupUi()
{
    QToolBar *barre = new QToolBar;

    barre->addAction(previousAction);
    barre->addAction(playAction);
    barre->addAction(pauseAction);
    barre->addAction(stopAction);
    barre->addAction(nextAction);
    barre->addSeparator();
    barre->addAction(randomAction);
    barre->addSeparator();
    barre->addSeparator();
    barre->addAction(removeFilesAction);

    seekSlider = new Phonon::SeekSlider(this);
    seekSlider->setMediaObject(mediaObject);

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setAudioOutput(audioOutput);
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QLabel *volumeLabel = new QLabel;
    volumeLabel->setPixmap(QPixmap("images/volume.png"));

    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);//changer visuel

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);

    QStringList headers;
    headers  << tr("Title") << tr("N°") << tr("Artist") << tr("Album") << tr("Year") << "";

    musicTable = new QTableWidget(0, 6);//new (0,5)old
    musicTable->setGridStyle(Qt::DotLine);//new
    //musicTable->setShowGrid(false);//new
    musicTable->setHorizontalHeaderLabels(headers);

    musicTable->horizontalHeader()->setStretchLastSection(true);//new
    //musicTable->horizontalHeader()->resizeMode(QHeaderView::Stretch);

    musicTable->setSelectionMode(QAbstractItemView::SingleSelection);
    musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    musicTable->setContextMenuPolicy(Qt::DefaultContextMenu);//new

    connect(musicTable, SIGNAL(cellDoubleClicked(int,int)),this, SLOT(tableDoubleClicked(int,int)));

    QHBoxLayout *seekerLayout = new QHBoxLayout;
    seekerLayout->addWidget(seekSlider);
    seekerLayout->addWidget(timeLcd);

    QHBoxLayout *playbackLayout = new QHBoxLayout;
    playbackLayout->addWidget(barre);
    playbackLayout->addStretch();
    playbackLayout->addWidget(volumeLabel);
    playbackLayout->addWidget(volumeSlider);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(musicTable);
    mainLayout->addLayout(seekerLayout);
    mainLayout->addLayout(playbackLayout);

    QWidget *widget = new QWidget;
    widget->setLayout(mainLayout);

    setCentralWidget(widget);
    setWindowTitle("BennyMusique");
}
