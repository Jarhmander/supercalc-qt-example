#include "supercalc_plugin_mult.h"

/*
 * L'opération de multiplication
 */
double Supercalc_plugin_mult::operation(double op1, double op2)
{
    return op1 * op2;
}

/*
 * Le nom de l'opération
 */
std::string Supercalc_plugin_mult::get_name() const
{
    return "multiplication (*)";
}

/*
 * La fonction qui sera appelée par l'application.
 *
 * La fonction doit retourner une instance du "plugin".
 */
extern "C" SUPERCALC_PLUGIN_MULTSHARED_EXPORT SuperCalcPluginPtr SuperCalcPlugin_new()
{
    return std::make_unique<Supercalc_plugin_mult>();
}
