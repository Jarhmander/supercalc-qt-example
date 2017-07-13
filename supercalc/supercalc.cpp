#include <QApplication>
#include <QDirIterator>
#include <QMessageBox>
#include <QDebug>
#include <QLibrary>
#include <QWidget>
#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCloseEvent>
#include "supercalc.h"
#include "ui_supercalc.h"

/*
 * Ici nous définissons les "plugins" de base: Addition et Soustraction
 */
class Addition : public SuperCalcPlugin
{
public:
    double operation(double op1, double op2) override { return op1 + op2; }
    std::string get_name() const override { return "addition (+)"; }
};

class Substraction : public SuperCalcPlugin
{
public:
    double operation(double op1, double op2) override { return op1 - op2; }
    std::string get_name() const override { return "soustraction (-)"; }
};

/*
 * Constructeur défini par Qt, avec modification
 */
SuperCalc::SuperCalc(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SuperCalc)
{
    ui->setupUi(this);
    // Ajout de l'appel de cet méthode.
    do_connections();
}

/*
 * Destructeur défini par Qt
 */
SuperCalc::~SuperCalc()
{
    delete ui;
}

/*
 * Méthode (slot) qui affiche le résultat sur l'écran de calculatrice
 */
void SuperCalc::calculate()
{
    /*
     * On recueille les données, on prend l'index de l'élément affiché du combobox pour aller
     * chercher une opération mathématique dans la liste `plugins`, et on effectue le calcul.
     */
    const double op1 = ui->spinbox_1st_arg->value();
    const double op2 = ui->spinbox_2nd_arg->value();

    const unsigned index = ui->combo_operation->currentIndex();

    /*
     * Normalement, étant donné que le combobox et `plugins` ont le même nombre d'éléments, l'index
     * de l'élément désigné par le combobox est toujours valide comme index dans la liste
     * `plugins`... excepté lorsque le combobox est vide, alors l'index est -1. En pleine
     * connaissance de cause, on a pris soin de mettre l'index du combobox dans une variable
     * intégrale non-signée, ce qui a pour effet qu'un nombre négatif tel que -1 devient un très
     * grand nombre positif. Dans un tel cas, on détecte facilement cette condition, car cette
     * valeur dépassera la taille de la liste `plugin`. Également, en présence d'un bogue (erreur de
     * logique) dans le code, on évitera également un plantage car on évite tout accès hors de la
     * liste.
     */
    if (index < plugins.size()) {
        const double result = plugins[index]->operation(op1, op2);
        ui->lcd_result->display(result);
    }
}

/*
 * Réalise les connexions des signaux.
 */
void SuperCalc::do_connections()
{

    /*
     * La méthode QObject::connect est très versatile. On peut connecter un signal à un slot, ou à
     * un autre signal. De plus, il existe deux manières de faire les connexions, soit la nouvelle
     * manière (Qt5) et l'ancienne manière (Qt4, qui utilise les macros SIGNAL et SLOT). Dans ce qui
     * suit on utilise un mélange des deux, juste à guise de démonstration, mais la nouvelle manière
     * est à prévilégier.
     *
     * Cette page du wiki de Qt résume bien les avantages et inconvénients des deux approches:
     * https://wiki.qt.io/New_Signal_Slot_Syntax
     */

    // Connexion signal à slot, style Qt5
    connect(this, &SuperCalc::calculate_needed, this, &SuperCalc::calculate);

    // Connexion signal à signal, style Qt4
    connect(ui->spinbox_1st_arg, SIGNAL(valueChanged(double)), this, SIGNAL(calculate_needed()));

    /*
     * Le signal "valueChanged" a deux surcharges, une acceptant un `double`, et l'autre un `const
     * QString &`. Comme mentionné dans l'article cité, la nouvelle manière a un peu de difficulté
     * avec les méthodes surchargées. Si on tient vraiment à l'utiliser dans un tel cas, cela donne
     * quelque chose de très... "verbeux".
     */
    connect(ui->spinbox_2nd_arg,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            &SuperCalc::calculate_needed);

    /*
     * Comme `currentIndexChanged` a également plusieurs surcharges, nous allons sauver un peu de
     * caractères ici en établissant la connexion à la Qt4.
     *
     * L'inconvénient d'utiliser cette façon de connecter les signaux, c'est que le compilateur ne
     * sera pas en mesure de nous aider si nous faisons une erreur. Par exemple, si on fait une
     * erreur sur le nom du signal, la compilation passe quand même, sans même un avertissement;
     * lorsque le signal sera connecté, nous aurons seulement un message dans la console de débogage
     * qui indiquera que le signal est mal connecté.
     *
     * Il est facile de le vérifier, si l'on change par exemple `currentIndexChanged` par
     * `curentIndexChanged`
     */
    connect(ui->combo_operation, SIGNAL(currentIndexChanged(int)), this, SIGNAL(calculate_needed()));

    /*
     * Manière Qt5. Si l'on fait une erreur sur le nom des classes ou des signaux, une erreur de
     * compilation survient.
     */
    connect(ui->action_Quitter, &QAction::triggered, this, &QWidget::close);

    /*
     * Avec la nouvelle manière de connecter les signaux, on peut même lier des fonctions
     * directement aux signaux, et quand on utilise le C++11 (ou supérieur), on peut même lier des
     * expressions lambda.
     * Par exemple, nous aurions pu définir un slot, ou même une méthode régulière et la lier au
     * signal, cela aura fait pareil... mais dans ce contexte, on peut voir que d'utiliser une
     * expression lambda pour quelque chose d'aussi simple est très concis.
     */
    connect(ui->action_Recharger, &QAction::triggered, [this]{
        reload();
        emit calculate_needed();
    });
}

/*
 * Cette méthode est appelé lorsque le menu "Fichier > Recharcher" est sélectionné.
 */
void SuperCalc::reload()
{
    clear_operations();

    qDebug() << "Chargement plugins...";

    load_builtin_operations();
    load_plugins();

    qDebug() << "Terminé\n";
}

/*
 * Petit utilitaire qui ne fait que charger les opérations de base, "Addition" et "Soustraction".
 */
void SuperCalc::load_builtin_operations()
{
    /*
     * `std::make_unique` est une fonction template qui alloue dynamiquement un objet et retourne son
     * pointeur sous forme d'un `std::unique_ptr`. C'est l'équivalent de faire ceci:
     *
     *     std::unique_ptr<T>(new T(paramètres...))
     *
     * pour un type T donné.
     *
     * Notez que tout comme les conversions de pointeurs d'un type dérivé vers un type de base sont
     * permises (ex: `Addition *` peut se convertir en `SuperCalcPlugin *`, parce que la classe
     * `Addition` hérite de `SuperCalcPlugin`), les conversions de std::unique_ptr<T> sont permises
     * dans les mêmes cas: `std::unique_ptr<Addition>` est convertible en
     * `std::unique_ptr<SuperCalcPlugin>`
     */
    add_operation(std::make_unique<Addition>());
    add_operation(std::make_unique<Substraction>());
}

/*
 * Utilitaire qui traverse les fichiers du répertoire contenant l'application, et tente de les
 * charger en tant que "plugin".
 */
void SuperCalc::load_plugins()
{
    static const QDir::Filters exec = QDir::Files | QDir::Readable;

    QDirIterator it { QApplication::applicationDirPath(), exec };

    while (it.hasNext()) {
        // la méthode `next` retourne un nom de fichier et "avance" l'itérateur au fichier suivant.
        // Donc, à chaque itération de cette présente boucle, `it.next()` retourne un nom de fichier
        // qui se trouve dans le répertoire où se trouve l'application.
        try_load_library(it.next());
    }
}

/*
 * Tente de charger un "plugin"; si le chargement réussit, l'ajoute via `add_operation`.
 */
void SuperCalc::try_load_library(const QString &file)
{
    // On affiche le nom du fichier dans la console de débogage.
    qDebug() << file;

    /*
     * Chargement de la librairie. Si l'on tente de charger un fichier qui n'est pas une
     * bibliothèque dynamique, rien de spécial ne se passera. Évidemment, dans ce cas, la méthode
     * `resolve` retournera toujours un pointeur nul.
     */
    QLibrary lib { file };

    /*
     * La "magie" opère sur cette ligne. On recherche un symbole qui est la fonction
     * `SuperCalcPlugin_new`, dans la librairie dynamique. Si elle existe, on peut appeler cette
     * fonction qui retournera une instance fraîche d'un "plugin".
     *
     * Le résultat de `QLibrary::resolve` nécessite un transtypage, car la fonction n'est évidemment
     * pas en mesure de déterminer le type du symbole recherché; on présume le connaître.
     *
     * La ligne ci-dessous aurait pu être écrit comme suit, et effectivement, avant C++11, c'est
     * ce qui aurait marché:
     *
     *     typedef SuperCalcPlugin (*FactoryFunction)();
     *     FactoryFunction factory = reinterpret_cast<FactoryFunction>(lib.resolve("SuperCalcPlugin_new"));
     *
     * L'utilisation de `auto` nous permet de ne pas nous répéter. L'utilisation de `decltype` nous
     * permet de déduire le type de la fonction, et en ajoutant un astérisque, va donner le bon type
     * de pointeur de fonction pour accepter une fonction qui a la même signature que
     * `SuperCalcPlugin_new`, nous évitant une autre forme de répétition.
     *
     * Pour l'usage de `QLibrary` avec la liaison explicite, voir ce lien:
     * https://wiki.qt.io/How_to_create_a_library_with_Qt_and_use_it_in_an_application#Using_QLibrary_to_load_the_shared_library
     */
    typedef decltype(SuperCalcPlugin_new) *FactoryFunction;

    auto factory = reinterpret_cast<FactoryFunction>(lib.resolve("SuperCalcPlugin_new"));

    if (factory) {
        add_operation(factory());
    }
}

/*
 * Efface tous les éléments de la combobox d'opération ainsi que de la liste `plugins`.
 */
void SuperCalc::clear_operations()
{
    ui->combo_operation->clear();
    plugins.clear();
}

/*
 * Utilitaire qui ajoute une nouvelle opération au combobox et à la liste `plugins`.
 */
void SuperCalc::add_operation(SuperCalcPluginPtr plugin)
{
    ui->combo_operation->addItem(plugin->get_name().c_str());

    /*
     * Comme mentionné dans "supercalcplugin.h", un `SuperCalcPlugin` n'est pas copiable, il faut
     * utiliser `std::move` si l'on veut le passer en paramètre. Par contre, une fois que l'objet
     * est transféré de la sorte, il est invalide; il ne faut pas essayer d'utiliser l'instance
     * `plugin` après l'instruction suivante.
     */
    plugins.push_back(std::move(plugin));
}
