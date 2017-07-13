#ifndef SUPERCALC_PLUGIN_MULT_H
#define SUPERCALC_PLUGIN_MULT_H

#include "supercalc_plugin_mult_global.h"
#include "../supercalc/supercalcplugin.h"

// Classe de multiplication.
class Supercalc_plugin_mult : public SuperCalcPlugin
{

public:
    Supercalc_plugin_mult() {}
    double operation(double op1, double op2) override;
    std::string get_name() const override;
};

#endif // SUPERCALC_PLUGIN_MULT_H
