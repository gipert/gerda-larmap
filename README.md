# gerda-larmap

Toolbox to produce a detection probability map for 128nm photons in liquid argon in the GERDA experiment.

Put a [gerda-sw-all](https://github.com/mppmu/gerda-sw-all) Singularity container under `bin/gerda-sw-all.simg`, then run:
```
# compile create-larmap.cc
make

# run MaGe simulations (with qsub)
cd gerda-larmap/gen/jobs && ./run-all-jobs

# create photon maps
cd gerda-larmap/jobs && ./run-all-jobs

# merge everything
gerda-larmap/map-merger jobs/out/*.root
```

You can eventually smooth the final map by applying an average moving window filter. Use `map-smoother --width n file.root` for that.

## GERDA Tomography

Run `./gerdatomography` to start a nice visualization tool for the just created probability map.
