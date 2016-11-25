#ifndef ADAPTER_H
#define ADAPTER_H

#include <mpi.h>
#include <string>
#include <vector>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
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
	bool _subcyclingEnabled = false;

	scalar _couplingIterationTimeValue;
	label _couplingIterationTimeIndex;
	std::vector<volScalarField*> _volScalarFields;
	std::vector<volScalarField*> _volScalarFieldCopies;
	std::vector<volVectorField*> _volVectorFields;
	std::vector<volVectorField*> _volVectorFieldCopies;
	std::vector<surfaceScalarField*> _surfaceScalarFields;
	std::vector<surfaceScalarField*> _surfaceScalarFieldCopies;

	/**
	 * @brief Makes a copy of the Foam::Time object
	 */
	void _storeCheckpointTime();

	/**
	 * @brief Restores the copy of the Foam::Time object
	 */
	void _reloadCheckpointTime();

	/**
	 * @brief Returns true if MPI is used
	 */
	bool _isMPIUsed();

	/**
	 * @brief Returns the MPI rank
	 */
	int _getMPIRank();

	/**
	 * @brief Returns the MPI size
	 */
	int _getMPISize();

public:

	/**
	 * @brief Adapter
	 * @param participantName: Name of the current participant
	 * @param configFilename: name of the .yml file
	 * @param mesh: OpenFOAM mesh object
	 * @param runTime
	 * @param solverName
	 * @param subcyclingEnabled: Whether subcycling is implemented for this solver
	 */
	Adapter(
	        std::string participantName,
	        std::string configFilename,
	        fvMesh & mesh, Foam::Time & runTime,
	        std::string solverName,
	        bool subcyclingEnabled = false // disabled by default because it requires explicit checkpointing of the flow fields in the adapter!
	        );

    /**
     * @brief createInterfacesFromConfigFile
     * @param configFilename
     * @param participantName
     */
    virtual void createInterfacesFromConfigFile( std::string configFilename, std::string participantName ) {};

	/**
	 * @brief Creates a new interface to be handled by preCICE
	 * @param meshName: Name of the surface mesh as specified in precice-config.xml
	 * @param patchNames: Names of the patches that are part of this interface, as specified in the OpenFOAM case
	 * @return Reference to the created interface
	 */
	Interface & addNewInterface( std::string meshName, std::vector<std::string> patchNames );

	/**
	 * @brief initialize
	 */
	void initialize();

	/**
	 * @brief Receives the coupling data by calling precice::readBlockScalarData for each interface
	 */
	void readCouplingData();

	/**
	 * @brief Sends the coupling data by calling precice::writeBlockScalarData for each interface
	 */
	void writeCouplingData();

	/**
	 * @brief Advances the coupling
	 */
	void advance();

	/**
	 * @brief Adjusts the solver time step based on the coupling time step determined by preCICE
	 */
	void adjustSolverTimeStep();

	/**
	 * @brief Returns true if the coupling is still ongoing
	 */
	bool isCouplingOngoing();

	/**
	 * @brief Determines whether the coupling iteration is complete
	 */
	bool isCouplingTimeStepComplete();

	/**
	 * @brief Returns true if checkpoint must be written
	 */
	bool isReadCheckpointRequired();

	/**
	 * @brief Returns true if checkpoint must be read
	 */
	bool isWriteCheckpointRequired();

	/**
	 * @brief Tells preCICE that the checkpoint has been read
	 */
	void fulfilledReadCheckpoint();

	/**
	 * @brief Tells preCICE that the checkpoint has been written
	 */
	void fulfilledWriteCheckpoint();

	/**
	 * @brief Set whether checkpointing is enabled
	 */
	void setCheckpointingEnabled( bool value );

	/**
	 * @brief Returns true if checkpointing is enabled
	 */
	bool isCheckpointingEnabled();

	/**
	 * @brief Adds a volScalarField for checkpointing
	 */
	void addCheckpointField( volScalarField & field );

	/**
	 * @brief Adds a volVectorField for checkpointing
	 */
	void addCheckpointField( volVectorField & field );

	/**
	 * @brief Adds a surfaceScalarField for checkpointing
	 */
	void addCheckpointField( surfaceScalarField & field );

	/**
	 * @brief Restores checkpointed fields and time
	 */
	void readCheckpoint();

	/**
	 * @brief Stores fields and time
	 */
	void writeCheckpoint();

	virtual ~Adapter();
};

}

#endif // ADAPTER_H
