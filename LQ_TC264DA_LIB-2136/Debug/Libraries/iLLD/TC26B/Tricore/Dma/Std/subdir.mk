################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Dma/Std/IfxDma.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Dma/Std/IfxDma.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Dma/Std/IfxDma.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Dma/Std/IfxDma.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/iLLD/TC26B/Tricore/Dma/Std/%.src: ../Libraries/iLLD/TC26B/Tricore/Dma/Std/%.c Libraries/iLLD/TC26B/Tricore/Dma/Std/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fC:/Users/gyy123/Desktop/znc/LQ_TC264DA_LIB-2136/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo 'Finished building: $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Dma/Std/%.o: ./Libraries/iLLD/TC26B/Tricore/Dma/Std/%.src Libraries/iLLD/TC26B/Tricore/Dma/Std/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo 'Finished building: $<'
	@echo ' '


