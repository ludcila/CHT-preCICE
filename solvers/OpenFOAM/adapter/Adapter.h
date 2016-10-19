#ifndef COUPLER_H
#define COUPLER_H

#include <string>
#include <vector>
#include "fvCFD.H"
#include "precice/SolverInterface.hpp"
#include "Interface.h"
#include "Checkpoint.h"

namespace adapter
{
class Adapter
{
protected:

	precice::SolverInterface & _precice;
	std::vector<Interface*> _interfaces;
	fvMesh & _mesh;
	Foam::Time & _runTime;
	std::string _solverName;

	bool _checkpointingIsEnabled = false;
	scalar _couplingIterationTimeValue;
	label _couplingIterationTimeIndex;
	std::vector<volScalarField*> _volScalarFields;
	std::vector<volScalarField*> _volScalarFieldCopies;
	std::vector<volVectorField*> _volVectorFields;
	std::vector<volVectorField*> _volVectorFieldCopies;
	std::vector<surfaceScalarField*> _surfaceScalarFields;
	std::vector<surfaceScalarField*> _surfaceScalarFieldCopies;
	void _storeCheckpointTime( );
	void _reloadCheckpointTime( );

public:

	Adapter( precice::SolverInterface & precice, fvMesh & mesh, Time & runTime, std::string solverName );

	precice::SolverInterface & precice( )
	{
		return _precice;
	}

	Interface & addNewInterface( std::string meshName, std::vector<std::string> patchNames );

	void configure( );
	void initialize( );
	void initializeData( );
	void receiveCouplingData( );
	void sendCouplingData( );
	void advance( );
	void adjustTimeStep( );

	void enableCheckpointing( );
	void addCheckpointField( volScalarField & field );
	void addCheckpointField( volVectorField & field );
	void addCheckpointField( surfaceScalarField & field );
	void readCheckpoint( );
	void writeCheckpoint( );

	~Adapter( );
};

}

#endif // COUPLER_H
