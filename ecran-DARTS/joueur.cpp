#include "joueur.h"

#include <QtDebug>

/**
* @file joueur.cpp
*
* @brief Définition de la classe Joueur
*
* @author Bounoir Fabien
*
* @version 0.1
*
*/

/**
 * @brief constructeur de la classe Joueur
 *
 * @fn Joueur::Joueur
 * @param nom
 * @param score
 * @param nbFlechette
 */
Joueur::Joueur(QString nom, int score , int nbFlechette): nom(nom), score(score), moyenneVolee(0), scoreManchePrecedente(score), nbFlechette(nbFlechette), elimine(false)
{
    qDebug() << Q_FUNC_INFO << nom << " " << score << " " << nbFlechette;
}

/**
 * @brief retourne le nom du joueur
 *
 * @fn Joueur::getNom
 * @return QString
 */
QString Joueur::getNom() const
{
    return this->nom;
}

/**
 * @brief retourne le score du joueur
 *
 * @fn Joueur::getScore
 * @return int
 */
int Joueur::getScore() const
{
    return this->score;
}

/**
 * @brief retourne le score de la manche precedente
 *
 * @fn Joueur::getScoreManchePrecedente
 * @return int
 */
int Joueur::getScoreManchePrecedente() const
{
    return this->scoreManchePrecedente;
}

/**
 * @brief retourne le nombre de flechette du joueur
 *
 * @fn Joueur::getFlechette
 * @return int
 */
int Joueur::getFlechette() const
{
    return this->nbFlechette;
}

/**
 * @brief retourne la moyenne des volees
 *
 * @fn Joueur::getFlechette
 * @return float
 */
int Joueur::getMoyenneVolee() const
{
    return this->moyenneVolee;
}

/**
 * @brief retourne le vector contenant tous les scores des volées precedente
 *
 * @fn Joueur::getHistoriqueVolees
 * @return QVector<float>
 */
QVector<float> Joueur::getHistoriqueVolees() const
{
    return this->historiqueVolees;
}

/**
 * @brief retourne un etat true/false pour savoir si le joueur est eliminé
 *
 * @fn Joueur::getEliminer
 * @return bool
 */
bool Joueur::getEliminer() const
{
    return this->elimine;
}

/**
 * @brief permet de modifier si le joueur est eliminer
 *
 * @fn Joueur::setEliminer
 * @param elimine
 */
void Joueur::setEliminer(bool elimine)
{
    this->elimine = elimine;
}

/**
 * @brief permet de mettre à jour la moyenne des volées
 *
 * @fn Joueur::setMoyenneVolee
 * @param moyenneVolee
 */
void Joueur::setMoyenneVolee(int moyenneVolee)
{
    this->moyenneVolee = moyenneVolee;
}


/**
 * @brief permet de mettre à jour le score du joueur
 *
 * @fn Joueur::setScore
 * @param score
 */
void Joueur::setScore(int score)
{
    this->score = score;
}

/**
 * @brief permet de mettre à jour le score de la manche precedente
 *
 * @fn Joueur::setScoreManchePrecedente
 * @param scoreManchePrecedente
 */
void Joueur::setScoreManchePrecedente(int scoreManchePrecedente)
{
    this->scoreManchePrecedente = scoreManchePrecedente;
}

/**
 * @brief permet de mettre à jour le nombre de flechette du joueur
 *
 * @fn Joueur::setNbFlechette
 * @param nbFlechette
 */
void Joueur::setNbFlechette(int nbFlechette)
{
    this->nbFlechette = nbFlechette;
}

/**
 * @brief Méthode qui ajoute la volée au vecteur contenant l'historique des volées
 *
 * @fn Joueur::addHistoriqueVolees
 * @param volee
 */
void Joueur::addHistoriqueVolees(float volee)
{
    historiqueVolees.push_back(volee);
}