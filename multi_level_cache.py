from m5.objects import *
import m5
import sys

args = sys.argv[1:]
if "--" in args:
    args = args[args.index("--") + 1:]
if len(args) == 0:
    print("Usage: build/RISCV/gem5.opt multilevel_cache.py -- <binary> [args...]")
    exit(1)

binary = args[0]
binary_args = args[1:]
print(f"Binary: {binary}")
print(f"Arguments: {binary_args}")

system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '2GHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('16GB')]

system.cpu = TimingSimpleCPU()
system.l2bus = L2XBar()
system.l3bus = L2XBar()
system.membus = SystemXBar()

class L1Cache(Cache):
    assoc = 4
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 16
    tgts_per_mshr = 20

class L2Cache(Cache):
    assoc = 8
    tag_latency = 8
    data_latency = 8
    response_latency = 8
    mshrs = 32
    tgts_per_mshr = 20

class L3Cache(Cache):
    assoc = 16
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 64
    tgts_per_mshr = 20

system.cpu.icache = L1Cache(size='64kB')
system.cpu.dcache = L1Cache(size='64kB')
system.cpu.icache.cpu_side = system.cpu.icache_port
system.cpu.dcache.cpu_side = system.cpu.dcache_port
system.cpu.icache.mem_side = system.l2bus.cpu_side_ports
system.cpu.dcache.mem_side = system.l2bus.cpu_side_ports

system.l2cache = L2Cache(size='512kB')
system.l2cache.cpu_side = system.l2bus.mem_side_ports
system.l2cache.mem_side = system.l3bus.cpu_side_ports

system.l3cache = L3Cache(size='2MB')
system.l3cache.cpu_side = system.l3bus.mem_side_ports
system.l3cache.mem_side = system.membus.cpu_side_ports

system.cpu.createInterruptController()

system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR4_2400_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

process = Process()
process.cmd = [binary] + binary_args
system.workload = SEWorkload.init_compatible(binary)
system.cpu.workload = process
system.cpu.createThreads()

system.system_port = system.membus.cpu_side_ports

root = Root(full_system=False, system=system)
m5.instantiate()

print("Starting simulation (Multi-Level Cache)...")
exit_event = m5.simulate()
print(f"Exiting @ tick {m5.curTick()} because {exit_event.getCause()}")
