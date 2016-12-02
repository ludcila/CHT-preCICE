#include "BuoyantSimpleFoamAdapter.h"

#include "adapter/CouplingDataUser/CouplingDataWriter/HeatTransferCoefficientBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/HeatTransferCoefficientBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/SinkTemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/SinkTemperatureBoundaryCondition.h"

#include <boost/log/trivial.hpp>

using namespace adapter;

BuoyantSimpleFoamAdapter::BuoyantSimpleFoamAdapter(
	std::string participantName,
	std::string configFile,
	fvMesh & mesh,
	Time & runTime,
	std::string solverName,
	rhoThermo & thermo,
	autoPtr<compressible::RASModel> & turbulence,
	bool subcyclingEnabled ) :
	_thermo( thermo ),
	_turbulence( turbulence ),
	Adapter( participantName, configFile, mesh, runTime, solverName, subcyclingEnabled )
{
	createInterfacesFromConfigFile( configFile, participantName );
}

void BuoyantSimpleFoamAdapter::createInterfacesFromConfigFile( std::string configFile, std::string participantName )
{
	ConfigReader config( configFile, participantName );

	for( int i = 0 ; i < config.interfaces().size() ; i++ )
	{

		Interface * interface = new Interface( *_precice, _mesh, config.interfaces().at( i ).meshName, config.interfaces().at( i ).patchNames );
		_interfaces.push_back( interface );

		for( int j = 0 ; j < config.interfaces().at( i ).writeData.size() ; j++ )
		{
			std::string dataName = config.interfaces().at( i ).writeData.at( j );

			if( dataName.find( "Heat-Transfer-Coefficient" ) == 0 )
			{
				interface->addCouplingDataWriter( dataName, new HeatTransferCoefficientBoundaryValues<autoPtr<compressible::RASModel> >( _turbulence ) );
			}
			else if( dataName.find( "Sink-Temperature" ) == 0 )
			{
				interface->addCouplingDataWriter( dataName, new SinkTemperatureBoundaryValues( _thermo.T() ) );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " is not valid";
				exit( 1 );
			}
		}

		for( int j = 0 ; j < config.interfaces().at( i ).readData.size() ; j++ )
		{
			std::string dataName = config.interfaces().at( i ).readData.at( j );

			if( dataName.find( "Heat-Transfer-Coefficient" ) == 0 )
			{
				interface->addCouplingDataReader( dataName, new HeatTransferCoefficientBoundaryCondition<autoPtr<compressible::RASModel> >( _thermo.T(), _turbulence ) );
			}
			else if( dataName.find( "Sink-Temperature" ) == 0 )
			{
				interface->addCouplingDataReader( dataName, new SinkTemperatureBoundaryCondition( _thermo.T() ) );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " is not valid";
				exit( 1 );
			}

		}
	}
}

