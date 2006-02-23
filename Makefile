all:	x.signature

install: x.signature
	install -c -m 0644 x.signature ~/.signature

uninstall:
	${RM} ~/.signature
