#include "BuoyantPimpleHeatFluxBoundaryValues.h"


ofcoupler::BuoyantPimpleHeatFluxBoundaryValues::BuoyantPimpleHeatFluxBoundaryValues(volScalarField &T, rhoThermo &thermo, autoPtr<compressible::turbulenceModel> &turbulence) :
    _T(T),
    _thermo(thermo),
    _turbulence(turbulence)
{

}


void ofcoupler::BuoyantPimpleHeatFluxBoundaryValues::write(double * dataBuffer)
{
    int bufferIndex = 0;
    for(int k = 0; k < _patchIDs.size(); k++) {

        int patchID = _patchIDs.at(k);

        scalarField flux = - _turbulence->alphaEff()().boundaryField()[patchID]
                * _thermo.Cp()().boundaryField()[patchID]
                * refCast<fixedValueFvPatchScalarField>(_T.boundaryField()[patchID]).snGrad();

        std::cout << "Flux" << std::endl;
        forAll(flux, i) {
            dataBuffer[bufferIndex++] = flux[i];
            std::cout << flux[i] << std::endl;
        }

    }

}
