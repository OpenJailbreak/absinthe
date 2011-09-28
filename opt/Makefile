all:
	make -C ../tools;
	@for i in `find . -name "dyld*"`;do echo "Analyzing $$i...";../tools/gizmo $$i >./`dirname $$i`/offsets.h;done;
	
clean:
	@for i in `find . -name "offsets.h"`;do echo "Removing $$i...";rm -rf $$i;done;
