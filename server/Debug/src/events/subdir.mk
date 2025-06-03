################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/events/SendWorldMessageEvent.cpp 

CPP_DEPS += \
./src/events/SendWorldMessageEvent.d 

OBJS += \
./src/events/SendWorldMessageEvent.o 


# Each subdirectory must supply rules for building sources it contributes
src/events/%.o: ../src/events/%.cpp src/events/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-events

clean-src-2f-events:
	-$(RM) ./src/events/SendWorldMessageEvent.d ./src/events/SendWorldMessageEvent.o

.PHONY: clean-src-2f-events

