//  SourceGrades.c  //
/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex2 - Grades

* *********************************************/
/// Description: This is the functions module for the project Grades. Contains the different functions that main module uses.

// Includes:
#include "SourceGrades.h"

/// === thread_main ===
/// Description: This is the main function that runs for every school, by a thread.
///				 It reads the grades from folders and calculate the average for each student in that school.
///				 The results with the averages are saved to Results folder, in relevant text file.
/// Parameters:
///		lpParam - a void pointer that refers to grade_params struct. It contains 
///				  the index of the school and the weigt for grading each subject
///	Returns: int value - 0 if succeeded, 1 if failed
DWORD WINAPI thread_main(LPVOID lpParam)
{
	const char* input_folders[FOLDERS_NUM] = { "Real", "Human", "Eng", "Eval" };
	char* result_char_arr = NULL;
	int* int_grades_arr = NULL, * result_arr = NULL, * p_lines_count = NULL;
	int path_len, folder_index = 0, ret_val = 0;
	char* path_name = NULL;

	grade_params* p_params;
	/* Check if lpParam is NULL */
	if (NULL == lpParam) {
		printf("Error: pointer 'lpParam' is NULL\n");
		return STATUS_FAILURE;
	}
	p_params = (grade_params*)lpParam;
	int weight_arr[FOLDERS_NUM] = { p_params->real,p_params->human,p_params->english,p_params->eval };

	p_lines_count = (int*)malloc(sizeof(int));
	if (NULL == p_lines_count) {
		printf("Error: pointer is NULL, malloc failed");
		return STATUS_FAILURE;
	}
	*p_lines_count = -1; // marks that we didn't find the num of lines yet

	// find lines count in order to creare result array and start calculating the avg
	folder_index = 0;
	path_len = snprintf(NULL, 0, "Debug/%s/%s%d.txt", input_folders[folder_index], input_folders[folder_index], p_params->index);
	path_name = (char*)malloc(sizeof(char) * path_len + 1);
	if (NULL == path_name) {
		printf("Error: pointer is NULL, malloc failed\n");
		return STATUS_FAILURE;
	}
	snprintf(path_name, path_len + 1, "Debug/%s/%s%d.txt", input_folders[folder_index], input_folders[folder_index], p_params->index);
	//printf("%s\n", path_name);
	int_grades_arr = read_file(path_name, p_lines_count);
	if (NULL == int_grades_arr) {
		printf("Error: A function failed\n");
		return STATUS_FAILURE;
	}
	result_arr = (int*)malloc(*p_lines_count * sizeof(int));
	if (NULL == result_arr) {
		printf("Error: pointer is NULL, malloc failed\n");
		return STATUS_FAILURE;
	}
	//put zeros in result array
	for (int i = 0; i < *p_lines_count; i++) {
		result_arr[i] = 0;
	}
	//add contribution for the first subject to the avg
	ret_val = calc_avg(int_grades_arr, result_arr, weight_arr[folder_index], *p_lines_count);
	if (ret_val) {
		printf("Error: A function failed\n");
		return STATUS_FAILURE;
	}
	free(path_name);

	// continue to calc avg with the rest of the folders
	for (folder_index = 1; folder_index < FOLDERS_NUM; folder_index++) {
		path_len = snprintf(NULL, 0, "Debug/%s/%s%d.txt", input_folders[folder_index], input_folders[folder_index], p_params->index);
		path_name = (char*)malloc(sizeof(char) * path_len + 1);
		if (NULL == path_name) {
			printf("Error: pointer is NULL, malloc failed\n");
			return STATUS_FAILURE;
		}
		snprintf(path_name, path_len + 1, "Debug/%s/%s%d.txt", input_folders[folder_index], input_folders[folder_index], p_params->index);
		//printf("%s\n",path_name);
		int_grades_arr = read_file(path_name, p_lines_count);
		if (NULL == int_grades_arr) {
			printf("Error: A function failed\n");
			return STATUS_FAILURE;
		}
		ret_val = calc_avg(int_grades_arr, result_arr, weight_arr[folder_index], *p_lines_count);
		if (ret_val) {
			printf("Error: A function failed\n");
			return STATUS_FAILURE;
		}
		free(path_name);
	}
	// final calc - for the total avg:
	for (int i = 0; i < *p_lines_count; i++) {
		result_arr[i] = result_arr[i] / 100;
	}
	//convert the result int array into string
	result_char_arr = convert_int_array_to_str(result_arr, p_lines_count);
	if (NULL == result_char_arr) {
		printf("Error: A function failed\n");
		return STATUS_FAILURE;
	}
	// write avg to file: //
	//path name to file write
	path_len = snprintf(NULL, 0, "Debug/Results/Results%d.txt", p_params->index);
	path_name = (char*)malloc(sizeof(char) * path_len + 1);
	if (NULL == path_name) {
		printf("Error: pointer is NULL, malloc failed");
		return STATUS_FAILURE;
	}
	snprintf(path_name, path_len + 1, "Debug/Results/Results%d.txt", p_params->index);
	ret_val = write_file(path_name, *p_lines_count, result_char_arr);
	if (ret_val) {
		printf("Error: A function failed\n");
		return STATUS_FAILURE;
	}
	free(path_name);
	free(int_grades_arr);
	free(p_lines_count);
	free(result_arr);
	free(result_char_arr);
	return STATUS_SUCCESS;
}

/// === read_file ===
/// Description: This function reads from grades file in a certain subject.
///				 It converts the grades values into an int array and returns it.
/// Parameters:
///		file_name - the file name that needs to be read.
///		p_lines_count - int pointer that refers to the amount of lines in the file.
///					  if this is the first time the read is used - the pointer is to value of -1. 
///	Returns: int array of the grades values that were read.
extern int* read_file(LPSTR file_name, int* p_lines_count)
{
	int ret_val = 0;
	if (NULL == p_lines_count) {
		printf("Error: argument pointer is NULL\n");
		return NULL; //error
	}
	int* int_grades_arr = NULL;

	HANDLE grades_file = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (grades_file == INVALID_HANDLE_VALUE) { 		// If failed to open file
		printf("Error: Failed to open plain file! The error code is %d\n", GetLastError());
		return NULL; //error
	}
	DWORD file_size, dw_bytes_read = 0;
	file_size = GetFileSize(grades_file, NULL);
	char* grades_arr = (char*)malloc(file_size * sizeof(char) + 1);
	if (grades_arr == NULL) {
		printf("Error: Failed to allocate memory\n");
		return NULL; //error
	}

	BOOL read_status = ReadFile(grades_file, (void*)grades_arr, file_size, &dw_bytes_read, NULL);
	if (read_status == 0) {		// If failed to read file
		printf("Error: Failed to Read file! The error code is %d\n", GetLastError());
		return NULL; //error
	}
	if (dw_bytes_read > 0)
		grades_arr[dw_bytes_read] = '\0';

	if (-1 == (*p_lines_count)) { // if we didn't find the num of lines in file yet
								// convert the file to int array of grades
		int_grades_arr = (int*)malloc(file_size * sizeof(int));
		if (int_grades_arr == NULL) {
			printf("Error: Failed to allocate memory\n");
			return NULL; //error
		}
		ret_val = convert_str_to_int_arr(grades_arr, int_grades_arr, p_lines_count);
		if (ret_val) {
			printf("Error: A function failed\n");
			return NULL; //error
		}
		//realloc according to num of lines
		int_grades_arr = (int*)realloc(int_grades_arr, (*p_lines_count) * sizeof(int));
		if (int_grades_arr == NULL) {
			printf("Error: Failed to re-allocate memory\n");
			return NULL; //error;
		}
	}
	else { // if we already know the num of lines
		int_grades_arr = (int*)malloc((*p_lines_count) * sizeof(int));
		if (int_grades_arr == NULL) {
			printf("Error: Failed to allocate memory\n");
			return NULL; //error
		}
		ret_val = convert_str_to_int_arr(grades_arr, int_grades_arr, p_lines_count);
		if (ret_val) {
			printf("Error: A function failed\n");
			return NULL; //error
		}
	}
	ret_val = CloseHandle(grades_file);
	if (FALSE == ret_val) {
		printf("Error when closing file handle. The error is: %d\n", GetLastError());
		return NULL; //error
	}
	free(grades_arr);
	return int_grades_arr;
}


/// === convert_str_to_int_arr ===
/// Description: This function converts string source to int array.
/// Parameters:
///		source_file - char pointer to a source string to be converted.
///		source_arr - int pointer to the result array - eventually will be converted from string to int.
///		p_lines_count -  int pointer that refers to the amount of lines in the file.	
///	Returns: int value - 0 if succeeded, 1 if failed
extern int convert_str_to_int_arr(char* source_file, int* source_arr, int* p_lines_count) {
	if (NULL == source_file || NULL == source_arr || NULL == p_lines_count) {
		printf("Error: argument pointer is NULL\n");
		return STATUS_FAILURE;
	}
	char* token;
	int top = -1;
	*p_lines_count = 0;
	// Extract the first token
	token = strtok(source_file, "\r\n");
	// loop through the string to extract all other tokens
	while (token != NULL) {
		top++;
		source_arr[top] = atoi(token);
		(*p_lines_count)++;
		token = strtok(NULL, "\r\n");
	}
	return STATUS_SUCCESS;
}

/// === calc_avg ===
/// Description: This function adds the current file's contribution to the toatl average. (per subject)
/// Parameters:
///		source_arr - int pointer to the source array with the relevant grades.
///		result_arr - int pointer to the result array. Will contain the sum of all subjects contribution to the average grade. 
///		weight - int. The relevant weight of the current subject.
///		arr_len -  int. The length of the arrays.
///	Returns: int value - 0 if succeeded, 1 if failed
int calc_avg(int* source_arr, int* result_arr, int weight, int arr_len) {
	if (NULL == source_arr || NULL == result_arr) {
		printf("Error: argument pointer is NULL\n");
		return STATUS_FAILURE;
	}
	int val = 0, i = 0;
	for (i = 0; i < arr_len; i++) {
		result_arr[i] += source_arr[i] * weight;
	}
	return STATUS_SUCCESS;
}

/// === convert_int_array_to_str ===
/// Description: This function converts int array into a string.
/// Parameters:
///		src_arr - int pointer to the int array to be converted.
///		p_lines_count -  int pointer that refers to the amount of lines in the file.	
///	Returns: con_arr - the string that was made by the function.
extern char* convert_int_array_to_str(int* src_arr, int* p_lines_count)
{
	if (NULL == src_arr || NULL == p_lines_count) {
		printf("Error: argument pointer is NULL\n");
		return NULL; // error
	}
	int mem_size = 0, tmp = 0;
	char* con_arr = NULL;
	int index = 0;
	for (int i = 0; i < *p_lines_count; i++) {
		tmp = snprintf(NULL, 0, "%d\r\n", src_arr[i]);
		mem_size += tmp;
		tmp = 0;
	}
	con_arr = (char*)malloc(sizeof(char) * mem_size + 1);
	if (NULL == con_arr) {
		printf("Error: pointer is NULL, malloc failed\n");
		return NULL; // error
	}
	for (int i = 0; i < *p_lines_count; i++) {
		index += snprintf(&con_arr[index], mem_size - index, "%d\r\n", src_arr[i]);
	}
	return con_arr;
}

/// === write_file ===
/// Description: This function writes the average grades file.
///				 It gets a string with the relevant grades that needs to be written.
/// Parameters:
///		path_name - the path for the file that needs to be read.
///		lines_count - int that refers to the amount of lines in the file.
///		result_arr - char pointer to a string that contains the average grades.
///	Returns: int value - 0 if succeeded, 1 if failed
extern DWORD write_file(LPSTR path_name, int lines_count, char* result_arr) {
	if (NULL == result_arr) {
		printf("Error: argument pointer is NULL\n");
		return STATUS_FAILURE;
	}

	HANDLE h_file = CreateFileA(path_name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h_file == INVALID_HANDLE_VALUE) {	// If failed to open file
		printf("Error: Failed to open file! The error code is %d\n", GetLastError());
		return STATUS_FAILURE;
	}
	DWORD dw_bytes_2_write = lines_count * sizeof(int), dw_bytes_written = 0;
	BOOL write_status = WriteFile(h_file, (void*)result_arr, dw_bytes_2_write, &dw_bytes_written, NULL);
	if (write_status == 0 && dw_bytes_2_write != dw_bytes_written) {
		printf("Error: File write partially or failed. The error code is %d\n", GetLastError());
		return STATUS_FAILURE;
	}
	CloseHandle(h_file);
	return STATUS_SUCCESS;
}