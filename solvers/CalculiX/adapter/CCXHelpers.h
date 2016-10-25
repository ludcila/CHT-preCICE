#ifndef CCXHELPERS_H
#define CCXHELPERS_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../CalculiX.h"

char * toNodeSetName(char * name);
char * toFaceSetName(char * name);
ITG getSetID(char * setName, char * set, ITG nset);
ITG getNumSetElements(ITG setID, ITG * istartset, ITG * iendset);
void getSurfaceElementsAndFaces(ITG setID, ITG * ialset, ITG * istartset, ITG * iendset, ITG * elements, ITG * faces);
void getNodeCoordinates(ITG * nodes, ITG numNodes, double * co, double * coordinates);
void getNodeTemperatures(ITG * nodes, ITG numNodes, double * v, int mt, double * temperatures);
void getTetraFaceCenters(ITG * elements, ITG * faces, ITG numElements, ITG * kon, ITG * ipkon, double * co, double * faceCenters);
void getTetraFaceNodes(ITG * elements, ITG * faces, ITG * nodes, ITG numElements, ITG numNodes, ITG * kon, ITG * ipkon, int * tetraFaceNodes);
void getXloadIndices(char * loadType, ITG * elementIDs, ITG * faceIDs, ITG numElements, ITG nload, ITG * nelemload, char * sideload, ITG * xloadIndices);
void setXload(double * xload, int * xloadIndices, double * values, int numValues, int indexOffset);
void setNodeTemperatures(double * temperatures, ITG numNodes, int * xbounIndices, double * xboun);
bool isSteadyStateSimulation( ITG * nmethod );

#endif // CCXHELPERS_H
