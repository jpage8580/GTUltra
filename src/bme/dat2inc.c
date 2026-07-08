//
// Datafile -> C include file
//

#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *in;
    FILE *out;
    int length;
    int c;

    if (argc < 3)
    {
        printf("Usage: dat2inc <datafile> <includefile>\n\n");
        return 1;
    }
    in = fopen(argv[1], "rb");
    if (!in)
    {
        printf("Datafile open error!\n");
        return 1;
    }
    out = fopen(argv[2], "wt");
    if (!out)
    {
        printf("Includefile open error!\n");
        return 1;
    }
    fseek(in, 0, SEEK_END);
    length = ftell(in);
    fseek(in, 0, SEEK_SET);

    fprintf(out, "unsigned char datafile[] = {\n");
    for (c = 0; c < length; c++)
    {
        if (c)
        { 
            fprintf(out, ", ");
            if (!(c % 10)) fprintf(out, "\n");
        }
        fprintf(out, "0x%02x", fgetc(in));
    }
    fprintf(out, "};\n");
    fclose(in);
    fclose(out);
    return 0;
}

