/*
 * PARSEC HEADER: e_demo.h
 */

#ifndef _E_DEMO_H_
#define _E_DEMO_H_


// external functions

size_t		DEMO_ParseDemoHeader( FILE *fp, int demoid, int verbose );
void		DEMO_RegisterInitialDemos();

void		DEMO_UserInputDisabling();

int			DEMO_ReplayActive();
void		DEMO_StopReplay();
void		DEMO_ClearDemo( int stopreplay );
void		DEMO_InitInfo();
void		DEMO_CountDemoFrame();

int			DEMO_TimedemoActive();
refframe_t	DEMO_GetTimedemoBase();

int			DEMO_BinaryReplayActive();
void		DEMO_BinaryStopReplay();
FILE*		DEMO_BinaryOpenDemo( const char *demoname );
void		DEMO_BinaryExecCommands( int interactive );

int			DEMO_PlayBehindMenu( const char *demoname );
int			DEMO_PlayFromMenu( const char *demoname );

int			Cmd_PlayBinaryDemoFile( char *command );


// external variables

extern int			num_registered_demos;
extern int			max_registered_demos;
extern char*		registered_demo_names[];
extern char*		registered_demo_titles[];

extern refframe_t	demoinfo_curtime;
extern int			demoinfo_curline;
extern int			demoinfo_curframe;
extern dword		demoinfo_curofs;
extern refframe_t	demoinfo_lastwait;

struct keyfunc_s;
extern keyfunc_s*	key_replay_map;


#endif // _E_DEMO_H_


