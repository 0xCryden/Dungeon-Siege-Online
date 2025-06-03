################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/net/Connection.cpp \
../src/net/ListeningSocket.cpp \
../src/net/Network.cpp \
../src/net/Packet.cpp 

CPP_DEPS += \
./src/net/Connection.d \
./src/net/ListeningSocket.d \
./src/net/Network.d \
./src/net/Packet.d 

OBJS += \
./src/net/Connection.o \
./src/net/ListeningSocket.o \
./src/net/Network.o \
./src/net/Packet.o 


# Each subdirectory must supply rules for building sources it contributes
src/net/%.o: ../src/net/%.cpp src/net/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	C:/msys64/mingw64/bin/g++.exe -I"C:/msys64/mingw64/include/libxml2" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-net

clean-src-2f-net:
	-$(RM) ./src/net/Connection.d ./src/net/Connection.o ./src/net/ListeningSocket.d ./src/net/ListeningSocket.o ./src/net/Network.d ./src/net/Network.o ./src/net/Packet.d ./src/net/Packet.o

.PHONY: clean-src-2f-net

