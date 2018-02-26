#include "Test.h"

const bool learn = false;

int main()
{
    Init();
    if (learn)
    {
        Learn();
    }
    else
    {
        Test();
    }
    return 0;
}