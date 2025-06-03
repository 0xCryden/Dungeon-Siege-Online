################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/server/Account.cpp \
../src/server/Server.cpp 

CPP_DEPS += \
./src/server/Account.d \
./src/server/Server.d 

OBJS += \
./src/server/Account.o \
./src/server/Server.o 


# Each subdirectory must supply rules for building sources it contributes
src/server/%.o: ../src/server/%.cpp src/server/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-server

clean-src-2f-server:
	-$(RM) ./src/server/Account.d ./src/server/Account.o ./src/server/Server.d ./src/server/Server.o

.PHONY: clean-src-2f-server

