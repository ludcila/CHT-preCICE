#include "TemperatureBoundaryCondition.h"

ofcoupler::TemperatureBoundaryCondition::TemperatureBoundaryCondition(volScalarField &T) :
    _T(T)
{

}

void ofcoupler::TemperatureBoundaryCondition::readFromBuffer()
{
    scalarField temperatureField(_bufferSize); // TODO: avoid this

    forAll(temperatureField, i) {
        temperatureField[i] = _dataBuffer[i];
    }
    refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[_patchID]) == temperatureField;
    Info << "+++++++++" << temperatureField << endl;
}
