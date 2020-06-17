#include "communication.h"
#include <QDebug>

/**
* @file communication.cpp
*
* @brief Définition de la classe Communication (Module Ecran-DARTS)
*
* @author Bounoir Fabien
*
* @version 0.2
*
*/

/**
 * @brief constructeur de la classe Communication
 *
 * @fn Communication::Communication
 * @param darts
 * @param parent
 */
Communication::Communication(Darts *darts, QObject *parent) : QObject(parent), darts(darts), serveur(nullptr), socket(nullptr), localDeviceName("Ecran-Darts"), trame("")
{
    qDebug() << Q_FUNC_INFO;

    parametrerBluetooth();

    miseAJourEtatPartieAttente();
}

/**
 * @brief destructeur de la classe Communication
 *
 * @fn Communication::~Communication
 */
Communication::~Communication()
{
    arreter();
    localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    qDebug() << Q_FUNC_INFO;
}

/**
 * @brief Méthode qui retourne l'état de la partie
 *
 * @fn Communication::getEtatPartie
 * @return int
 */
int Communication::getEtatPartie()
{
    return etatPartie;
}

/**
 * @brief configure la communication Bluetooth
 *
 * @fn Communication::parametrerBluetooth
 */
void Communication::parametrerBluetooth()
{
    if (!localDevice.isValid())
    {
        qDebug() << Q_FUNC_INFO << "Communication Bluetooth locale valide : " << localDevice.isValid();
        emit erreurBluetooth("Communication Bluetooth locale valide : " + QString::number(localDevice.isValid()));

        return;
    }
    else
    {
        // active le bluetooth
        localDevice.powerOn();

        // lire le nom de l'appareil local
        localDeviceName = localDevice.name();


        //localDevice.setHostMode(QBluetoothLocalDevice::HostConnectable);

        //ou

        //les appareil qui ne sont pas jumelé peuvent decouvrir ecran-DARTS
        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

        QList<QBluetoothAddress> remotes;
        remotes = localDevice.connectedDevices();

        connect(&localDevice, SIGNAL(deviceConnected(QBluetoothAddress)), this, SLOT(deviceConnected(QBluetoothAddress)));
        connect(&localDevice, SIGNAL(deviceDisconnected(QBluetoothAddress)), this, SLOT(deviceDisconnected(QBluetoothAddress)));
        connect(&localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)), this, SLOT(error(QBluetoothLocalDevice::Error)));

        connect(darts, SIGNAL(etatPartieFini()), this , SLOT(miseAJourEtatPartieFin()));
        connect(darts, SIGNAL(changerEtatPartie()), this , SLOT(miseAJourEtatPartieEnCours()));
    }
}

/**
 * @brief Démarre le serveur
 *
 * @fn Communication::demarrer
 */
void Communication::demarrer()
{
    if (!localDevice.isValid())
        return;

    if (!serveur)   //Démarre le serveur s'il n'est pas déjà démarré
    {
        serveur = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
        connect(serveur, SIGNAL(newConnection()), this, SLOT(nouveauClient()));

        QBluetoothUuid uuid = QBluetoothUuid(serviceUuid);
        serviceInfo = serveur->listen(uuid, serviceNom);
    }
}

/**
 * @brief arrete le serveur
 *
 * @fn Communication::arreter
 */
void Communication::arreter()
{
    if (!localDevice.isValid())
        return;

    if (!serveur)
        return;

    serviceInfo.unregisterService();

    if (socket)
    {
        if (socket->isOpen())
        {
           socket->close();
        }
        delete socket;
        socket = nullptr;
    }

    delete serveur;
    serveur = nullptr;
}

/**
 * @brief Méthode appelée quand un nouveau client se connecte
 *
 * @fn Communication::nouveauClient
 */
void Communication::nouveauClient()
{
    // on récupère la socket
    socket = serveur->nextPendingConnection();
    if (!socket)
        return;

    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
}

/**
 * @brief Méthode appelée quand une trame est disponible
 *
 * @fn Communication::socketReadyRead
 */
void Communication::socketReadyRead()
{
    QByteArray donnees;

    while (socket->bytesAvailable())
    {
        donnees += socket->readAll();
        usleep(150000); // cf. timeout
    }

    trame = QString(donnees);

    qDebug() << Q_FUNC_INFO << QString::fromUtf8("Trame reçues : ") << QString(donnees);

    decomposerTrame();
}

/**
 * @brief Méthode qui decompose la trame reçue et signale l'état en fonction du type de trame
 *
 * @fn Communication::decomposerTrame
 */
void Communication::decomposerTrame()
{
    if(estValide()) //test si la trame est valide
    {
        trame.remove(DELIMITEUR_FIN);
        if(trame.contains("START") && (etatPartie == EtatPartie::Attente || etatPartie == EtatPartie::Fin))      /** $DART;START;501;1;2;fabien;erwan */
        {
            emit resetPartie();
            darts->reinitialiserPartie();

            QString modeJeu;
            QStringList joueurs;

            extraireParametresTrameStart(joueurs, modeJeu);
        }
        else if(trame.contains("GAME") && etatPartie == EtatPartie::EnCours)      /** $DART;GAME;3;7 */
        {
            darts->receptionnerImpact(trame.section(";",2,2).toInt(), trame.section(";",3,3).toInt());
        }
        else if(trame.contains("REGLE")&& etatPartie != EtatPartie::Regle)   /** $DART;REGLE */
        {
            extraireParametresTrameRegle();
        }
        else if(trame.contains("PAUSE") && etatPartie == EtatPartie::EnCours)     /** $DART;PAUSE */
        {
            emit pause();
            miseAJourEtatPartiePause();
        }
        else if(trame.contains("PLAY") && etatPartie == EtatPartie::Pause)       /** $DART;PLAY */
        {
            emit play();
            miseAJourEtatPartieEnCours();
        }
        else if(trame.contains("RESET")) // quelque soit l'état de la partie    /** $DART;RESET */
        {
            reamorcerPartie();
        }
        else if(trame.contains("STOP") && (etatPartie == EtatPartie::EnCours || etatPartie == EtatPartie::Pause))  /** $DART;STOP */
        {
            darts->arreterPartie();
        }
        else if(trame.contains("STOP") && (etatPartie == EtatPartie::Regle))  /** $DART;STOP */ //permet l'arret des regles en cours de lecture
        {
            emit stopperRegle();
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "Trame non Traité: " << trame;
        }
    }
}

/**
 * @brief Méthode qui teste si la trame reçu est valide
 *
 * @fn Communication::estValide
 * @return bool
 */
bool Communication::estValide()
{
     if(trame.startsWith(TYPE_TRAME) && trame.endsWith(DELIMITEUR_FIN) && trame.contains(";"))
     {
         return true;
     }
     else
     {
         qDebug() << Q_FUNC_INFO << "Trame non Valide: " << trame;
         return false;
     }
}

/**
 * @brief Méthode qui decompose la trame Start
 *
 * @fn Communication::extraireParametresTrameStart
 * @param joueurs
 * @param modeJeu
 */
void Communication::extraireParametresTrameStart(QStringList &joueurs, QString &modeJeu)
{
    modeJeu = trame.section(";",2,2);

    if(trame.section(";",4,4).toInt() == 0) //test effectuer pour verifier que la trame n'est pas une trame de la version du protocol DARTS 0.2
        return;

    for(int i = 0;i <= trame.section(";",4,4).toInt();i++)  //boucle qui recuperer les noms des differents joueurs
    {
        if(trame.section(";",4+i,4+i) == "")    //test si le joueur a un nom
        {
            joueurs.push_back("Joueur[" + QString::number(i) + "]");
        }
        else
        {
            joueurs.push_back(trame.section(";",4+i,4+i));
        }
    }

    darts->initialiserPartie(joueurs, modeJeu);

    if(trame.section(";",3,3) == "1")
    {
        if(etatPartie != EtatPartie::Pause)
            emit pause();

        emit afficherRegle(darts->testerModeDeJeu());
    }

}

/**
 * @brief Méthode qui decompose la trame regle
 *
 * @fn Communication::extraireParametresTrameRegle
 */
void Communication::extraireParametresTrameRegle()
{
    QString regle ="";

    if(etatPartie != EtatPartie::Pause)
        emit pause();

    if(trame.section(";",2,2).contains("DOUBLE_OUT"))
    {
        emit afficherRegle("DOUBLE_OUT");
    }
    else if(trame.section(";",2,2) == "" && (etatPartie == EtatPartie::EnCours || etatPartie == EtatPartie::Pause))
    {
        emit afficherRegle(darts->testerModeDeJeu());
    }
    else
    {
        emit afficherRegle("SANS_DOUBLE_OUT");
    }
}

/**
 * @brief Méthode qui reinitialise la partie
 *
 * @fn Communication::reamorcerPartie
 */
void Communication::reamorcerPartie()
{
    emit resetPartie();
    darts->reinitialiserPartie();
    miseAJourEtatPartieAttente();
}

/**
 * @brief Méthode appelée quand le socket est déconnecté
 *
 * @fn Communication::socketDisconnected
 */
void Communication::socketDisconnected()
{
    QString message = QString::fromUtf8("Périphérique déconnecté ");
    qDebug() << Q_FUNC_INFO << message;
}

/**
 * @brief Méthode appelée quand l'appareil est connecté
 *
 * @fn Communication::deviceConnected
 * @param adresse
 */
void Communication::deviceConnected(const QBluetoothAddress &adresse)
{
    qDebug() << Q_FUNC_INFO << adresse << localDevice.pairingStatus(adresse);
    QString message = QString::fromUtf8("Demande connexion du client ") + adresse.toString();
    emit appareilConnecter();
    if (localDevice.pairingStatus(adresse) == QBluetoothLocalDevice::Paired || localDevice.pairingStatus(adresse) == QBluetoothLocalDevice::AuthorizedPaired)
        message += " [" + QString::fromUtf8("appairé") + "]";
    else
        message += " [" + QString::fromUtf8("non appairé") + "]" ;
    qDebug() << message << endl;

    if(etatPartie == EtatPartie::Pause) // si l'appareil est reconnecte, la partie reprend
    {
        emit play();
        miseAJourEtatPartieEnCours();
    }
}

/**
 * @brief Méthode appelée quand l'appareil est deconnecté
 *
 * @fn Communication::deviceDisconnected
 * @param adresse
 */
void Communication::deviceDisconnected(const QBluetoothAddress &adresse)
{
    qDebug() << Q_FUNC_INFO << adresse;
    emit afficherAttenteConnexion();

    if(etatPartie == EtatPartie::EnCours) // si l'appareil se deconnecte pendant la partie, il la met donc en pause
    {
        emit pause();

        miseAJourEtatPartiePause();
    }
}

/**
 * @brief Méthode appelée quand il y a une erreur avec l'appareil connecté
 *
 * @fn Communication::error
 * @param erreur
 */
void Communication::error(QBluetoothLocalDevice::Error erreur)
{
    qDebug() << Q_FUNC_INFO << erreur;
}

/**
 * @brief Méthode qui met à jour l'état de la partie Attente
 *
 * @fn Communication::miseAJourEtatPartieAttente
 */
void Communication::miseAJourEtatPartieAttente()
{
    qDebug() << Q_FUNC_INFO << "EtatPartie::Attente";
    etatPartie = EtatPartie::Attente;
}

/**
 * @brief Méthode qui met à jour l'état de la partie Pause
 *
 * @fn Communication::miseAJourEtatPartiePause
 */
void Communication::miseAJourEtatPartiePause()
{
    qDebug() << Q_FUNC_INFO << "EtatPartie::Pause";
    etatPartie = EtatPartie::Pause;
}

/**
 * @brief Méthode qui met à jour l'état de la partie Fin
 *
 * @fn Communication::miseAJourEtatPartieFin
 */
void Communication::miseAJourEtatPartieFin()
{
    qDebug() << Q_FUNC_INFO << "EtatPartie::Fin";
    etatPartie = EtatPartie::Fin;
}

/**
 * @brief Méthode qui met à jour l'état de la partie EnCours
 *
 * @fn Communication::miseAJourEtatPartieEnCours
 */
void Communication::miseAJourEtatPartieEnCours()
{
    qDebug() << Q_FUNC_INFO << "EtatPartie::EnCours";
    etatPartie = EtatPartie::EnCours;
}

/**
 * @brief Méthode qui met à jour l'état de la partie en regle
 *
 * @fn Communication::miseAJourEtatPartieRegle
 */
void Communication::miseAJourEtatPartieRegle()
{
    qDebug() << Q_FUNC_INFO << "EtatPartie::Regle";
    etatPartie = EtatPartie::Regle;
}