cc = gcc

CFLAG = -DUSE3RD_STB_IMAGE  -g -fsanitize=address -fno-omit-frame-pointer

test: test.c util_image.o
	$(cc) -o $(@) test.c util_image.o $(CFLAG) -lm

util_image.o: util_image.h util_image.c
	$(cc) -c util_image.c $(CFLAG)

clean:
	rm *.o && rm test
