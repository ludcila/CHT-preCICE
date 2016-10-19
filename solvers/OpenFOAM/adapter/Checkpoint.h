#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <vector>
#include "fvCFD.H"

namespace adapter
{
class Checkpoint
{
protected:
    scalar _couplingIterationTimeValue;
    label _couplingIterationTimeIndex;
    std::vector<volScalarField*> _volScalarFields;
    std::vector<volScalarField*> _volScalarFieldCopies;
    std::vector<volVectorField*> _volVectorFields;
    std::vector<volVectorField*> _volVectorFieldCopies;
    std::vector<surfaceScalarField*> _surfaceScalarFields;
    std::vector<surfaceScalarField*> _surfaceScalarFieldCopies;
    Foam::Time & _runTime;
    void _storeTime();
    void _reloadTime();
    bool _enabled;
public:
    Checkpoint(Foam::Time & runTime, bool checkpointingEnabled);
    void addField(volScalarField & field);
    void addField(volVectorField & field);
    void addField(surfaceScalarField & field);
    void write();
    void read();
};
}

#endif // CHECKPOINT_H
