#ifndef COUPLER_H
#define COUPLER_H

#include <mpi.h>
#include <string>
#include <vector>
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"
#include "Interface.h"

namespace adapter
{
class Adapter
{
protected:

	precice::SolverInterface * _precice;
	std::vector<Interface*> _interfaces;
	fvMesh & _mesh;
	Foam::Time & _runTime;
	std::string _solverName;

	double _preciceTimeStep;
	double _solverTimeStep;

	bool _checkpointingIsEnabled = true;
	scalar _couplingIterationTimeValue;
	label _couplingIterationTimeIndex;
	std::vector<volScalarField*> _volScalarFields;
	std::vector<volScalarField*> _volScalarFieldCopies;
	std::vector<volVectorField*> _volVectorFields;
	std::vector<volVectorField*> _volVectorFieldCopies;
	std::vector<surfaceScalarField*> _surfaceScalarFields;
	std::vector<surfaceScalarField*> _surfaceScalarFieldCopies;
	void _storeCheckpointTime();
	void _reloadCheckpointTime();

	bool _isMPIUsed();
	int _getMPIRank();
	int _getMPISize();

public:

	Adapter( std::string participantName, std::string preciceConfigFilename, fvMesh & mesh, Foam::Time & runTime, std::string solverName );

	Interface & addNewInterface( std::string meshName, std::vector<std::string> patchNames );

	void initialize();
	void receiveCouplingData();
	void sendCouplingData();
	void advance();
	void adjustTimeStep( bool forcePreciceTimeStep = false );
	bool isCouplingOngoing();
	void checkCouplingTimeStepComplete();

	bool isReadCheckpointRequired();
	bool isWriteCheckpointRequired();
	void fulfilledReadCheckpoint();
	void fulfilledWriteCheckpoint();
	void setCheckpointingEnabled( bool value );
	bool isCheckpointingEnabled();
	void addCheckpointField( volScalarField & field );
	void addCheckpointField( volVectorField & field );
	void addCheckpointField( surfaceScalarField & field );
	void readCheckpoint();
	void writeCheckpoint();

	~Adapter();
};

}

#endif // COUPLER_H
