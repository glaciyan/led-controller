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

adv_data = [
    chunk(0x01, [0x06]),
    chunk(0x0a, [0xeb]),
    chunk(0x03, [0xab, 0xcd]),
    chunk(0x09, "ESP_BLE50_SERVER".encode())
]

array_name = "ext_adv_raw_data"

print("std::array<uint8_t, %d> %s = {\n%2s //\n};" % (chunk_len_sum, array_name, ", //\n".join(["    " + d for d in adv_data])))
