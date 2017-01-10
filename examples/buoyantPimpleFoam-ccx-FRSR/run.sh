 #!/bin/bash


# Run
buoyantPimpleFoam_preCICE -case Fluid -precice-participant Fluid > Fluid.log &
PIDFluid=$!
ccx_preCICE -i Solid/solid -precice-participant Solid > Solid.log &
PIDSolid=$!

# Wait for the run to finish
wait $PIDFluid
wait $PIDSolid

rm diff.txt
diff convergence-Fluid.txt convergence-Fluid.txt.ref &>> diff.txt
sample -case Fluid -time 1
diff Fluid/postProcessing/sets/1/line_T.xy line_T.xy &>> diff.txt
wc -c diff.txt
