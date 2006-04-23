struct coordinates {
	float x;
	float y;
	float z;
};

struct rectangle {
	struct coordinates front;
	struct coordinates rear;
};

struct s3d_object {
	int oid;
	char name[20];
	float pos[3];
	float rot[3];
	float poi[3];
	int focus;
};

extern int obj_term;
extern int obj_cursor;
extern int obj_cursor_mp;
extern struct s3d_object **obj;
extern struct olsr_node *search_node;

void create_cursor();
void create_terminal();
void rotate_cursor();
void write_terminal(int key);
