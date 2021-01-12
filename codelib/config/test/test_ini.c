#include "../src/libconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

static int ini_test(void)
{
    struct config *conf = conf_load("example.ini");
    printf("***** ini_test *****\n");

    printf("\n***** start dump *****\n");
    conf_dump(conf);

    printf("\n***** get vaule from [wine] *****\n");
    printf("grape = %s\n", conf_get_string(conf, "wine:grape"));
    printf("year = %d\n", conf_get_int(conf, "wine:year"));
    printf("country = %s\n", conf_get_string(conf, "wine:country"));
    printf("year = %f\n", conf_get_double(conf, "wine:alcohol"));

    printf("\n***** set vaule for [pizza] *****\n");
    conf_set_string(conf, "pizza:ham", "no");
    conf_set_string(conf, "pizza:mushrooms", "False");
    conf_set_string(conf, "pizza:capres", "1");
    conf_set_string(conf, "pizza:cheese", "have");
    conf_set_string(conf, "pizza:test", "dddddd");

    printf("\n***** test dump *****\n");
    conf_dump(conf);

    printf("\n***** test del *****\n");
    conf_del(conf, "wine:test");

    printf("\n***** end dump *****\n");
    conf_dump(conf);

    // conf_save(conf);
    conf_unload(conf);

    return 0;
}


int main(int argc, char **argv)
{
    ini_test();

    return 0;
}
