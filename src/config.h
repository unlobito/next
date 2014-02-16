typedef struct course course;
struct course {
	unsigned int code;
	char *name;
	char *period;
	char *instructor;
	unsigned int start_time;
	unsigned int end_time;
};

typedef struct schoolconfig schoolconfig;
struct schoolconfig {
	unsigned int start_time;
	unsigned int end_time;
	unsigned int end_time_academictime;
};