#ifndef TODORL_H__
#define TODORL_H__

/*	
	Explicitly declare readline functions because on several distributions
	the readline headers do NOT compile under C++ at all due to a lack of
	function parameters.
*/
typedef int intfunc ();

extern "C" int rl_initialize();
extern "C" char *rl_readline_name;
extern "C" char **completion_matches(char*, char*(*)(...));
extern "C" void rl_insert_text(char*);
extern "C" char **(*rl_attempted_completion_function)(...);
extern "C" int (*rl_startup_hook)(...);
extern "C" char *readline(char*);
extern "C" void add_history(char*);
extern "C" int rl_getc(FILE *);
extern "C" int rl_point, rl_end, rl_mark;
extern "C" int rl_delete_text(int, int);
extern "C" void rl_redisplay();
extern "C" void rl_clear_message();
extern "C" intfunc *rl_getc_function;

#endif
