SUBDIRS = src

.PHONY: main clean install $(SUBDIRS)

main: $(SUBDIRS)

clean: 
	@$(MAKE) TARGET=clean

$(SUBDIRS):
	@echo descending to $@
	@$(MAKE) -C $@ $(TARGET)

install:
	cp ./src/condor ./bin/condor
