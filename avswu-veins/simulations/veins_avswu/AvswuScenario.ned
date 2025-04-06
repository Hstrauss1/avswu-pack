
package avswu.simulations.veins_avswu;

import avswu.simulations.veins_avswu.Scenario;
import avswu.simulations.veins_avswu.RSU;
import avswu.simulations.veins_avswu.Manufacturer;

// WORKS @display("p=250,2000;i=veins/node/drone;is=s");

network AvswuScenario extends Scenario
{
    parameters:
        @display("bgb=4000,4000");
    submodules:
        rsu[4]: RSU {
            @display("p=1500,2000");
        }
        man[1]: Manufacturer {
            @display("p=250,2000");
        }
}
