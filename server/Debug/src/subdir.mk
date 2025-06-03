################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AIQuery.cpp \
../src/CurrentTime.cpp \
../src/Engine.cpp \
../src/Gas.cpp \
../src/Log.cpp \
../src/main.cpp 

CPP_DEPS += \
./src/AIQuery.d \
./src/CurrentTime.d \
./src/Engine.d \
./src/Gas.d \
./src/Log.d \
./src/main.d 

OBJS += \
./src/AIQuery.o \
./src/CurrentTime.o \
./src/Engine.o \
./src/Gas.o \
./src/Log.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/AIQuery.d ./src/AIQuery.o ./src/CurrentTime.d ./src/CurrentTime.o ./src/Engine.d ./src/Engine.o ./src/Gas.d ./src/Gas.o ./src/Log.d ./src/Log.o ./src/main.d ./src/main.o

.PHONY: clean-src

