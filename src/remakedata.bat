copy ..\win32\gtskins.bin
copy ..\win32\charset.bin chargen.bin
del gt2stereo.dat
del goatdata.c
del goatdata.o
make -f makefile.win