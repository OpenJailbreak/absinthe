all:
	make -C ../tools
	@echo "// GreenPois0n Absinthe - offsets.h" >../include/offsets.h
	@echo "// Copyright (C) 2011 Chronic-Dev Team" >>../include/offsets.h
	@for i in `find . -name "dyld_shared_cache_armv7"`;do echo "Analyzing $$i...";../tools/gizmo $$i >./`dirname $$i`/offsets.h;cd ../include;echo "#include \"`dirname ../opt/$$i`/offsets.h\"" >>../include/offsets.h;cd ../opt;done;
	
clean:
	@rm -rf ../include/offsets.h
	@for i in `find . -name "offsets.h"`;do echo "Removing $$i...";rm -rf $$i;done;

maintainer-clean:
	@rm -rf ../include/offsets.h
	@for i in `find . -name "offsets.h"`;do echo "Removing $$i...";rm -rf $$i;done;