Import("env")

board_config = env.BoardConfig()
# should be array of VID:PID pairs
board_config.update("build.hwids", [
    ["0x2341", "0x2341", "0x2A03", "0x2A03"], #VIDS
    ["0x0036", "0x8036", "0x0036", "0x8036"] #PIDS
])
board_config.update("build.usb_product", "Trim Wheel Pro")