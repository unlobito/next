typedef struct course course;
struct course {
	unsigned int code;
	char *name;
	char *period;
	char *instructor;
	char *start_time;
	char *end_time;
	unsigned int start_time_seconds;
	unsigned int end_time_seconds;
};

typedef struct appconfig appconfig;
struct appconfig {
	char *start_time;
	char *end_time;
	unsigned int start_time_seconds;
	unsigned int end_time_seconds;
	unsigned int hr24_time;
};