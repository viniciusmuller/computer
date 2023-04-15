alias b := build
alias r := run

host := `uname -a`

cpu_binary := "cpu"

build:
  gcc *.c -o {{cpu_binary}}

run PROGRAM: build
  ./{{cpu_binary}} {{PROGRAM}}
