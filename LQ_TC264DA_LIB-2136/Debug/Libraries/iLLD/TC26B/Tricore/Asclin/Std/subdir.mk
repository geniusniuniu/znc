################################################################################
# �Զ����ɵ��ļ�����Ҫ�༭��
################################################################################

# ����Щ���ߵ��õ�����������ӵ��������� 
C_SRCS += \
../Libraries/iLLD/TC26B/Tricore/Asclin/Std/IfxAsclin.c 

OBJS += \
./Libraries/iLLD/TC26B/Tricore/Asclin/Std/IfxAsclin.o 

COMPILED_SRCS += \
./Libraries/iLLD/TC26B/Tricore/Asclin/Std/IfxAsclin.src 

C_DEPS += \
./Libraries/iLLD/TC26B/Tricore/Asclin/Std/IfxAsclin.d 


# ÿ����Ŀ¼����Ϊ�����������׵�Դ�ṩ����
Libraries/iLLD/TC26B/Tricore/Asclin/Std/%.src: ../Libraries/iLLD/TC26B/Tricore/Asclin/Std/%.c Libraries/iLLD/TC26B/Tricore/Asclin/Std/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING C/C++ Compiler'
	cctc -D__CPU__=tc26xb "-fC:/Users/gyy123/Desktop/znc_back/znc/LQ_TC264DA_LIB-2136/Debug/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc26xb -o "$@"  "$<"  -cs --dep-file="$(@:.src=.d)" --misrac-version=2012 -N0 -Z0 -Y0 2>&1;
	@echo '�ѽ��������� $<'
	@echo ' '

Libraries/iLLD/TC26B/Tricore/Asclin/Std/%.o: ./Libraries/iLLD/TC26B/Tricore/Asclin/Std/%.src Libraries/iLLD/TC26B/Tricore/Asclin/Std/subdir.mk
	@echo '���ڹ����ļ��� $<'
	@echo '���ڵ��ã� TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<" --list-format=L1 --optimize=gs
	@echo '�ѽ��������� $<'
	@echo ' '


