; **************************************************************************************
; Pressure sensor hardware 
; **************************************************************************************
; Address of pressure sensor buffer in block ram
; Addr Range: Len : Description
; 0x70-0xFF : 32  : Address range for entire pressure sensor block
; 0x70      : 1   : Control/status register
; 0x71      : 1   : Data register (for indirect write)
; 0x72      : 1   : Index pointer (LSByte)
; 0x73      : 1   ; Index pointer (MSByte)
; 0x74      : 4   : latch timestamp register
CONSTANT PRESSURE_CTRL_REG, 70
; Pressure Control/status register bits
;  Bit 0 : TIMESTAMP_LATCH (write)
;          When written 1, will copy current 32bit timestamp value into timestamp buffer.
;          Timestamp buffer can be read by reading TIMESTAMP_REGX
;  Bit 1 : READY (write) 
;          When written 1 will will rotate pressure buffer, and 
;          also start transfered of data from pressure buffer to ET1100 
;          Write data is being transfered busy flag (bit2) will be set.
;          Flag will be cleaered when pressure data has been completely copied to ET1100.
;  Bit 2 : BUSY (read)
;          This bit will be set when 
;          treat this as busy flag, pressure buffer should not be rotated again when 
;          flag is set.
;  Bit 3 : ENABLE (read)
;          When set to 1, pressure sensor data should be read, 
;          When set to 0, pressure sensor read should be skipped.
;          This bit is controlled by computer software
	CONSTANT PRESSURE_TIMESTAMP_LATCH_FLAG, 1
	CONSTANT PRESSURE_READY_FLAG, 2
	CONSTANT PRESSURE_BUSY_FLAG, 4
	CONSTANT PRESSURE_ENABLE_FLAG, 8
; Data register that allows indirect write to pressure data buffer.
;  when data register is writen, the value is stored in pressure buffer at location 
;  based on index pointer.  After data is written, index pointer is incremented by 1
CONSTANT PRESSURE_DATA_REG, 71
; Index pointer (9bit).  Points to location where data is stored when next write to 
;  PRESSURE_DATA_REG is preformed.  Index pointer can be changed by writting new value 
;  to register.  Currently, index pointer cannot be read.
CONSTANT PRESSURE_INDEX_LOW_REG, 72  ; Lower 8bits of 9bit index pointer
CONSTANT PRESSURE_INDEX_HIGH_REG, 73 ; Upper 1 bit of 9bit index pointer
; 4-byte register where latched timestamp value is stored
CONSTANT PRESSURE_TIMESTAMP_REG_0, 74   ; LSByte of 32bit timestamp 
CONSTANT PRESSURE_TIMESTAMP_REG_1, 75
CONSTANT PRESSURE_TIMESTAMP_REG_2, 76
CONSTANT PRESSURE_TIMESTAMP_REG_3, 77   ; MSByte of 32bit timestamp



;**********************************************************************************
; This program writes "Hello WG006!" into pressure buffer using ASCII characters
;  An ASCII table can be found at http://www.asciitable.com/
main_loop:
	
	; reset pressure buffer index pointer to back to 0
		load s0, 0
		output s0, PRESSURE_INDEX_LOW_REG
		output s0, PRESSURE_INDEX_HIGH_REG	

	; When writing PRESSURE_DATA_REG, the value is written to the pressure 
	; buffer based on value of PRESSURE_INDEX register.
	; After data is written, the PRESSURE_INDEX value is incremented by 1

	; Hello 
		load s0, 48 ; 0x48 = 'H'
		output s0, PRESSURE_DATA_REG
		load s0, 65 ; 0x65 = 'e'
		output s0, PRESSURE_DATA_REG
		load s0, 6C ; 0x6C = 'l'
		output s0, PRESSURE_DATA_REG
		output s0, PRESSURE_DATA_REG
		load s0, 6F ; 0x6F = 'o'
		output s0, PRESSURE_DATA_REG

	; Space
		load s0, 20 ; 0x20 = ' '
		output s0, PRESSURE_DATA_REG

	; WG006!
		load s0, 57
		output s0, PRESSURE_DATA_REG
		load s0, 47
		output s0, PRESSURE_DATA_REG
		load s0, 30
		output s0, PRESSURE_DATA_REG
		output s0, PRESSURE_DATA_REG
		load s0, 36
		output s0, PRESSURE_DATA_REG
		load s0, 21
		output s0, PRESSURE_DATA_REG

	; Wait for pressure buffer to flip...
	wait_loop:
		load s0, PRESSURE_CTRL_REG	
		test s0, PRESSURE_BUSY_FLAG
		jump NZ, wait_loop

	; Flag pressure data as being ready
		load s0, PRESSURE_READY_FLAG
		output s0, PRESSURE_CTRL_REG

	; Repeat
	jump main_loop

