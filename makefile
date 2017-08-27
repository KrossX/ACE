#NMAKE

CC_FLAGS = /c /O1 /Og /Gs /MD /nologo
LINKER_FLAGS = /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /NOLOGO

all: ace.exe
	@del *.obj
	@del *.res

ace.exe : ace.obj ace.res
	@link ace.obj ace.res user32.lib advapi32.lib $(LINKER_FLAGS)

ace.obj: src/ace.c
	@cl src/ace.c $(CC_FLAGS)

ace.res: src/ace.rc
	@rc /fo ace.res src/ace.rc