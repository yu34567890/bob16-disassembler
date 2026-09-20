#include "decompiler.h"
#include "stdio.h"
#include <stdlib.h>
#include <inttypes.h>

#define FILE_SIZE (128 * 1024) 

static uint8_t target_flags[65536];  

int main(int argc, char **argv) // assembler havent rewriten yet
{

	uint16_t memory[65536]={0};
	
	if (argc != 2)
	{
		printf("wrong number of arguments given" 
		"\nusage: bob16disassembler <program.bin>\n");
		exit(1);
	}
	
	FILE *file = fopen(argv[1], "r");

	if (file==NULL)
	{
		perror("file not found");
		exit(0xdeadbeef);
	}

	size_t bytesRead = fread(memory, 1, FILE_SIZE, file);
	printf("program loaded size:%lu\n", bytesRead);
	printf("first 4 words 0x%X 0x%X 0x%X 0x%X\n", memory[0], memory[1], memory[2], memory[3]);
	fflush(stdout);


	fclose(file);

	// first pass labels
	for (size_t i=0; i<65536; i++)
	{
		uint16_t ir = memory[i];
		INSTRUCTION opcode = ir>>12;
		
		
		switch (opcode)
		{
			case JSR:
				target_flags[(uint16_t)(i+(sext_11(ir&0x7ff)+1))] = 1;
				break;

			case BR:
				if ((uint16_t)(i+(sext_9(ir&0x1ff)+1)) > 3)
					target_flags[(uint16_t)(i+(sext_9(ir&0x1ff)+1))] = 1;
				break; 

			case LD:
			case LDI:
			case ST:
			case STI:
				target_flags[(uint16_t)(i+((sext_9(ir&0x1ff)+1)))] = 2;
				break;

		}


		
	}
	
	// second pass disassembling
	for (size_t i=0; i<65536; i++)
	{
		INSTRUCTION opcode = memory[i]>>12; 
		char tmp_buffer[32]={0};
		uint16_t ir = memory[i];
		if (target_flags[i]==1)
		{
			printf("\nL_%d:\n", i);	
		}

		if (opcode == JSR && !(ir & 0x800))
		{
			sprintf(tmp_buffer, "(L_%d)", (uint16_t)(i+(sext_11 (ir&0x7ff)+1)));
			printf("%04" PRIx16 ":%04" PRIx16 ":%s %s\n", (uint16_t)i, memory[i], decode(memory[i]), tmp_buffer);

		}

		else if (opcode == BR)
		{
			sprintf(tmp_buffer, "(L_%d)", (uint16_t)(i+(sext_9(ir&0x1ff)+1)));
			if (target_flags[i]==1)
				printf("%04" PRIx16 ":%04" PRIx16 ":%s %s\n", (uint16_t)i, memory[i], decode(memory[i]), tmp_buffer);
			else 
				printf("%04" PRIx16 ":%04" PRIx16 ":%s\n", (uint16_t)i, memory[i], decode(memory[i]));

		}
		

		else 
		{
			printf("%04" PRIx16 ":%04" PRIx16 ":%s\n", (uint16_t)i, memory[i], decode(memory[i]));



		}
	}
}
