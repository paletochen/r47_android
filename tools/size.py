import sys

def usage():
    print("Usage: %s [dmcp5] elf-symbol-filename" % sys.argv[0])
    sys.exit(1)

# Check command line
n = len(sys.argv)
if n == 2:
    dmcp5 = False
elif n == 3:
    if sys.argv[1] != 'dmcp5':
        usage()
    dmcp5 = True
else:
    usage()
infile = sys.argv[-1]

# Initialise
mem = {
    "flash" : 1024 * (1408 if dmcp5 else 704),
    "ram"   : 1024 * (16 if dmcp5 else 16),
    "qspi"  : 1024 * (2048 if dmcp5 else 2048 - 12)
}
mode = 0
sects = [ 0, 0, 0, 0, 0 ]
sizes = []

# Read elf sections file
f = open(infile, "r")
for line in f:
    if line == "\n":
        mode = 0
        continue
    if line == "Program Headers:\n":
        mode = 1
        continue
    if line == "  Segment Sections...\n":
        mode = 2
        continue
    f = line.split()
    if mode == 1:
        # Line is formatted as: "Type Offset VirtAddr PhysAddr FileSiz MemSiz Flg Align"
        # and we want the MemSiz.
        if f[0] != 'Type':
            sizes.append(int(f[5], 0))
    elif mode == 2:
        # Line is formatted as: "Segment# Sections" and we want the first section.
        sects[int(f[0])] = f[1]

# Compute section totals
used = {
    "flash" : 0,
    "ram"   : 0,
    "qspi"  : 0
}
for i in range(len(sizes)):
    if sects[i] == ".rodata" or sects[i] == ".text":
        used["flash"] += sizes[i]
    elif sects[i] == ".data":
        # The initialisation for the data is in flash
        # The data itself is in RAM
        used["flash"] += sizes[i]
        used["ram"] += sizes[i]
    elif sects[i] == ".bss":
        used["ram"] += sizes[i]
    elif sects[i] == ".qspi":
        used["qspi"] += sizes[i]
    else:
        print("Warning: Unknown section:", sects[i], "- ignoring.")
        continue

# Display totals and free space
def output(sect):
    print("%-5s  %8d  %8d  %8d" % (sect, used[sect], mem[sect], mem[sect] - used[sect]))

print("\n%-7s%8s  %8s  %8s" % ("section", "used", "total", "left"))
output("flash")
output("ram")
output("qspi")
