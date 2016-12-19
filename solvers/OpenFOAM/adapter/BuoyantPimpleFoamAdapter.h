#ifndef BUOYANTPIMPLEFOAMADAPTER_H
#define BUOYANTPIMPLEFOAMADAPTER_H

#include "fvCFD.H"
#include "rhoThermo.H"
#include "turbulentFluidThermoModel.H"
#include "Adapter.h"
#include "ConfigReader.h"

namespace adapter
{

class BuoyantPimpleFoamAdapter : public Adapter
{
protected:

	rhoThermo & _thermo;
	autoPtr<compressible::turbulenceModel> & _turbulence;

public:

	BuoyantPimpleFoamAdapter(
	        std::string participantName,
	        std::string preciceConfigFilename,
	        fvMesh & mesh, Foam::Time & runTime,
	        rhoThermo & thermo,
	        autoPtr<compressible::turbulenceModel> & turbulence,
	        bool subcyclingEnabled = false // disabled by default because it requires explicit checkpointing of the flow fields in the adapter!
	        );
	void createInterfacesFromConfigFile( std::string configFile, std::string participantName );
};

}

#endif // BUOYANTPIMPLEFOAMADAPTER_H
