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
  AND = 1,
  OR = 2,
  XOR = 3,
  NOT = 4,
  LDA = 5,
  STA = 6,
  HLT = 7,
  TST = 8,
  JPZ = 9,
  ADD = 10,
  OUT = 11
} instruction;

void cycle(cpu_state *cpu) {
  if (cpu->offset + 2 == MEMORY_SIZE) {
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
    printf("OUT at %x: %d\n", cpu->offset, cpu->reg);
  }

  if (instruction == STA) {
    cpu->memory[address] = cpu->reg;
  }
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

  uint8_t *firmware = malloc(FIRMWARE_SIZE);
  fread(firmware, sizeof(uint8_t), FIRMWARE_SIZE, f);

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
