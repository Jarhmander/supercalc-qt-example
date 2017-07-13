/*
 * Définition d'un "plugin".
 *
 * Un "plugin" doit inclure cet en-tête et définir une classe qui hérite de `SuperCalcPlugin`, et
 * définir une fonction `SuperCalcPlugin_new` retournant une instance fraîche du "plugin" (la classe
 * venant d'être définie).
 */
#ifndef SUPERCALCPLUGIN_H
#define SUPERCALCPLUGIN_H

#include <string>
#include <memory>

/*
 * Classe de base pour tout "plugin".
 *
 * Il s'agit d'une classe abstraite, c'est-à-dire qui comporte au moins une méthode virtuelle pure.
 * (Une méthode virtuelle pure est déclarée avec le mot-clef "virtual" et a ce curieux " = 0". Une
 * telle classe ne peut être instanciée. De plus, une classe dérivée doit implémenter les méthodes
 * abstraites, sinon cette classe dérivée est également abstraite et donc ne peut être instanciée.
 *
 * Un peu de terminologie: une classe abstraite ayant uniquement des méthodes virtuelles pures est
 * appelé également "interface". Une interface est une classe abstraite, mais l'inverse n'est pas
 * forcément vrai. Lorsqu'une classe hérite d'une interface et définit les méthodes abstraites, on
 * dit que la classe "implémente" une interface.
 *
 * L'avantage de faire en sorte qu'un plugin soit basé sur une interface, c'est bien sûr pour
 * exploiter le polymorphisme: cela permet de pouvoir appeler des méthodes d'intérêt sur le "plugin"
 * sans avoir à ce que l'application ait à connaître la classe du "plugin". L'autre avantage, c'est
 * qu'une interface est une classe minimale qui documente précisément le genre d'opération à
 * supporter pour une classe dérivée.
 */
class SuperCalcPlugin
{
public:
    virtual ~SuperCalcPlugin() {}

    virtual double operation(double op1, double op2) = 0;
    virtual std::string get_name() const = 0;
};

/*
 * Un typedef pour simplifier l'écriture.
 *
 * Un `unique_ptr` est un pointeur intelligent (template) qui pointe sur un objet alloué
 * dynamiquement. L'avantage par rapport à un pointeur brut est que la mémoire est gérée
 * automatiquement: l'équivalent de "delete p" est appelé sur le pointeur géré, par défaut, lorsque
 * le pointeur est détruit. Par contre, le pointeur n'est pas copiable, il peut seulement être
 * transféré (à l'aide de std::move).
 *
 * Ce typedef désigne donc un pointeur sur un "plugin" quelconque, donc la mémoire sera libérée
 * automatiquement sur destruction du pointeur.
 */
typedef std::unique_ptr<SuperCalcPlugin> SuperCalcPluginPtr;

/*
 * Cette fonction déclaré n'est pas défini dans l'application; il ne faut pas l'appeler. Par contre,
 * il faut définir cette fonction dans un "plugin".
 *
 * Chaque "plugin" contiendra la définition de cette fonction. Un "plugin" est simplement une
 * bibliothèque dynamique (un fichier ".dll" sous Windows et un fichier ".so" sous Linux).
 * L'application peut donc charger une bibliothèque dynamique, rechercher cette fonction (avec
 * QLibrary::resolve) et l'appeler.
 *
 * Comme mentionné précédemment, cette fonction doit retourner une nouvelle instance d'un "plugin".
 */
extern "C" SuperCalcPluginPtr SuperCalcPlugin_new();

#endif // SUPERCALCPLUGIN_H
