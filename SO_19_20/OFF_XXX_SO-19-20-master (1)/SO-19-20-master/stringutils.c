#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void strtohex(char* str_in, char* str_out)
{
	unsigned char v1, v2;

	for (int i=0; i<=strlen(str_in); i++)
	{
		v1 = (unsigned char) str_in[i] / 16;
		v2 = (unsigned char) str_in[i] % 16;

		if (v1 <= 9)
			v1 = '0' + v1;
		else
			v1 = 'A' + v1 - 10;

		if (v2 <= 9)
			v2 = '0' + v2;
		else
			v2 = 'A' + v2 - 10;

		str_out[i*2] = v1;
		str_out[i*2+1] = v2;
	}

	str_out[strlen(str_in)*2] = '\0';
}

void hextostr(char* str_in,char* str_out)
{
    char v1, v2;

    for (int i=0, j=1; j<=strlen(str_in); i+=2, j+=2)
    {
        if (str_in[i]<='9')
            v1 = str_in[i] - '0';
        else
            v1 = str_in[i] - 'A' + 10;

        if (str_in[j]<='9')
            v2 = str_in[j] - '0';
        else
            v2 = str_in[j] - 'A' + 10;

        str_out[i/2] = (char) (v1*16 + v2);
    }

    str_out[strlen(str_in)/2] = '\0';
}