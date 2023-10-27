################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../src/User/LQ_AnoScope.c \
../src/User/LQ_ImageProcess.c \
../src/User/LQ_Inductor.c \
../src/User/LQ_MotorServo.c \
../src/User/LQ_PID.c 

OBJS += \
./src/User/LQ_AnoScope.o \
./src/User/LQ_ImageProcess.o \
./src/User/LQ_Inductor.o \
./src/User/LQ_MotorServo.o \
./src/User/LQ_PID.o 

COMPILED_SRCS += \
./src/User/LQ_AnoScope.src \
./src/User/LQ_ImageProcess.src \
./src/User/LQ_Inductor.src \
./src/User/LQ_MotorServo.src \
./src/User/LQ_PID.src 

C_DEPS += \
./src/User/LQ_AnoScope.d \
./src/User/LQ_ImageProcess.d \
./src/User/LQ_Inductor.d \
./src/User/LQ_MotorServo.d \
./src/User/LQ_PID.d 


# 每个子目录必须为构建它所贡献的源提供规则
src/User/%.src: ../src/User/%.c src/User/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fC:/Users/gyy123/Desktop/znc/LQ_TC264DA_LIB-2136/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

src/User/%.o: ./src/User/%.src src/User/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


