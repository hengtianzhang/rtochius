
kernel_library()

kernel_library_sources(serial.c)

kernel_library_sources_ifdef(CONFIG_SERIAL_AMBA_PL011
	amba-pl011.c
)
