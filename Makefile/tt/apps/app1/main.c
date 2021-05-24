#include <stdio.h>

#include "../../include/modlue1/m1-a.h"
#include "../../include/modlue1/m1-b.h"
#include "../../include/modlue2/m2-a.h"
#include "../../include/modlue2/m2-b.h"
#include "../../include/modlue3/m3-a.h"
#include "../../include/modlue3/m3-b.h"

#include "../../include/test.h"



int main()
{
    m1_a_p1();
    m2_a_p1();
    m3_a_p1();
    m1_b_p1();
    m2_b_p1();
    m3_b_p1();

    main_test_p();

    return 0;
}
