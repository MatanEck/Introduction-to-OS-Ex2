/**********************************************

Authors:
Matan Eckhaus Moyal
Dvir Katz

Project : Ex2 - Grades

* *********************************************/

/// Description: This is the main module for the project Grades
///				 This program will calculate the average grades for BAGROOT tests, in diferent schools.

// Includes:
#include "SourceGrades.h"


/**************************************** Main Function Summary: ****************************************
/// Description: This program will calculate the average grades for BAGROOT tests, in different schools.
///				 The main function will create a folder named "Results", that will eventually contain the average grades.
///				 The program will create a thread for each school. Every thread will calculate the average grades accordingly.
///
/// Parameters:
///		argc - int. the number of input arguments - should accept 5 arguments.
///		argv - char pointer to a list of input arguments - number of schools, weight of Real, Human, English classes
///			   and the weight of School Evaluation
///	Returns: int value - 0 if succeeded, 1 if failed
*********************************************************************************************************/
int main(int argc, char* argv[])
{
	// Check if the input is correct (5  arguments)
	if (6 != argc)
	{
		printf("Error: Wrong number of input arguments!\n");
		return STATUS_FAILURE;
	}
	// creating Results folder in the current directory
	LPCSTR dir_name = "Debug/Results"; // for the current directory
	BOOL Results = CreateDirectoryA(dir_name, NULL);
	if (Results == 0) {		// If failed to create the folder
		printf("Error: Failed to create directory! The error code is %d\n", GetLastError());
		return STATUS_FAILURE;
	}
	int num_of_schools = atoi(argv[1]);
	HANDLE* p_thread_handles = (HANDLE*)malloc(MAX_THREADS * sizeof(HANDLE));
	DWORD* p_thread_id = (DWORD*)malloc(MAX_THREADS * sizeof(DWORD));
	if (NULL == p_thread_handles || NULL == p_thread_id) {
		printf("Error when allocating memory\n");
		return STATUS_FAILURE;
	}
	DWORD finished_thread_index, exit_code;
	BOOL ret_val;
	size_t i;
	grade_params* p_thread_params;
	p_thread_params = (grade_params*)malloc(sizeof(grade_params) * MAX_THREADS);
	if (NULL == p_thread_params)
	{
		printf("Error when allocating memory\n");
		return STATUS_FAILURE;
	}

	/* Prepare parameters for thread */
	for (i = 0; i < MAX_THREADS; i++) {
		p_thread_params[i].index = i;
		p_thread_params[i].real = atoi(argv[2]);
		p_thread_params[i].human = atoi(argv[3]);
		p_thread_params[i].english = atoi(argv[4]);
		p_thread_params[i].eval = atoi(argv[5]);
	}
	if (num_of_schools < MAX_THREADS)
	{
		for (i = 0; i < num_of_schools; i++)
		{
			p_thread_handles[i] = create_thread_grades(thread_main, &p_thread_params[i], &p_thread_id[i]);
			if (p_thread_handles[i] == NULL) {
				printf("Error when creating thread\n");
				return STATUS_FAILURE;
			}
		}
	}
	else
	{
		for (i = 0; i < MAX_THREADS; i++) // Condition of less than 10 threads
		{
			p_thread_handles[i] = create_thread_grades(thread_main, &p_thread_params[i], &p_thread_id[i]);
			if (p_thread_handles[i] == NULL) {
				printf("Error when creating thread\n");
				return STATUS_FAILURE;
			}
		}
		for (i; i < num_of_schools; i++) // Conditions for testing parallelism for more than 10 threads
		{
			finished_thread_index = WaitForMultipleObjects(MAX_THREADS, p_thread_handles, FALSE, DELAY_TIME); // return the index of the finished thread
			CloseHandle(p_thread_handles[finished_thread_index]);
			p_thread_params[finished_thread_index].index = i;
			p_thread_params[finished_thread_index].real = atoi(argv[2]);
			p_thread_params[finished_thread_index].human = atoi(argv[3]);
			p_thread_params[finished_thread_index].english = atoi(argv[4]);
			p_thread_params[finished_thread_index].eval = atoi(argv[5]);
			p_thread_handles[finished_thread_index] = create_thread_grades(thread_main, &p_thread_params[finished_thread_index], &p_thread_id[finished_thread_index]); // Create the thread on the finished thread
			if (p_thread_handles[finished_thread_index] == NULL) {
				printf("Error when creating thread\n");
				return STATUS_FAILURE;
			}
		}
		finished_thread_index = WaitForMultipleObjects(MAX_THREADS, p_thread_handles, TRUE, DELAY_TIME); // Ensure that all threads finished
	}

	/* Close thread handles */
	for (i = 0; i < MAX_THREADS; i++) {
		/* Check the ret_val returned by Thread */
		ret_val = GetExitCodeThread(p_thread_handles[i], &exit_code);
		if (0 == ret_val) {
			printf("Error when getting thread exit code\n");
		}
		ret_val = CloseHandle(p_thread_handles[i]);
		if (FALSE == ret_val) {
			printf("Error when closing thread: %d\n", GetLastError());
			return STATUS_FAILURE;
		}
	}
	free(p_thread_handles);
	free(p_thread_params);
	free(p_thread_id);
	return STATUS_SUCCESS;
}

/// === create_thread_grades ===
/// Description: This function creates thread that should run the thread_main function.
/// Parameters:
///		p_start_routine - a pointer to the function the thread runs (thread_main).
///		p_thread_parameters - void pointer to with the relevant parameters that the function needs.
///		p_thread_id -  int pointer for the thread's ID
///	Returns: int value - 0 if succeeded, 1 if failed
static HANDLE create_thread_grades(LPTHREAD_START_ROUTINE p_start_routine,
	LPVOID p_thread_parameters,
	LPDWORD p_thread_id)
{
	HANDLE thread_handle;

	if (NULL == p_start_routine) {
		printf("Error when creating a thread. Received null pointer\n");
		return NULL;
	}

	if (NULL == p_thread_id) {
		printf("Error when creating a thread. Received null pointer\n");
		return NULL;
	}

	thread_handle = CreateThread(
		NULL,                /*  default security attributes */
		0,                   /*  use default stack size */
		p_start_routine,     /*  thread function */
		p_thread_parameters, /*  argument to thread function */
		0,                   /*  use default creation flags */
		p_thread_id);        /*  returns the thread identifier */

	return thread_handle;
}