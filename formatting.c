#include "formatting.h"

// void get_terminal_size(int *rows, int *cols)
// {
//     struct winsize w;
//     ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
//     *rows = w.ws_row; 
//     *cols = w.ws_col;
// }

void welcome_message(int width)
{
    if (width > 70 )
    {
        printf("@@@@@@@@   @@@@@@@@   @@@@@@@@   @@@@@@   @@@  @@@  @@@@@@@@  @@@       @@@\n");
        printf("@@@@@@@@  @@@@@@@@@  @@@@@@@@@  @@@@@@@   @@@  @@@  @@@@@@@@  @@@       @@@\n");
        printf("@@!       !@@        !@@        !@@       @@!  @@@  @@!       @@!       @@!\n");
        printf("!@!       !@!        !@!        !@!       !@!  @!@  !@!       !@!       !@!\n");
        printf("@!!!:!    !@! @!@!@  !@! @!@!@  !!@@!!    @!@!@!@!  @!!!:!    @!!       @!!\n");
        printf("!!!!!:    !!! !!@!!  !!! !!@!!   !!@!!!   !!!@!!!!  !!!!!:    !!!       !!!\n");
        printf("!!:       :!!   !!:  :!!   !!:       !:!  !!:  !!!  !!:       !!:       !!:\n");
        printf(":!:       :!:   !::  :!:   !::      !:!   :!:  !:!  :!:        :!:       :!:\n");
        printf(" :: ::::   ::: ::::   ::: ::::  :::: ::   ::   :::   :: ::::   :: ::::   :: ::::\n");
        printf(": :: ::    :: :: :    :: :: :   :: : :     :   : :  : :: ::   : :: : :  : :: : :\n");

    }

    else
    {
        printf(" _______                     __           __ __\n");
        printf("|    ___|.-----.-----.-----.|  |--.-----.|  |  |\n");
        printf("|    ___||  _  |  _  |__ --||     |  -__||  |  |\n");
        printf("|_______||___  |___  |_____||__|__|_____||__|__|\n");
        printf("         |_____|_____|                          \n");
    }
}