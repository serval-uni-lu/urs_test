PYSRC = chi2.py

chi2.sif: base.sif chi2.def ${PYSRC}
	singularity build --fakeroot "chi2.sif" "chi2.def"

base.sif: base.def
	singularity build --fakeroot "base.sif" "base.def"
