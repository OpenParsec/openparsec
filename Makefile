SUBDIRS=src/
.PHONY: all clean
all: 
	for subdir in $(SUBDIRS); do (cd $${subdir}; $(MAKE) $@); done

clean:
	for subdir in $(SUBDIRS); do (cd $${subdir}; $(MAKE) $@); done

