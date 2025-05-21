PYSRC = src/chi2.py src/samplers.py src/tests.py src/util.py

chi2.sif: base.sif chi2.def ${PYSRC}
	rm -f chi2.sif
	singularity build --fakeroot "chi2.sif" "chi2.def"

base.sif: base.def
	rm -f base.sif
	singularity build --fakeroot "base.sif" "base.def"
