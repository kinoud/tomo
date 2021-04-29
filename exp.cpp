#include"config.h"
#include<fstream>
using namespace std;

// CONFIGURATION BEGIN
/*
* 1: projection
* 2: reconstruction
*/
int task = 2;
char* working_dir = "exp/task/head";
// CONFIGURATION END

Config* cfg = new Config();
extern void projection();
extern void reconstruction();

void open_file(ifstream& s, const char* fname) {
    static char tmp_str[100];
    sprintf(tmp_str, "%s/%s", working_dir, fname);
    if (s.is_open())s.close();
    s.open(tmp_str);
    printf("opening file '%s' ...\n", tmp_str);
    if (!s.is_open()) {
        printf("opening file '%s' failed\n", tmp_str);
        exit(-1);
    }
}

int main() {
	if (task == 1)projection();
	else if (task == 2)reconstruction();
}