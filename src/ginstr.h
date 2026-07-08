#ifndef GINSTR_H
#define GINSTR_H

#define LAST_INST 10

#ifndef GINSTR_C
extern INSTR instrcopybuffer;
#endif

void instrumentcommands(GTOBJECT *gt);
void nextinstr(void);
void previnstr(void);
void clearinstr(int num);
void gotoinstr(int i);
void showinstrtable(void);

#endif
