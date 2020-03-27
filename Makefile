CPP = g++ -std=c++14
INC = -I../cryphutil -I../fontutil -I../glslutil -I../ImageReader -I../mvcutil -I/usr/local/include -I. -I./src -I./src/inc

# >>> FOR LINUX, uncomment next few lines; comment out the MAC ones.
C_FLAGS = -fPIC -g -c $(INC) -DGL_GLEXT_PROTOTYPES
GL_LIB_LOC = -L/usr/lib/nvidia-375
GL_LIBRARIES = $(GL_LIB_LOC) -lglfw -lGLU -lGL
MAKE = make
# >>> FOR MAC, uncomment next few lines; comment out previous linux ones.
# C_FLAGS = -fPIC -g -c $(INC) -DGLFW_INCLUDE_GLEXT -DGLFW_INCLUDE_GLCOREARB -DGL_SILENCE_DEPRECATION
# GL_LIBRARIES = -L/usr/local/lib -lglfw -framework OpenGL
# MAKE = make -f MakefileMac
# >>> END: FOR LINUX - FOR MAC
VPATH = src:src/inc:../lib
LINK = g++ -fPIC -g
LOCAL_UTIL_LIBRARIES = ../lib/libcryph.so ../lib/libfont.so ../lib/libglsl.so ../lib/libImageReader.so ../lib/libmvc.so

OBJS = obj/main.o obj/ExtendedController.o obj/PhongMaterial.o obj/SceneElement.o obj/Tower.o obj/Facade.o obj/LightPole.o obj/Street.o obj/Ground.o obj/Sign.o obj/TextureInfo.o obj/TexManager.o obj/Skybox.o

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

obj/main.o: main.c++
	$(CPP) $(C_FLAGS) $< -o $@
obj/ExtendedController.o: ExtendedController.c++
	$(CPP) $(C_FLAGS) $< -o $@
obj/PhongMaterial.o: PhongMaterial.c++
	$(CPP) $(C_FLAGS) $< -o $@
obj/SceneElement.o: SceneElement.c++ SceneElement.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Tower.o: Tower.c++ Tower.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Facade.o: Facade.c++ Facade.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Street.o: Street.c++ Street.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Ground.o: Ground.c++ Ground.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Sign.o: Sign.c++ Sign.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/LightPole.o: LightPole.c++ LightPole.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/TextureInfo.o: TextureInfo.c++
	$(CPP) $(C_FLAGS) $< -o $@
obj/TexManager.o: TexManager.c++ TexManager.h
	$(CPP) $(C_FLAGS) $< -o $@
obj/Skybox.o: Skybox.c++
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