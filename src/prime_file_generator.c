#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define ull unsigned long long

int main(int argc, const char **argv){
    if(argc < 2){
        printf("Argumentos insuficientes\n");
    }
    else{
        FILE *in_file = fopen(argv[1], "r");
        FILE *out_file = fopen("encoded_primes.p", "w+");
        char buffer[10];
        int buf_ocup = 0;
        char c;
        do
        {
            c = getc(in_file);
            if(isdigit(c))
                buffer[buf_ocup++] = c;
            else if (buf_ocup != 0)
            {
                buffer[buf_ocup] = 0;
                
                char * e;
                ull n = strtoll(buffer, &e, 0);
                printf("%lld\n",n);
                fwrite(&n,sizeof(long long),1, out_file);
                buf_ocup = 0;
            }
        } while (c!=EOF);

        fclose(in_file);
        fclose(out_file);
    }
}