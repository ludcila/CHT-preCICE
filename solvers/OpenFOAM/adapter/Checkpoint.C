#include "Checkpoint.h"

ofcoupler::Checkpoint::Checkpoint(Foam::Time & runTime) :
    _runTime(runTime)
{
    
}

void ofcoupler::Checkpoint::addVolScalarField(volScalarField & field)
{
    volScalarField * copy = new volScalarField(field);
    _volScalarFields.push_back(&field);
    _volScalarFieldCopies.push_back(copy);
}

void ofcoupler::Checkpoint::addVolVectorField(volVectorField & field)
{
    volVectorField * copy = new volVectorField(field);
    _volVectorFields.push_back(&field);
    _volVectorFieldCopies.push_back(copy);
}

void ofcoupler::Checkpoint::addSurfaceScalarField(surfaceScalarField &field)
{
    surfaceScalarField * copy = new surfaceScalarField(field);
    _surfaceScalarFields.push_back(&field);
    _surfaceScalarFieldCopies.push_back(copy);
}

void ofcoupler::Checkpoint::write()
{
    
    _storeTime();
    
    for(int i = 0; i < _volScalarFields.size(); i++) {
        *(_volScalarFieldCopies.at(i)) = *(_volScalarFields.at(i));
    }
    for(int i = 0; i < _volVectorFields.size(); i++) {
        *(_volVectorFieldCopies.at(i)) = *(_volVectorFields.at(i));
    }
    for(int i = 0; i < _surfaceScalarFields.size(); i++) {
        *(_surfaceScalarFieldCopies.at(i)) = *(_surfaceScalarFields.at(i));
    }

}

void ofcoupler::Checkpoint::read()
{
    
    _reloadTime();
    
    for(int i = 0; i < _volScalarFields.size(); i++) {
        *(_volScalarFields.at(i)) = *(_volScalarFieldCopies.at(i));
    }
    for(int i = 0; i < _volVectorFields.size(); i++) {
        *(_volVectorFields.at(i)) = *(_volVectorFieldCopies.at(i));
    }
    for(int i = 0; i < _surfaceScalarFields.size(); i++) {
        *(_surfaceScalarFields.at(i)) = *(_surfaceScalarFieldCopies.at(i));
    }
}

void ofcoupler::Checkpoint::_storeTime()
{
    _couplingIterationTimeIndex = _runTime.timeIndex();
    _couplingIterationTimeValue = _runTime.value();
}

void ofcoupler::Checkpoint::_reloadTime()
{
    _runTime.setTime(_couplingIterationTimeValue, _couplingIterationTimeIndex);
    std::cout << "Reset time = " << _couplingIterationTimeValue << " (" << _couplingIterationTimeIndex << ")" << std::endl;
}
