def list_hex(list):
    strs = []
    for i in list:
        strs.append(f"0x{i:0>2X}")
    return ", ".join(strs)

chunk_len_sum = 0

def chunk(type, data):
    global chunk_len_sum
    chunk_len_sum += 2 + len(data)
    return f"0x{(len(data) + 1):0>2X}, 0x{(type):0>2X}, {list_hex(data)}"

LELimitedDiscovery = 1 << 0
LEGeneralDiscoverable = 1 << 1
BR_EDRNotSupported = 1 << 2
LE_And_BR_EDR_Capable_Controller = 1 << 3
LE_And_BR_EDR_Capable_Controller = 1 << 3
Previously_Used = 1 << 4

CabinetLight = [0x92, 0x05]

adv_data = [
    chunk(0x01, [LEGeneralDiscoverable | BR_EDRNotSupported]), # Flags
    chunk(0x19, CabinetLight),
    # chunk(0x0a, [0xeb]), # TX Power Level
    # chunk(0x03, [0xab, 0xcd]), # Complete List of 16-bit Service Class
    chunk(0x09, "LED Showcase Light".encode()) # Complete Local Name
]

array_name = "ext_adv_raw_data"

print("constexpr std::array<uint8_t, %d> %s = {\n%2s //\n};" % (chunk_len_sum, array_name, ", //\n".join(["    " + d for d in adv_data])))
