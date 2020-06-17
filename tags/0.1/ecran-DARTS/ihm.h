#ifndef IHM_H
#define IHM_H

/**
 * @file ihm.h
 * @brief Déclaration de la classe Ihm (Module Ecran-DARTS)
 *
 * @version 0.1
 *
 * @author Bounoir Fabien
 */

#include "communication.h"
#include "darts.h"
#include <QWidget>
#include <QTimer>
#include <QSound>

/**
 * @def PERIODE_HORLOGE
 * @brief Définit la périodicité de l'horloge en millisecondes
 */
#define PERIODE_HORLOGE     1000

/**
 * @def CHEMIN_FICHIER_MUSIQUE
 * @brief Définit le chemin pour les sons
 */
#define CHEMIN_FICHIER_MUSIQUE "/son/"

namespace Ui {
class Ihm;
}

/**
 * @class Ihm
 * @brief Déclaration de la classe Ihm (Module Ecran-DARTS)
 * @details Cette classe s'occupe de l'affichage sur l'écran
*/
class Ihm : public QWidget
{
    Q_OBJECT

public:
    explicit Ihm(QWidget *parent = nullptr);
    ~Ihm();

private:
    Ui::Ihm *ui;                        //!< object de notre Ihm
    QTimer *timerHorloge;               //!< objet timerHorloge
    Communication *communication;       //!< objet communication
    Darts *darts;                       //!< objet darts
    QSound musique;                     //!< objet musique
    QSound musiquePause;                //!< objet musiquePause
    int compteurDureePartie;            //!< compteur de secondes pour la durée d'une séance
    QPixmap sauvegardeImpactEncours;    //!< sauvegarde le pixmap de l'état de la cible
    QString messageStatut;              //!< contient le message de statut qui est affiché

    /**
     * @enum Page
     * @brief Définit les différentes pages de l'IHM
     *     
     */
    enum Page
    {
        PageAttente = 0,
        PageJeu,
        PageStatistique,
        NbPages
    };

    void attribuerRaccourcisClavier();
    void initialiserEvenements();
    void initialiserHorloge();
    void mettreAJourMessageStatut(int typePoint, int point);

public slots:
    void actualiserHeure();
    void allerPage(Ihm::Page page);
    void allerPagePrecedente();
    void allerPageSuivante();
    void fermerApplication();
    void afficherAttenteConfiguration();
    void afficherAttenteConnexion();
    void afficherImpact(int typePoint, int point);
    void afficherPartie();
    void mettreAJourScore();
    void mettreAJourManche();
    void mettreAJourMoyenneVolee();
    void afficherVoleeAnnulee();
    void afficherNouvellePartie();
    void finirPartie(QString gagnant, int voleeMaxJoueur);
    void mettreAJourJoueur();
    void afficherDureePartie();
    void mettreAJoursolution(QString solution);
    void mettrePausePartie();
    void relancerpartie();
    void mettreAJourCible();
    void mettreAJourMessageStatut(QString);
    void jouerSon(QString son);
};

#endif // IHM_H