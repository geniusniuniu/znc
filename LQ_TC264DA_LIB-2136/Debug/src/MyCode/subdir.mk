################################################################################
# 自动生成的文件。不要编辑！
################################################################################

# 将这些工具调用的输入和输出添加到构建变量 
C_SRCS += \
../src/MyCode/BlueTooth.c \
../src/MyCode/MyEncoder.c \
../src/MyCode/MyKalmanfilter.c \
../src/MyCode/MyMPU6050.c \
../src/MyCode/MyPWM.c \
../src/MyCode/My_Camera.c \
../src/MyCode/My_Timer.c \
../src/MyCode/PID.c 

OBJS += \
./src/MyCode/BlueTooth.o \
./src/MyCode/MyEncoder.o \
./src/MyCode/MyKalmanfilter.o \
./src/MyCode/MyMPU6050.o \
./src/MyCode/MyPWM.o \
./src/MyCode/My_Camera.o \
./src/MyCode/My_Timer.o \
./src/MyCode/PID.o 

COMPILED_SRCS += \
./src/MyCode/BlueTooth.src \
./src/MyCode/MyEncoder.src \
./src/MyCode/MyKalmanfilter.src \
./src/MyCode/MyMPU6050.src \
./src/MyCode/MyPWM.src \
./src/MyCode/My_Camera.src \
./src/MyCode/My_Timer.src \
./src/MyCode/PID.src 

C_DEPS += \
./src/MyCode/BlueTooth.d \
./src/MyCode/MyEncoder.d \
./src/MyCode/MyKalmanfilter.d \
./src/MyCode/MyMPU6050.d \
./src/MyCode/MyPWM.d \
./src/MyCode/My_Camera.d \
./src/MyCode/My_Timer.d \
./src/MyCode/PID.d 


# 每个子目录必须为构建它所贡献的源提供规则
src/MyCode/%.src: ../src/MyCode/%.c src/MyCode/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fC:/Users/gyy123/Desktop/znc_back/znc/LQ_TC264DA_LIB-2136/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '已结束构建： $<'
	@echo ' '

src/MyCode/%.o: ./src/MyCode/%.src src/MyCode/subdir.mk
	@echo '正在构建文件： $<'
	@echo '正在调用： TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '已结束构建： $<'
	@echo ' '


