#include "BuoyantPimpleSinkTemperatureBoundaryValues.h"

adapter::BuoyantPimpleSinkTemperatureBoundaryValues::BuoyantPimpleSinkTemperatureBoundaryValues(volScalarField &T, rhoThermo &thermo, autoPtr<compressible::turbulenceModel> &turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}


void adapter::BuoyantPimpleSinkTemperatureBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;
    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField flux = - _turbulence->alphaEff()().boundaryField()[patchID]
                * _thermo.Cp()().boundaryField()[patchID]
                * _T.boundaryField()[patchID].snGrad();

        std::cout << "Sink temperature" << std::endl;
        forAll(flux, i) {
            double h = 16e3;
            dataBuffer[bufferIndex++] = _T.boundaryField()[patchID][i] - flux[i] / h;
            std::cout << i << ") " << "Tfw = " << _T.boundaryField()[patchID][i] << "\tTfl = " << dataBuffer[bufferIndex-1] << std::endl;
        }

    }

}
