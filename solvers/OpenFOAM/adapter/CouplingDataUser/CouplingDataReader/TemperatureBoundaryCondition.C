#include "TemperatureBoundaryCondition.h"

adapter::TemperatureBoundaryCondition::TemperatureBoundaryCondition(volScalarField &T) :
    _T(T)
{

}

void adapter::TemperatureBoundaryCondition::read(double * dataBuffer)
{

    int bufferIndex = 0;

    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        std::cout << patchID << std::endl;

        int size = _T.boundaryField()[patchID].size();

        std::cout << size << std::endl;

        scalarField temperatureField(size); // TODO: avoid this?
        forAll(temperatureField, i) {
            temperatureField[i] = dataBuffer[bufferIndex++];
        }
        refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[patchID]) == temperatureField;

        // Info << "Temperature" << temperatureField << endl;

    }

}
