#include "Checkpoint.h"

adapter::Checkpoint::Checkpoint( Foam::Time & runTime, bool checkpointingEnabled = true ) :
	_runTime( runTime ),
	_enabled( checkpointingEnabled )
{

}

void adapter::Checkpoint::addField( volScalarField & field )
{
	volScalarField * copy = new volScalarField( field );
	_volScalarFields.push_back( &field );
	_volScalarFieldCopies.push_back( copy );
}

void adapter::Checkpoint::addField( volVectorField & field )
{
	if( _enabled )
	{
		volVectorField * copy = new volVectorField( field );
		_volVectorFields.push_back( &field );
		_volVectorFieldCopies.push_back( copy );
	}
}

void adapter::Checkpoint::addField( surfaceScalarField & field )
{
	if( _enabled )
	{
		surfaceScalarField * copy = new surfaceScalarField( field );
		_surfaceScalarFields.push_back( &field );
		_surfaceScalarFieldCopies.push_back( copy );
	}
}

void adapter::Checkpoint::write()
{

	_storeTime();

	for( int i = 0 ; i < _volScalarFields.size() ; i++ )
	{
		*( _volScalarFieldCopies.at( i ) ) = *( _volScalarFields.at( i ) );
	}

	for( int i = 0 ; i < _volVectorFields.size() ; i++ )
	{
		*( _volVectorFieldCopies.at( i ) ) = *( _volVectorFields.at( i ) );
	}

	for( int i = 0 ; i < _surfaceScalarFields.size() ; i++ )
	{
		*( _surfaceScalarFieldCopies.at( i ) ) = *( _surfaceScalarFields.at( i ) );
	}
}

void adapter::Checkpoint::read()
{

	_reloadTime();

	for( int i = 0 ; i < _volScalarFields.size() ; i++ )
	{
		*( _volScalarFields.at( i ) ) = *( _volScalarFieldCopies.at( i ) );
	}

	for( int i = 0 ; i < _volVectorFields.size() ; i++ )
	{
		*( _volVectorFields.at( i ) ) = *( _volVectorFieldCopies.at( i ) );
	}

	for( int i = 0 ; i < _surfaceScalarFields.size() ; i++ )
	{
		*( _surfaceScalarFields.at( i ) ) = *( _surfaceScalarFieldCopies.at( i ) );
	}
}

void adapter::Checkpoint::_storeTime()
{
	_couplingIterationTimeIndex = _runTime.timeIndex();
	_couplingIterationTimeValue = _runTime.value();
}

void adapter::Checkpoint::_reloadTime()
{
	_runTime.setTime( _couplingIterationTimeValue, _couplingIterationTimeIndex );
	std::cout << "Reset time = " << _couplingIterationTimeValue << " (" << _couplingIterationTimeIndex << ")" << std::endl;
}

