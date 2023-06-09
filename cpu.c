#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#define MEMORY_SIZE 16

#define INSTRUCTION_SIZE 8
#define ADDRESS_SIZE 8

#define FIRMWARE_SIZE 256

typedef struct cpu_state {
  int8_t reg;
  int8_t *memory;
  uint8_t *firmware;
  uint8_t offset;
  bool halted;
} cpu_state;

cpu_state *create_cpu(int8_t *memory, uint8_t *firmware) {
  cpu_state* cpu = malloc(sizeof(cpu_state));
  cpu->memory = memory;
  cpu->firmware = firmware;
  cpu->offset = 0;
  return cpu;
};

typedef enum instruction {
  AND = 0x1,
  OR  = 0x2,
  XOR = 0x3,
  NOT = 0x4,
  LDA = 0x5,
  STA = 0x6,
  HLT = 0x7,
  TST = 0x8,
  JPZ = 0x9,
  ADD = 0xA,
  OUT = 0xB,
  JMP = 0xC
} instruction;

void cycle(cpu_state *cpu) {
  if (cpu->offset + 2 >= FIRMWARE_SIZE) {
    cpu->halted = true;
    return;
  }

  int8_t instruction = cpu->firmware[cpu->offset];
  cpu->offset++;

  uint8_t address = cpu->firmware[cpu->offset];
  cpu->offset++;

  if (instruction == NOT) {
    cpu->reg = ~cpu->reg;
  } 

  if (instruction == HLT) {
    printf("HLT at %x\n", cpu->offset);
    cpu->halted = true;
  } 

  if (instruction == ADD) {
    cpu->reg += cpu->memory[address];
  }

  if (instruction == JPZ && cpu->reg == 0) {
    cpu->offset = address;
  }

  if (instruction == JMP) {
    cpu->offset = address;
  }

  if (instruction == TST) {
    if (cpu->reg == cpu->memory[address]) {
      cpu->reg = 0;
    }
  }

  if (instruction == OR) {
    cpu->reg |= cpu->memory[address];
  }

  if (instruction == XOR) {
    cpu->reg ^= cpu->memory[address];
  }

  if (instruction == AND) {
    cpu->reg &= cpu->memory[address];
  }

  if (instruction == LDA) {
    cpu->reg = cpu->memory[address];
  }

  if (instruction == OUT) {
    printf("%d\n", cpu->reg);
  }

  if (instruction == STA) {
    cpu->memory[address] = cpu->reg;
  }
}

void print_firmware(uint8_t *firmware) {
  puts("Firmware:");

  for (int i = 0; i < FIRMWARE_SIZE; i++) {
    printf("%x = %x\n", i, firmware[i]);
  }

  puts("");
}

void print_memory(int8_t *memory) {
  for (int i = 0; i < MEMORY_SIZE; i++) {
    printf("%x = %x\n", i, memory[i]);
  }
}

int main(int argc, char **argv) {
  bool dump_memory = false;
  int opt;
  char *file_name = NULL;

  while ((opt = getopt(argc, argv, "df")) != -1) {
      switch (opt) {
      case 'd': dump_memory = true; break;
      default:
          fprintf(stderr, "Usage: %s [-d] <file>\n", argv[0]);
          exit(EXIT_FAILURE);
      }
  }

  FILE *f = fopen(argv[optind], "r");
  if (f == NULL) {
    printf("Could not open file: %s\n", argv[optind]);
    return 1;
  }

  int8_t *memory = malloc(MEMORY_SIZE);
  fread(memory, sizeof(int8_t), MEMORY_SIZE, f);

  uint8_t *firmware = malloc(FIRMWARE_SIZE * sizeof(uint8_t));
  fread(firmware, sizeof(uint8_t), FIRMWARE_SIZE, f);

  /* print_firmware(firmware); */

  cpu_state *cpu = create_cpu(memory, firmware);

  puts("Loaded memory:");
  print_memory(cpu->memory);

  do {
    cycle(cpu);
  }
  while (!cpu->halted);

  puts("Execution finished!");

  if (dump_memory) {
    puts("Dumping memory:");
    print_memory(cpu->memory);
  }

  fclose(f);
  return 0;
}
