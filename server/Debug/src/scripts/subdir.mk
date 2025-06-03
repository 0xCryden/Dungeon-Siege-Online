################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/scripts/Player.cpp 

CPP_DEPS += \
./src/scripts/Player.d 

OBJS += \
./src/scripts/Player.o 


# Each subdirectory must supply rules for building sources it contributes
src/scripts/%.o: ../src/scripts/%.cpp src/scripts/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-scripts

clean-src-2f-scripts:
	-$(RM) ./src/scripts/Player.d ./src/scripts/Player.o

.PHONY: clean-src-2f-scripts

