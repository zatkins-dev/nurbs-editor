CPP = g++ -std=c++17
INC = -I../cryphutil -I../ImageReader -I../fontutil -I../glslutil -I../mvcutil -I./ext/imgui
ROOT_PATH = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
IMGUI_LOADER = include/imgui_loader_glext.h
IMGUI_CONFIG = include/imconfig.h

# >>> FOR LINUX, uncomment next few lines; comment out the MAC ones.
C_FLAGS = -fPIC -g -c $(INC) -DGL_GLEXT_PROTOTYPES
C_FLAGS += -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=\"$(ROOT_PATH)$(IMGUI_LOADER)\"
C_FLAGS += -DIMGUI_USER_CONFIG=\"$(ROOT_PATH)$(IMGUI_CONFIG)\"
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
VPATH = src:src/interactive:../lib:lib:ext:ext/imgui:ext/imgui/examples
LINK = g++ -fPIC -g -Wall
LOCAL_UTIL_LIBRARIES = ../lib/libcryph.so ../lib/libfont.so ../lib/libglsl.so ../lib/libImageReader.so ../lib/libmvc.so

OBJS = obj/main.o obj/SceneElement.o obj/ImGUIMenu.o obj/ShaderIFManager.o
OBJS += obj/Interactive.o obj/InteractiveCurve.o obj/InteractivePoint.o 
OBJS += ext/imgui/build/obj/imgui_impl_glfw.o ext/imgui/build/obj/imgui_impl_opengl3.o
OBJS += ext/imgui/build/obj/imgui.o ext/imgui/build/obj/imgui_demo.o ext/imgui/build/obj/imgui_draw.o ext/imgui/build/obj/imgui_widgets.o
main: ensure-dirs $(OBJS) $(LOCAL_UTIL_LIBRARIES)
	$(LINK) -o $@ $(OBJS) $(LOCAL_UTIL_LIBRARIES) $(GL_LIBRARIES) -Wl,-rpath ./lib

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

obj/%.o : %.cpp 
	$(CPP) $(C_FLAGS) -o $@ -c $<

ext/imgui/build/obj/%.o: %.cpp
	$(CPP) $(C_FLAGS) -o $@ -c $<

.PHONY : clean
clean: 
	rm -rf obj bin main

.PHONY : clean-imgui
clean-imgui: 
	rm -rf ext/imgui/build/obj/*

.PHONY : clean-all
clean-all: clean clean-imgui
	rm -rf  ../cryphutil/*.o ../cryphutil/*.so \
		../fontutil/*.o ../fontutil/*.so \
		../glslutil/*.o ../glslutil/*.so \
		../ImageReader/*.o ../ImageReader/*.so \
		../mvcutil/*.o ../mvcutil/*.so \
		../lib/* 
	(cd ./ext/nanogui/build/$(BUILD); make clean)

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