################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
device.obj: ../device.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="device.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

n64usbhid.obj: ../n64usbhid.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="n64usbhid.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

protocol.obj: ../protocol.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="protocol.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

tm4c123gh6pm_startup_ccs.obj: ../tm4c123gh6pm_startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="tm4c123gh6pm_startup_ccs.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

ustdlib.obj: ../ustdlib.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="/opt/tiva/ccs/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="/opt/tiva/tivaware/usblib" --include_path="/opt/tiva/tivaware/usblib/device" --include_path="/opt/tiva/tivaware/driverlib" --include_path="/opt/tiva/tivaware/inc" -g --gcc --define=ccs --define=USE_ROMLIB --diag_warning=225 --display_error_number --diag_wrap=off --preproc_with_compile --preproc_dependency="ustdlib.pp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


