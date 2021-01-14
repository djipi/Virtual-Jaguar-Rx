#include <stdlib.h>

// Decode an unsigned LEB128
// Algorithm from Appendix C of the DWARF 2, and 3, spec section "7.6"
unsigned long ReadULEB128(char *addr)
{
	unsigned long result = 0;
	size_t shift = 0;
	unsigned char byte;

	do
	{
		byte = *addr++;
		result |= (byte & 0x7f) << shift;
		shift += 7;
	}
	while ((byte & 0x80));

	return result;
}


// Decode a signed LEB128
// Algorithm from Appendix C of the DWARF 2, and 3, spec section "7.6"
long ReadLEB128(char *addr)
{
	long result = 0;
	size_t shift = 0;
	unsigned char byte;

	do
	{
		byte = *addr++;
		result |= (byte & 0x7f) << shift;
		shift += 7;
	}
	while ((byte & 0x80));

	if ((shift < (8 * sizeof(result))) && (byte & 0x40))
	{
		result |= (~0 << shift);
	}

	return result;
}

