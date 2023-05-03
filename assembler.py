import sys
from dataclasses import dataclass
import itertools
import pprint

MAIN_LABEL_NAME = 'MAIN'
COMMENT_STRING = ';'

WORD_SIZE = 1
INSTRUCTION_SIZE = 2
TOTAL_MEMORY_CELLS = 16

INSTRUCTIONS_TABLE = {
    "AND": 1,
    "OR": 2,
    "XOR": 3,
    "NOT": 4,
    "LDA": 5,
    "STA": 6,
    "HLT": 7,
    "TST": 8,
    "JPZ": 9,
    "ADD": 10,
    "OUT": 11,
    "JMP": 12
}

@dataclass
class Instruction:
    name: str
    target: str | int

@dataclass
class Label:
    name: str
    instructions: list[Instruction]

@dataclass
class ProgramStructure:
    memory: list
    labels: list[Label]

def compile(content):
    lines = process_content(content)
    structure = get_structure(lines)

    pprint.pprint(structure)

    if not any(map(lambda l: l.name == MAIN_LABEL_NAME, structure.labels)):
        raise Exception(f"\"{MAIN_LABEL_NAME}\" label must be provided")

    if not structure.labels[0].name == MAIN_LABEL_NAME:
        raise Exception(f"{MAIN_LABEL_NAME} must be the first label")

    labels_addresses = {}
    offset = 0

    for label in structure.labels:
        labels_addresses[label.name] = offset
        label_size = len(label.instructions) * INSTRUCTION_SIZE
        offset += label_size

    print(labels_addresses)

    compiled_labels = [compile_label(label, labels_addresses) for label in structure.labels]
    result = structure.memory + [*itertools.chain.from_iterable(compiled_labels)]
    print(compiled_labels)
    print(result)
    print(len(result))
    return bytes(result)

def compile_label(label, label_addresses) -> bytes:
    instructions: [[int, int]] = []

    for instruction in label.instructions:
        instruction_binary = INSTRUCTIONS_TABLE.get(instruction.name)

        if instruction_binary is None:
            raise Exception(f"Invalid instruction: {instruction.name}")

        if instruction.target is None:
            instructions.append([instruction_binary, 0x0])
            continue

        if type(instruction.target) == int:
            instructions.append([instruction_binary, instruction.target])
        else:
            label_address = label_addresses.get(instruction.target)
            if label_address is None:
                raise Exception(f"Unknown target: {instruction.target}")

            instructions.append([instruction_binary, label_address])

    return [*itertools.chain.from_iterable(instructions)]

def get_structure(processed_lines):
    memory = []
    memory_lines = processed_lines[:TOTAL_MEMORY_CELLS]

    # parse memory (always the first 16 lines)
    for line in memory_lines:
        [_, memory_value, *_] = line.split(":")
        parsed_value = int(memory_value.strip(), 16)

        if parsed_value < 0 or parsed_value > 0xff:
            raise Exception(f"Parsed value must be between 0x0 and 0xFF: {memory_value.strip()}")

        memory.append(parsed_value)

    # parse code (rest)
    labels = []

    current_label_name = None
    current_label_instructions = []

    for line in processed_lines[TOTAL_MEMORY_CELLS:]:
        if line.startswith("label"):
            if current_label_name is not None:
                if len(current_label_instructions) == 0:
                    raise Exception(f"Label without any instructions: {current_label_name}")

                labels.append(Label(current_label_name, current_label_instructions))
                current_label_instructions = []

            [_, label_name, *_] = line.split(" ")
            current_label_name = label_name.removesuffix(":")
            continue

        # unary instruction (e.g: HLT)
        if line.split(" ") == [line]:
            current_label_instructions.append(Instruction(line, None))
            continue

        # binary instructions
        [instruction, target, *_] = line.split(" ")
        try:
            target = int(target, 16)
        except ValueError:
            pass

        current_label_instructions.append(Instruction(instruction, target))

    if current_label_name is not None:
        if len(current_label_instructions) == 0:
            raise Exception(f"Label without any instructions: {current_label_name}")

        labels.append(Label(current_label_name, current_label_instructions))

    return ProgramStructure(memory, labels)

def process_content(content):
    lines = []

    for line in content.split('\n'):
        line = line.strip()

        # Lines that are comments
        if line.startswith(COMMENT_STRING):
            continue

        if line == "":
            continue

        # Remove inline comments
        comment_index = line.find(COMMENT_STRING)
        if comment_index > 0:
            line = line[:comment_index]

        lines.append(line)

    return lines

def write_target(binary, target):
    with open(target, 'wb') as f:
        f.write(binary)

def main():
    source = sys.argv[1]
    target = sys.argv[2]

    with open(source, 'r') as f:
        content = f.read()
        binary = compile(content)
        print(binary)
        write_target(binary, target)

    print(f"Succesfully assembled {target} from {source}")

if __name__ == '__main__':
    main()
