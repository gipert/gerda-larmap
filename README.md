# gerda-larmap

Toolbox to produce a detection probability map for 128nm photons in liquid argon in the GERDA experiment.

Put a [gerda-sw-all](https://github.com/mppmu/gerda-sw-all) Singularity container in the main directory, then run:
```
# compile create-larmap.cc
make

# run MaGe simulations (with qsub)
cd gen && ./run-all-jobs

# create photon maps
cd .. && ./run-all-jobs

# merge everything
./map-merger out/*.root
```

## GERDA Tomography

Run `./gerdatomography` to start a nice visualization tool for the just created probability map.
