#include <stdio.h>
int digit_cnt(char *argv)
{
    int num = 0;
    unsigned int i = 0;
    while(argv[i] != '\0' )
    {
        if (argv[i] >= '0' && argv[i] <= '9')
            num++;
        i++;
    }
    return num;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Less arguements than needed.\n");
        return 1;
    }
    printf("the number is %d\n", digit_cnt(argv[1]));
    return 0;
}

/*
    ** Digit counter **
    > (readelf -s digit_counter)
    Virtual address:   0000056d
    size: 80
    section: 14
    > (readelf -S digit_counter)
    Virtual address:   00000470
    offset: 00000470

    ** ntsc **
    > (readelf -s ntsc)
    Virtual address: 00000577
    size:  1136
    section: 14
    > (readelf -S ntsc)
    txt Virtual address:   00000410
    txt offset: 00000410

 */