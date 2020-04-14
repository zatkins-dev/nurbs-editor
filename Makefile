CPP = g++ -std=c++17
INC = -I../cryphutil -I../ImageReader -I../fontutil -I../glslutil -I../mvcutil -I./ext/imgui
ROOT_PATH = $(dir $(realpath $(firstword $(MAKEFILE_LIST))))
IMGUI_LOADER = include/imgui_loader_glext.h
IMGUI_CONFIG = include/imconfig.h

# >>> FOR LINUX, uncomment next few lines; comment out the MAC ones.
C_FLAGS = -fPIC -g -O -c $(INC) -DGL_GLEXT_PROTOTYPES
IMGUI_C_FLAGS = -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=\"$(ROOT_PATH)$(IMGUI_LOADER)\" -DIMGUI_USER_CONFIG=\"$(ROOT_PATH)$(IMGUI_CONFIG)\"
GL_LIB_LOC = -L/usr/lib/nvidia-375 
GL_LIBRARIES = $(GL_LIB_LOC) -lglfw -lGLU -lGL 
MAKE = make
BUILD = fedora
# >>> FOR MAC, uncomment next few lines; comment out previous linux ones.
# C_FLAGS = -fPIC -g -c $(INC) -DGLFW_INCLUDE_GLEXT -DGLFW_INCLUDE_GLCOREARB -DGL_SILENCE_DEPRECATION
# IMGUI_C_FLAGS = -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=\"$(ROOT_PATH)$(IMGUI_LOADER)\" -DIMGUI_USER_CONFIG=\"$(ROOT_PATH)$(IMGUI_CONFIG)\"
# GL_LIBRARIES = -L/usr/local/lib -lglfw -framework OpenGL
# MAKE = make -f MakefileMac
# BUILD = mac
# >>> END: FOR LINUX - FOR MAC
VPATH = src:src/interactive:../lib:lib:ext/imgui:ext/imgui/examples:ext/imgui/misc/cpp
LINK = g++ -fPIC -g -Wall
LOCAL_UTIL_LIBRARIES = ../lib/libcryph.so ../lib/libfont.so ../lib/libglsl.so ../lib/libImageReader.so ../lib/libmvc.so lib/libimgui.so

OBJS = obj/main.o obj/SceneElement.o obj/ImGUIMenu.o obj/ShaderIFManager.o obj/InteractiveController.o
OBJS += obj/Interactive.o obj/InteractiveSurface.o obj/InteractiveCurve.o obj/InteractivePoint.o 
IMGUI_OBJS = obj/ext/imgui_impl_glfw.o obj/ext/imgui_impl_opengl3.o
IMGUI_OBJS += obj/ext/imgui.o obj/ext/imgui_demo.o obj/ext/imgui_draw.o obj/ext/imgui_widgets.o obj/ext/imgui_stdlib.o

main: ensure-dirs $(OBJS) $(LOCAL_UTIL_LIBRARIES)
	@echo Linking $@
	@$(LINK) -o $@ $(OBJS) $(LOCAL_UTIL_LIBRARIES) $(GL_LIBRARIES) -Wl,-rpath ./lib

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

lib/libimgui.so : $(IMGUI_OBJS)
	@echo Linking $(notdir $@)
	@$(LINK) -shared -o libimgui.so $(IMGUI_OBJS)
	@mv libimgui.so lib/

obj/main.o : main.cpp
	@echo Compiling $(notdir $@)
	@$(CPP) $(C_FLAGS) $(IMGUI_C_FLAGS) -o $@ -c $<

obj/ImGUIMenu.o : ImGUIMenu.cpp ImGUIMenu.h
	@echo Compiling $(notdir $@)
	@$(CPP) $(C_FLAGS) $(IMGUI_C_FLAGS) -o $@ -c $<

obj/%.o : %.cpp %.h
	@echo Compiling $(notdir $@)
	@$(CPP) $(C_FLAGS) -o $@ -c $<

obj/ext/%.o : %.cpp %.h
	@echo Compiling Imgui::$(notdir $@)
	@$(CPP) $(C_FLAGS) $(IMGUI_C_FLAGS) -o $@ -c $<

obj/ext/%.o : %.cpp
	@echo Compiling Imgui::$(notdir $@)
	@$(CPP) $(C_FLAGS) $(IMGUI_C_FLAGS) -o $@ -c $<

.PHONY : clean
clean: 
	rm -f obj/*.o obj/ext/*.o bin/* lib/* main

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

.PHONY .SILENT: ensure-dirs
ensure-dirs:
	mkdir -p ./obj
	mkdir -p ../lib

.PHONY : dist
dist: tar
	echo "Making tarball"

.PHONY : tar
tar: clean
	tar -czvf ../$(shell basename '$(CURDIR)'.tar.gz) ../$(shell basename '$(CURDIR)')