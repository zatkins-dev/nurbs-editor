CPP = g++ -std=c++14
INC = -I../cryphutil -I../ImageReader -I../fontutil -I../glslutil -I../mvcutil

# >>> FOR LINUX, uncomment next few lines; comment out the MAC ones.
C_FLAGS = -fPIC -g -c $(INC) -DGL_GLEXT_PROTOTYPES
GL_LIB_LOC = -L/usr/lib/nvidia-375
GL_LIBRARIES = $(GL_LIB_LOC) -lglfw -lGLU -lGL
MAKE = make
BUILD = fedora
# >>> FOR MAC, uncomment next few lines; comment out previous linux ones.
# C_FLAGS = -fPIC -g -c $(INC) -DGLFW_INCLUDE_GLEXT -DGLFW_INCLUDE_GLCOREARB -DGL_SILENCE_DEPRECATION
# GL_LIBRARIES = -L/usr/local/lib -lglfw -framework OpenGL
# MAKE = make -f MakefileMac
# BUILD = mac
# >>> END: FOR LINUX - FOR MAC

LINK = g++ -fPIC -g -Wall
LOCAL_UTIL_LIBRARIES = ../lib/libcryph.so ../lib/libfont.so ../lib/libglsl.so ../lib/libImageReader.so ../lib/libmvc.so ./lib/libnanogui.so

OBJS = main.o SceneElement.o InteractiveAffPoint.o Curve.o CardinalSpline.o BezierCurve.o

main: ensure-dirs $(OBJS) $(LOCAL_UTIL_LIBRARIES)
	$(LINK) -o $@ $(OBJS) $(LOCAL_UTIL_LIBRARIES) $(GL_LIBRARIES)

../lib/libcryph.so: ../cryphutil/AffVector.h ../cryphutil/AffVector.c++ ../cryphutil/AffPoint.h ../cryphutil/AffPoint.c++
	(cd ../cryphutil; make)

../lib/libfont.so: ../fontutil/CFont.h ../fontutil/CFont.c++ ../fontutil/CGLString.h ../fontutil/CGLString.c++
	(cd ../fontutil; $(MAKE))

../lib/libglsl.so: ../glslutil/ShaderIF.h ../glslutil/ShaderIF.c++
	(cd ../glslutil; $(MAKE))

../lib/libImageReader.so: ../ImageReader/ImageReader.h ../ImageReader/ImageReader.c++
	(cd ../ImageReader; $(MAKE))

../lib/libmvc.so: ../mvcutil/Controller.h ../mvcutil/Controller.c++ ../mvcutil/ModelView.h ../mvcutil/ModelView.c++
	(cd ../mvcutil; $(MAKE))

./lib/libnanogui.so: 
	echo "Building NanoGUI...\n" 
	mkdir ./deps/nanogui/build/$(BUILD)
	cd ./deps/nanogui/build/$(BUILD)
	cmake ../.. -D NANOGUI_BACKEND:STRING=OpenGL -D NANOGUI_BUILD_EXAMPLES:BOOL=OFF -D NANOGUI_BUILD_GLAD:BOOL=OFF -D NANOGUI_BUILD_GLFW:BOOL=OFF -D NANOGUI_BUILD_PYTHON:BOOL=OFF -D NANOGUI_BUILD_SHARED:BOOL=ON -D NANOGUI_INSTALL:BOOL=OFF
	make; ln -s libnanogui.so ../../../../lib/libnanogui.so

obj/main.o: main.c++
	$(CPP) $(C_FLAGS) $< -o $@

.PHONY : clean
clean: 
	rm -rf obj bin main

.PHONY : clean-all
clean-all: clean
	rm -rf  ../cryphutil/*.o ../cryphutil/*.so \
		../fontutil/*.o ../fontutil/*.so \
		../glslutil/*.o ../glslutil/*.so \
		../ImageReader/*.o ../ImageReader/*.so \
		../mvcutil/*.o ../mvcutil/*.so \
		../lib/*

.PHONY : ensure-dirs
ensure-dirs:
	mkdir -p ./obj
	mkdir -p ./bin
	mkdir -p ../lib

.PHONY : dist
dist: tar
	echo "Making tarball"

.PHONY : tar
tar: clean
	tar -czvf ../$(shell basename '$(CURDIR)'.tar.gz) ../$(shell basename '$(CURDIR)')