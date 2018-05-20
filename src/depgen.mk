depend: depend.mk

depend.mk: *.[ch]
	for i in *.c; do \
		echo -n "$$i: "; \
		awk '/^#[ \t]*include[ \t]+".*"[ \t]*$$/ { sub(/^#[ \t]*include[ \t]+"/, ""); sub(/"[ \t]*$$/, ""); printf "%s ", $$0; }' "$$i"; \
		echo; \
	done > depend.mk

cookies:
	@echo "[1mI don't know how to do that :'([0m"
	@exit 1

include depend.mk

