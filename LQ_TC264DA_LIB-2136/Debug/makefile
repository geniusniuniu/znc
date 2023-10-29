################################################################################
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# 此处定义了参与生成的所有源
-include sources.mk
-include src/User/subdir.mk
-include src/MyCode/subdir.mk
-include src/Driver/subdir.mk
-include src/APP/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/_PinMap/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/_Lib/DataHandling/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/_Impl/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Vadc/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Vadc/Adc/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Stm/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Src/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Scu/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Qspi/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Qspi/SpiMaster/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Port/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Mtu/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Gtm/Tom/Pwm/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Gtm/Tim/In/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Gtm/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Gtm/Atom/Pwm/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Gpt12/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Flash/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Fft/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Fft/Fft/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Emem/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Dma/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Dma/Dma/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Cpu/Trap/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Cpu/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Cpu/Irq/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Cpu/CStart/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Ccu6/Timer/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Ccu6/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Asclin/Std/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Asclin/Spi/subdir.mk
-include Libraries/iLLD/TC26B/Tricore/Asclin/Asc/subdir.mk
-include Libraries/Service/CpuGeneric/SysSe/Math/subdir.mk
-include Libraries/Service/CpuGeneric/SysSe/Bsp/subdir.mk
-include Libraries/Service/CpuGeneric/StdIf/subdir.mk
-include Libraries/Service/CpuGeneric/If/subdir.mk
-include Libraries/Infra/Platform/Tricore/Compilers/subdir.mk
-include Configurations/Debug/subdir.mk
-include .metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := LQ_TC264DA_LIB-2136
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# 将这些工具调用的输入和输出添加到构建变量 
SECONDARY_SIZE += \
LQ_TC264DA_LIB-2136.siz \


# 所有目标
all: main-build

# 主构建目标
main-build: LQ_TC264DA_LIB-2136.elf secondary-outputs

# 工具调用
LQ_TC264DA_LIB-2136.elf: $(OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo '正在构建目标： $@'
	@echo '正在调用： TASKING Linker'
	cctc -lrt -lfp_fpu -lcs_fpu  -Wl-Oc -Wl-OL -Wl-Ot -Wl-Ox -Wl-Oy "C:\Users\gyy123\Desktop\znc\LQ_TC264DA_LIB-2136\src\APP\LQ_MPU6050_DMP.o" -Wl--map-file="LQ_TC264DA_LIB-2136.map" -Wl-mc -Wl-mf -Wl-mi -Wl-mk -Wl-ml -Wl-mm -Wl-md -Wl-mr -Wl-mu --no-warnings= -Wl--error-limit=42 --exceptions --strict --anachronisms --force-c++ -Ctc26xb   -o"LQ_TC264DA_LIB-2136.elf" -Wl-o"LQ_TC264DA_LIB-2136.hex:IHEX" --lsl-core=vtc  --lsl-file=../Lcf_Tasking_Tricore_Tc.lsl $(OBJS)
	@echo '已结束构建目标： $@'
	@echo ' '

LQ_TC264DA_LIB-2136.siz: LQ_TC264DA_LIB-2136.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo '正在调用： Print Size'
	elfsize  "LQ_TC264DA_LIB-2136.elf"
	@echo '已结束构建： $@'
	@echo ' '

# 其他目标
clean:
	find . ! -name "*.mk" ! -name "makefile" ! -name "." -type f -delete 
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build

-include ../makefile.targets
