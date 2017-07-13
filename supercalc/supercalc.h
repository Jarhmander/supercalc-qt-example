#ifndef SUPERCALC_H
#define SUPERCALC_H

#include <QMainWindow>
#include "supercalcplugin.h"
#include <vector>

// Dans le dialogue de nouveau projet, j'ai donné le nom "SuperCalc" au formulaire principal (étant
// également le seul formulaire de tout le projet). C'est pourquoi l'on retrouve "SuperCalc" au lieu
// de "MainWindow".
namespace Ui {
class SuperCalc;
}

class SuperCalc : public QMainWindow
{
    Q_OBJECT

public:
    // Constructeur et destructeur déclarés par Qt.
    explicit SuperCalc(QWidget *parent = 0);
    ~SuperCalc();

signals:
    // Notre signal central qui met à jour le résultat affiché. Plusieurs signaux du formulaire
    // principal (le seul formulaire) seront connectés à ce signal.
    void calculate_needed();

public slots:
    // Le signal `calculate_needed` sera connecté à ce slot.
    void calculate();

private:
    Ui::SuperCalc *ui;

    /*
     * Une liste de "plugin".
     *
     * std::vector est un template qui provient de la librairie standard du C++.
     */
    std::vector<SuperCalcPluginPtr> plugins;

private:
    // Plusieurs méthodes utilitaires.
    void do_connections();
    void reload();
    void load_builtin_operations();
    void load_plugins();
    void try_load_library(const QString &file);
    void clear_operations();
    void add_operation(SuperCalcPluginPtr plugin);
};

#endif // SUPERCALC_H
