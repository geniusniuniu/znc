################################################################################
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# ����Щ���ߵ��õ�����������ӵ��������� 
C_SRCS += \
../Libraries/Infra/Platform/Tricore/Compilers/CompilerDcc.c \
../Libraries/Infra/Platform/Tricore/Compilers/CompilerGhs.c \
../Libraries/Infra/Platform/Tricore/Compilers/CompilerGnuc.c \
../Libraries/Infra/Platform/Tricore/Compilers/CompilerTasking.c 

OBJS += \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerDcc.o \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGhs.o \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGnuc.o \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerTasking.o 

COMPILED_SRCS += \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerDcc.src \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGhs.src \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGnuc.src \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerTasking.src 

C_DEPS += \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerDcc.d \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGhs.d \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerGnuc.d \
./Libraries/Infra/Platform/Tricore/Compilers/CompilerTasking.d 


# ÿ����Ŀ¼����Ϊ�����������׵�Դ�ṩ����
Libraries/Infra/Platform/Tricore/Compilers/%.src: ../Libraries/Infra/Platform/Tricore/Compilers/%.c Libraries/Infra/Platform/Tricore/Compilers/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fC:/Users/gyy123/Desktop/znc_back/znc/LQ_TC264DA_LIB-2136/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '�ѽ��������� $<'
	@echo ' '

Libraries/Infra/Platform/Tricore/Compilers/%.o: ./Libraries/Infra/Platform/Tricore/Compilers/%.src Libraries/Infra/Platform/Tricore/Compilers/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '�ѽ��������� $<'
	@echo ' '


