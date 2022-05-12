add_executable(conveyer
    main.c
)

target_link_libraries(conveyer
    pico_stdlib
    freertos
)

pico_enable_stdio_usb(conveyer 1)
pico_enable_stdio_uart(conveyer 0)

pico_add_extra_outputs(conveyer)