#define MAX_LINES	50
#define MAX_CHARS	80
#define DEFAULT_FGCOLOR 2
#define DEFAULT_BGCOLOR 0
#define X_RATIO		0.75
#define CS			0.1

#define M_PIPE		1
#define M_PTY		2

/* #define M_LINE		1 */
#define M_CHAR		1

typedef struct char_struct
{
    char character;
    char fgcolor;
    char bgcolor;
}t_char;

typedef struct line_struct
{
    t_char chars[MAX_CHARS+1];
} t_line;

extern t_line line[MAX_LINES+1];
/* main.c */
void paintit();
void term_addchar(char toprint);
/* terminal.c */
void AddChar(char *_toadd);

extern int gotnewdata;
extern int cx,cy;
