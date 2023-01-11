# Utility script that converts binary to a C++ hex array and prints to stdout.
# Each line of binary corresponds to an element in the output array.

with open("bits", "r") as f:
    nums = []
    for line in f.readlines():
        num = int(line, 2)
        hex_num = hex(num)
        nums.append(hex_num)
    print("{" + ",".join(nums) + "}")