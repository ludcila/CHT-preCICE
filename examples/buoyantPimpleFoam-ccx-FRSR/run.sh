 #!/bin/bash


# Run
buoyantPimpleFoam_preCICE -case Fluid -precice-participant Fluid > Fluid.log &
PIDFluid=$!
ccx_preCICE -i Solid/solid -precice-participant Solid > Solid.log &
PIDSolid=$!

# Wait for the run to finish
wait $PIDFluid
wait $PIDSolid
