#ifndef BUOYANTSIMPLEFOAMADAPTER_H
#define BUOYANTSIMPLEFOAMADAPTER_H

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "ConfigReader.h"

namespace adapter
{
class BuoyantSimpleFoamAdapter : public Adapter
{
protected:

	rhoThermo & _thermo;
	autoPtr<compressible::RASModel> & _turbulence;

public:

	BuoyantSimpleFoamAdapter(
	        std::string participantName,
	        std::string preciceConfigFilename,
	        fvMesh & mesh, Foam::Time & runTime,
	        rhoThermo & thermo,
	        autoPtr<compressible::RASModel> & turbulence,
	        bool subcyclingEnabled = false
	        );
	virtual void createInterfacesFromConfigFile( std::string configFile, std::string participantName );
};

}

#endif // BUOYANTSIMPLEFOAMADAPTER_H
