#include "Interface.h"

adapter::Interface::Interface( precice::SolverInterface & precice, fvMesh & mesh, std::string meshName, std::vector<std::string> patchNames ) :
	_precice( precice ),
	_meshName( meshName ),
	_patchNames( patchNames ),
	_numDataLocations( 0 ),
	_numDims( 3 )
{
	_meshID = _precice.getMeshID( _meshName );

	for( uint i = 0 ; i < patchNames.size() ; i++ )
	{
		_patchIDs.push_back( mesh.boundaryMesh().findPatchID( patchNames.at( i ) ) );
	}
	_configureMesh( mesh );
    
    /* An interface has only one data buffer, which is shared between several CouplingDataReaders and CouplingDataWriters 
       The initial allocation assumes scalar data, if CouplingDataReaders or -Writers have vector data, it is resized (TODO) */    
	_dataBuffer = new double[_numDataLocations]();
}

void adapter::Interface::_configureMesh( fvMesh & mesh )
{

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
		_numDataLocations += mesh.boundaryMesh()[_patchIDs.at( k )].faceCentres().size();
	}
	int vertexIndex = 0;
	double vertices[3 * _numDataLocations];
	_vertexIDs = new int[_numDataLocations];

	for( uint k = 0 ; k < _patchIDs.size() ; k++ )
	{
        const vectorField & faceCenters = mesh.boundaryMesh()[_patchIDs.at( k )].faceCentres();

		for( uint i = 0 ; i < faceCenters.size() ; i++ )
		{
			vertices[vertexIndex++] = faceCenters[i].x();
			vertices[vertexIndex++] = faceCenters[i].y();
			vertices[vertexIndex++] = faceCenters[i].z();
		}
	}
	_precice.setMeshVertices( _meshID, _numDataLocations, vertices, _vertexIDs );

}

void adapter::Interface::addCouplingDataWriter( std::string dataName, CouplingDataWriter * couplingDataWriter )
{
	couplingDataWriter->setDataID( _precice.getDataID( dataName, _meshID ) );
	couplingDataWriter->setPatchIDs( _patchIDs );
	_couplingDataWriters.push_back( couplingDataWriter );

	if( couplingDataWriter->hasVectorData() )
	{
		// TODO: Resize buffer for vector data (if not already resized)
	}
}

void adapter::Interface::addCouplingDataReader( std::string dataName, adapter::CouplingDataReader * couplingDataReader )
{
	couplingDataReader->setDataID( _precice.getDataID( dataName, _meshID ) );
	couplingDataReader->setPatchIDs( _patchIDs );
	_couplingDataReaders.push_back( couplingDataReader );

	if( couplingDataReader->hasVectorData() )
	{
		// TODO: Resize buffer for vector data (if not already resized)
	}
}

void adapter::Interface::readCouplingData()
{
	if( _precice.isReadDataAvailable() )
	{
		for( uint i = 0 ; i < _couplingDataReaders.size() ; i++ )
		{
			adapter::CouplingDataReader * couplingDataReader = _couplingDataReaders.at( i );

			if( couplingDataReader->hasVectorData() )
			{
				_precice.readBlockVectorData( couplingDataReader->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
			}
			else
			{
				_precice.readBlockScalarData( couplingDataReader->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
			}
			couplingDataReader->read( _dataBuffer );
		}
	}
}

void adapter::Interface::writeCouplingData()
{
	for( uint i = 0 ; i < _couplingDataWriters.size() ; i++ )
	{
		adapter::CouplingDataWriter * couplingDataWriter = _couplingDataWriters.at( i );
		couplingDataWriter->write( _dataBuffer );

		if( couplingDataWriter->hasVectorData() )
		{
			_precice.writeBlockVectorData( couplingDataWriter->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
		}
		else
		{
			_precice.writeBlockScalarData( couplingDataWriter->getDataID(), _numDataLocations, _vertexIDs, _dataBuffer );
		}
	}
}

adapter::Interface::~Interface()
{

	BOOST_LOG_TRIVIAL( info ) << "Destroying interface...";

	for ( uint i = 0 ; i < _couplingDataReaders.size() ; i++ )
	{
		delete _couplingDataReaders.at( i );
	}
	_couplingDataReaders.clear();

	for ( uint i = 0 ; i < _couplingDataWriters.size() ; i++ )
	{
		delete _couplingDataWriters.at( i );
	}
	_couplingDataWriters.clear();
}

