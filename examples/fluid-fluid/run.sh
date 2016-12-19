buoyantPimpleFoam_preCICE -case bottom-fluid -precice-participant Bottom-Fluid > Bottom-Fluid.log &
pid0=$!

buoyantPimpleFoam_preCICE -case top-fluid -precice-participant Top-Fluid > Top-Fluid.log &
pid1=$!

wait $pid0
wait $pid1
