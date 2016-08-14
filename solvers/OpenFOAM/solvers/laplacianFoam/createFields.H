    Info<< "Reading field T\n" << endl;

    volScalarField T
    (
        IOobject
        (
            "T",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );


    Info<< "Reading transportProperties\n" << endl;

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );


    Info<< "Reading conductivity k\n" << endl;

    dimensionedScalar k
    (
        transportProperties.lookup("k")
    );


    Info<< "Reading density rho\n" << endl;

    dimensionedScalar rho
    (
        transportProperties.lookup("rho")
    );


    Info<< "Reading specific heat Cp\n" << endl;

    dimensionedScalar Cp
    (
        transportProperties.lookup("Cp")
    );
