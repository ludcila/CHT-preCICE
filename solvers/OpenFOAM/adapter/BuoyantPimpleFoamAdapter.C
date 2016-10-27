#include "BuoyantPimpleFoamAdapter.h"

#include "adapter/CouplingDataUser/CouplingDataReader/BuoyantPimpleHeatFluxBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataReader/TemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/BuoyantPimpleHeatFluxBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/TemperatureBoundaryValues.h"

#include "adapter/CouplingDataUser/CouplingDataReader/SinkTemperatureBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/SinkTemperatureBoundaryValues.h"
#include "adapter/CouplingDataUser/CouplingDataReader/HeatTransferCoefficientBoundaryCondition.h"
#include "adapter/CouplingDataUser/CouplingDataWriter/HeatTransferCoefficientBoundaryValues.h"

#include <boost/log/trivial.hpp>

using namespace adapter;

BuoyantPimpleFoamAdapter::BuoyantPimpleFoamAdapter(
        std::string participantName,
        std::string configFile,
        fvMesh & mesh,
        Time & runTime,
        std::string solverName,
        rhoThermo & thermo,
        autoPtr<compressible::turbulenceModel> & turbulence,
        bool subcyclingEnabled ) :
	_thermo( thermo ),
	_turbulence( turbulence ),
	Adapter( participantName, configFile, mesh, runTime, solverName, subcyclingEnabled )
{
	createInterfacesFromConfigFile( configFile, participantName );
}

void BuoyantPimpleFoamAdapter::createInterfacesFromConfigFile( std::string configFile, std::string participantName )
{
	ConfigReader config( configFile, participantName );

	for( int i = 0 ; i < config.interfaces().size() ; i++ )
	{

		Interface * interface = new Interface( *_precice, _mesh, config.interfaces().at( i ).meshName, config.interfaces().at( i ).patchNames );
		_interfaces.push_back( interface );

		for( int j = 0 ; j < config.interfaces().at( i ).writeData.size() ; j++ )
		{
			std::string dataName = config.interfaces().at( i ).writeData.at( j );

			if( dataName.compare( "Temperature" ) == 0 )
			{
				TemperatureBoundaryValues * bw = new TemperatureBoundaryValues( _thermo.T() );
				interface->addCouplingDataWriter( dataName, bw );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				BuoyantPimpleHeatFluxBoundaryValues * bw = new BuoyantPimpleHeatFluxBoundaryValues( _thermo.T(), _thermo, _turbulence );
				interface->addCouplingDataWriter( dataName, bw );
			}
			else if( dataName.find( "Heat-Transfer-Coefficient" ) == 0 )
			{
				HeatTransferCoefficientBoundaryValues<autoPtr<compressible::turbulenceModel> > * bw = new HeatTransferCoefficientBoundaryValues<autoPtr<compressible::turbulenceModel> >( _turbulence );
				interface->addCouplingDataWriter( dataName, bw );
			}
			else if( dataName.find( "Sink-Temperature" ) == 0 )
			{
				RefTemperatureBoundaryValues * bw = new RefTemperatureBoundaryValues( _thermo.T() );
				interface->addCouplingDataWriter( dataName, bw );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " does not exist.";
				exit( 1 );
			}
		}

		for( int j = 0 ; j < config.interfaces().at( i ).readData.size() ; j++ )
		{
			std::string dataName = config.interfaces().at( i ).readData.at( j );

			if( dataName.compare( "Temperature" ) == 0 )
			{
				TemperatureBoundaryCondition * br = new TemperatureBoundaryCondition( _thermo.T() );
				interface->addCouplingDataReader( dataName, br );
			}
			else if( dataName.compare( "Heat-Flux" ) == 0 )
			{
				BuoyantPimpleHeatFluxBoundaryCondition * br = new BuoyantPimpleHeatFluxBoundaryCondition( _thermo.T(), _thermo, _turbulence );
				interface->addCouplingDataReader( dataName, br );
			}
			else if( dataName.find( "Heat-Transfer-Coefficient" ) == 0 )
			{
				HeatTransferCoefficientBoundaryCondition<autoPtr<compressible::turbulenceModel> > * br = new HeatTransferCoefficientBoundaryCondition<autoPtr<compressible::turbulenceModel> >( _thermo.T(), _turbulence );
				interface->addCouplingDataReader( dataName, br );
			}
			else if( dataName.find( "Sink-Temperature" ) == 0 )
			{
				SinkTemperatureBoundaryCondition * br = new SinkTemperatureBoundaryCondition( _thermo.T() );
				interface->addCouplingDataReader( dataName, br );
			}
			else
			{
				BOOST_LOG_TRIVIAL( error ) << "Error: " << dataName << " does not exist.";
				exit( 1 );
			}
		}
	}
}

