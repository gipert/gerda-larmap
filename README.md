<img src=".github/gerda-logo.png" align="left"  height="80"/>

# gerda-larmap

Toolbox to produce a detection probability map for 128nm photons in liquid
argon (LAr) in the GERDA experiment.

Put a [gerda-sw-all](https://github.com/mppmu/gerda-sw-all) Singularity
container under `bin/gerda-sw-all.simg`, then run:
```
# compile create-larmap.cc
make

# run MaGe simulations
cd gerda-larmap/gen/jobs && ./run-all-jobs

# create photon maps
cd gerda-larmap/jobs && ./run-all-jobs

# merge everything
gerda-larmap/bin/map-merger jobs/out/*.root
```

You can eventually smooth the final map by applying an average moving window
filter. Use `bin/map-smoother --width n file.root` for that.

`bin/larmap-doctor` is a useful tool to investigate the statistical solidity of
your probability map. Be sure to understand what the program is doing by
inspecting the source code.

## GERDA Tomography

Run `bin/gerdatomography` to start a nice visualization tool for the just created
probability map.
