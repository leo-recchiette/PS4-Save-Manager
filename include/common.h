#ifndef _COMMON_H_
#define _COMMON_H_

#define SUCCESS 	0
#define FAILED	 	-1

const char * get_user_language(void);

//----------------------------------------
//FILE UTILS
//----------------------------------------

int file_exists(const char *path);
int dir_exists(const char *path);
int unlink_secure(const char *path);
int mkdirs(const char* dir);
int copy_file(const char* input, const char* output);
int clean_directory(const char* inputdir, const char* filter);

#endif
