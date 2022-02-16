/*
 *
 *	int readable(char *inputPath)
	Where: <inputPath> is a parameter that specifies a path in the directory system.
	If no parameter is present (NULL), then your program will operate on the current
	working directory. Your function should return the number of regular files that
	are readable.
 */
#include <stdio.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
int readable(char *inputPath){
	//create directory variable pointer named dir. It will hold the pointer to the directory being opened
	DIR *dir;
	//if there is no input(inputPath is NULL), get the current working directory using the function getcwd and set inputPath to it
	if(inputPath == NULL){
		//string is the empty input of size 256 for getcwd, getcwd needs it 
		char string[256] = " ";
		//set string to the current directory in getcwd, with string and 256 (size of string) as inputs 
		getcwd(string,256);
		//set inputPath to string
		inputPath = string;	
		//if inputPath is NULL, print error and return the negative of the error number
		if(inputPath == NULL){
			int error = errno;
			fprintf(stderr,"Error #%d = %s",error,strerror(error));
			return -1*error;
		}
	}
	//create a struct stat variable and its pointer p. They will be useful in giving information about a file
	struct stat dir_check, *p = &dir_check;
	//if p is successfuly set to inputPath (returns a 0),check to see if the file is a regular file or a directory (if it is a directory, the directory will be opened inside)
	//returns a 1 if inputPath is a readable regular file, 0 if it isn't
	if(lstat(inputPath,p) == 0){
		//checks if inputPath is a readable regular file using, and return a 1 if readable, 0 if not
		if(S_ISREG (p->st_mode)){
			if(access(inputPath,(R_OK)) == 0){
				return 1;
			}
			else{
				return 0;
			}
		}
		//checks if inputPath is a directory and open the directory inside the if statement
		if(S_ISDIR (p->st_mode)){
			//dir is set the output of inputPath being opened in inputPath
			dir = opendir(inputPath);
			//if dir is NULL, print error and return negative of the error
			if(dir == NULL){
				int error = errno;
				fprintf(stderr,"Error #%d = %s",error,strerror(error));
				return -1*error;
			}
			/*change current working directory to inputPath, as this function will be called recursively and will need to travel 
			 * through a child directory each specific call. As the parent will still be open, it is important for the child that 
			 * it (the specified working directory that is recursively inputed) is the current open directory. the chdir output 
			 * will also be stored in the variable change_directory_error, which is the error check var*/ 
			int change_directory_error = chdir(inputPath);
			//check change_directory_error for -1, if true,print error message and return the error#'s negative
			if(change_directory_error == -1){
				int error = errno;
				fprintf(stderr,"Error #%d = %s",error,strerror(error));
				return -1*error;
			}
		}
	}
	//if lstat failed (equal to -1), return the negative of the error
	else if(lstat(inputPath,p) == -1){
		int error = errno;
		return -1*error;
	}
	//create a struct dirent pointer dp. It will be used read the directory
	struct dirent *dp;
	//count will be the running count of regular files in a directory
	int count = 0;
	//create struct stat variables file_check and pointer to file_check s. They will be useful in giving information about the files in the directory
	struct stat file_check, *s = &file_check;
	//dp will read the directory dir until it gets to NULL, or in human terms, will read all the files in the directory
	while((dp = readdir(dir)) != NULL){
		//create a buffer to store file names
		char buffer[256];
		//copy the name of the file pointed by dp to buffer
		strcpy(buffer,dp->d_name);
		//if lstat successfully set the file represented by the buffer to s, check if the file is a regular file or a directory
		if(lstat(buffer,s) == 0){
			//check if the file is a regular file, if it is a regular file, use the built-in access function to determine its readibility, if readable, increment count
			if(S_ISREG (s->st_mode)){
				//check to see if buffer is good for reading
				int check_access = access(buffer,(R_OK));
				//on success (0), increment count
				if(check_access == 0){
					count++;
				}
			}
			/*execute if the file is a child directory.If successful, the if statement will recursively count the number of regular files in the child directory 
			 * and add it up to count If so, first check to see if the directory is accessible and if so, check to see buffer so that its not "." or "..", as it 
			 * will get stuck in a loop if you don't take any measures.On success with both checks, Concatenate the buffer with the string temp, which holds the 
			 * "./" phrase that goes at the begginign of buffer to access a directory. Make sure that you are in the right working directory. Recursively increment
			 *  count by inputting temp  into the function and after recursing, add the return_value of temp to count. Make sure to change the directory to the
			 *  initail paramater one as the current working directory would still be the child directory. */
			if(S_ISDIR(s->st_mode)){
				//check if the directory pointed by the buffer is readable in access
				if(access(buffer,(R_OK)) == 0){
					//before executing the code block, check to see if the buffer is not equal to "." and ".." to prevent infinite loops
					if((strcmp(".",buffer) != 0) && (strcmp("..",buffer) != 0)) {
						//allocate the same amount of space that buffer has and 2 more bytes (which will hold the "./" symbol) to temp
						char *temp = malloc(sizeof(buffer+2));
						//set temp to "./"
						strcpy(temp,"./");
						//concatenate temp with buffer. temp is the destination
						strcat(temp,buffer);
						//make sure inputPath is the same as the current working directory by setting inputPath to cwd. 
						//current is the string input for getcwd, while 256 is the size of the string to be outputted
						char current[256];
						inputPath = getcwd(current,256);
						//if inputPath failed to get cwd, print the error and return a negative of the error
						if(inputPath == NULL){
							int error = errno;
							fprintf(stderr,"Error #%d = %s",error,strerror(error));
							return error*-1;
						}
						//recurse temp and return the number of files in the child directory in recurse_count
						int recurse_count = readable(temp);
						count = count + recurse_count;
						//change the directory to the original paramater input, and save any error in new_directory_error
						int new_directory_error = chdir(inputPath);
						//if an error occurs in chdir, print the error message and return the negative of the error
						if(new_directory_error == -1){
							int error = errno;
							fprintf(stderr,"Error #%d = %s",error,strerror(error));
                                                	return -1*error;
						}
						//free temp, as it was dynamically allocated memory, to prevent memory leaks
						free(temp);
					}
				}	
			}
		}
		//if lstat fails on buffer and s (lstat returns -1), return the negative of the error number
		if(lstat(buffer,s) == -1){
			int error = errno;
			return -1*error;
		}
	}
	//close the directory dir and save its result in finish for error_checking
	int finish = closedir(dir);
	//if an error exists,print the error message and return negative the error #
	if(finish == -1){
		int error = errno;
		fprintf(stderr,"Error #%d = %s",error,strerror(error));
		return -1*error;
	}
	//return the readable regular file count
	return count;
}
