CC = gcc
CFLAG = -O3 -DUSE3RD_STB_IMAGE

GTK_CFG = `pkg-config --cflags --libs gtk+-2.0`

SRC  = global_fn.c ctrl_panel.c preview.c
OBJ  = sys_fn.o util_image.o random_draw.o

# For mingw
ifeq ($(OS),Windows_NT)
	RM = del
	BIN = redraw.exe
	LINK = -Wl,--subsystem,windows -lmingw32 -lSDL2main -lSDL2 -lopengl32 \
	-lm -lGLU32 -lGLAUX -lpthread -lcomdlg32
	CFLAG += -DUSING_WINAPI
else # For linux
	RM = rm
	BIN = redraw
	LINK = -lm -lSDL2 -lGL -lGLU -lpthread $(GTK_CFG)
	CFLAG += -DUSING_GTK $(GTK_CFG)
endif

$(BIN): redraw.c $(SRC) $(OBJ)
	$(CC) -o $(@) redraw.c $(OBJ) $(CFLAG) $(LINK)
sys_fn.o: sys_fn.h sys_fn.c
	$(CC) -c sys_fn.c $(CFLAG)
random_draw.o: random_draw.h random_draw.c
	$(CC) -c random_draw.c $(CFLAG)
util_image.o: utim/util_image.h utim/util_image.c
	$(CC) -c utim/util_image.c $(CFLAG)

clean:
	$(RM) *.o && $(RM) $(BIN)
