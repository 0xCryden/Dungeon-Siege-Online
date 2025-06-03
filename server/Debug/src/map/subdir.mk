################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/map/Node.cpp \
../src/map/Region.cpp \
../src/map/WorldMap.cpp 

CPP_DEPS += \
./src/map/Node.d \
./src/map/Region.d \
./src/map/WorldMap.d 

OBJS += \
./src/map/Node.o \
./src/map/Region.o \
./src/map/WorldMap.o 


# Each subdirectory must supply rules for building sources it contributes
src/map/%.o: ../src/map/%.cpp src/map/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-map

clean-src-2f-map:
	-$(RM) ./src/map/Node.d ./src/map/Node.o ./src/map/Region.d ./src/map/Region.o ./src/map/WorldMap.d ./src/map/WorldMap.o

.PHONY: clean-src-2f-map

