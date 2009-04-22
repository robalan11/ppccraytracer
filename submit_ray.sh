#!/bin/bash

mpirun -np 2 -mode VN -env BGLMPI_MAPPING=TXYZ -cwd /gpfs/small/PPCC/home/PPCCpitm/raytracer/ \
/gpfs/small/PPCC/home/PPCCpitm/raytracer/render -size 64 48 -input reflective_spheres.obj \
-num_shadow_samples 1 -num_bounces 3 -background_color 0.5 0.6 0.9 -camera_position 5 3 10 \
-poi 0.5 0 -0.5 > `printf "%d_LOG" $SLURM_JOB_ID` 2> `printf "ERR_%d" $SLURM_JOB_ID`
