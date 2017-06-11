#!/bin/sh
(
	# autoreconf -fvi
	aclocal --install
	autoconf --force
	automake --add-missing --copy --force-missing
	./configure
	make dist
	rm -rf RPM
	mkdir -p RPM/{SOURCES,RPMS,SRPMS,BUILD,SPECS}
	rpmbuild							\
		-D"_topdir ${PWD}/RPM"					\
		-D"_sourcedir ${PWD}"					\
		-D"_specdir ${PWD}"					\
		-ba							\
		signature.spec
) 2>&1 | tee bootstrap.log
