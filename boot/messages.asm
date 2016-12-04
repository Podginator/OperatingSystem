; Various messages output by the second stage

second_stage_msg	db 'Second stage loaded', 0	
protected_mode_msg	db 'In 32-bit protected mode', 0Dh, 0Ah, 0
a20_msg_one			db 'A20 line already enabled', 0
a20_msg_two			db 'A20 line enabled by BIOS', 0
a20_msg_three		db 'A20 line enabled using keyboard controller', 0
a20_msg_four		db 'A20 line enabled using Fast A20 gate', 0
no_a20_msg			db 'Unable to enable A20 line.', 0	
loadingMsg 	  		db 'Searching for KERNEL.SYS...', 0
msgFailure 	  		db 'Unable to find KERNEL.SYS. ', 0
msgWaitForKey		db 'Press Enter key to continue', 0

a20_message_list	dw no_a20_msg
					dw a20_msg_one
					dw a20_msg_two
					dw a20_msg_three
					dw a20_msg_four