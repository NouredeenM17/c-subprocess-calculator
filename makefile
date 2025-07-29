
all: trmn adder subtractor multiplier divider

trmn: trmn.c
	gcc trmn.c -o trmn

adder: adder.c
	gcc adder.c -o adder

subtractor: subtractor.c
	gcc subtractor.c -o subtractor

multiplier: multiplier.c
	gcc multiplier.c -o multiplier

divider: divider.c
	gcc divider.c -o divider
