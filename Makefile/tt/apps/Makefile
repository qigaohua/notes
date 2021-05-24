
APPS = app1 app2

all:
	@for app in $(APPS); \
		do $(MAKE) -C $$app; \
	done

app1:
	$(MAKE) -C app1

app2:
	$(MAKE) -C app2


ctags:
	@for app in $(APPS); \
		do $(MAKE) -C $$app $@; \
	done
	ctags -R



clean:
	@for app in $(APPS); \
		do $(MAKE) -C $$app $@; \
	done


.PHONY: all clean ctags $(APPS)



