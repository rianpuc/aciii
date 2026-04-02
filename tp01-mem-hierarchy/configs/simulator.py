import m5
from m5.objects import *
import argparse

# argumentos do terminal
parser = argparse.ArgumentParser(description='Simulador de Cache - Projeto de Arquitetura')
parser.add_argument('--binary', type=str, required=True, help='Caminho para o binario')
parser.add_argument('--args', type=str, default='', help='Argumentos para o binario')
parser.add_argument('--l1d_size', type=str, default='32kB', help='Tamanho do L1 Cache')
parser.add_argument('--l1_assoc', type=int, default=2, help='Associatividade do L1')
parser.add_argument('--block_size', type=int, default=64, help='Tamanho da linha de cache (em bytes)')
options = parser.parse_args()

# definindo as classes de cache
class L1Cache(Cache):
    assoc = 2
    tag_latency = 2
    data_latency = 2
    response_latency = 2
    mshrs = 4
    tgts_per_mshr = 20

class L2Cache(Cache):
    size = '256kB'
    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12

# criando o sistema base
system = System()
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '1GHz'
system.clk_domain.voltage_domain = VoltageDomain()
system.mem_mode = 'timing'
system.mem_ranges = [AddrRange('512MB')]
system.cache_line_size = options.block_size

# criando a cpu x86
system.cpu = X86TimingSimpleCPU()

# instanciando cache l1
system.cpu.icache = L1Cache(size='32kB', assoc=2)
system.cpu.dcache = L1Cache(size=options.l1d_size, assoc=options.l1_assoc)

system.cpu.icache.cpu_side = system.cpu.icache_port
system.cpu.dcache.cpu_side = system.cpu.dcache_port

system.l2bus = L2XBar()
system.cpu.icache.mem_side = system.l2bus.cpu_side_ports
system.cpu.dcache.mem_side = system.l2bus.cpu_side_ports

# conectando cache l2
system.l2cache = L2Cache()
system.l2cache.cpu_side = system.l2bus.mem_side_ports
system.membus = SystemXBar()
system.l2cache.mem_side = system.membus.cpu_side_ports

# config interrupcoes pra x86
system.cpu.createInterruptController()
system.cpu.interrupts[0].pio = system.membus.mem_side_ports
system.cpu.interrupts[0].int_responder = system.membus.mem_side_ports
system.cpu.interrupts[0].int_requestor = system.membus.cpu_side_ports

system.system_port = system.membus.cpu_side_ports

# configurando controlador de memoria
system.mem_ctrl = MemCtrl()
system.mem_ctrl.dram = DDR3_1600_8x8()
system.mem_ctrl.dram.range = system.mem_ranges[0]
system.mem_ctrl.port = system.membus.mem_side_ports

# carregando o binario c
process = Process()
process.executable = options.binary
process.cmd = [options.binary] + options.args.split()
system.workload = SEWorkload.init_compatible(options.binary)
system.cpu.workload = process
system.cpu.createThreads()

# rodando a simulacao
root = Root(full_system=False, system=system)
m5.instantiate()

print(f"Iniciando simulacao com L1D={options.l1d_size}, Assoc={options.l1_assoc}, Linha={options.block_size}B")
exit_event = m5.simulate()
print(f"Fim da simulacao no tick {m5.curTick()} (Motivo: {exit_event.getCause()})")
