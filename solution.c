/************************************************************************
//
//  DESCRIPTION:
//   Reads from two .csv files. One file contains the student IDs 
//   and names of 30 students. The other contatins the IDs and titles 
//   of 30 books and IDs of the students assigned to write a report 
//   for each book. Assumes a 1-1 relationship of books to students.
//
//   Outputs two .csv files. One file contains the merged data of the
//   two input files. The other file contains a subset of the merged
//   data - only rows which indicate that the student has not completed
//   their assigned report.
//
*************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct record {                 /* Data structure used to hold each record */
    int student_id;                 /* The Student ID */
    char last_name[20];             /* The Last Name */
    char first_name[20];            /* The First Name */
    int book_id;                    /* The Book ID */
    char title[50];                 /* The Book Title */
    char report_submitted[5];       /* YES/NO - If the Report has been submitted */
    struct record *next;
};

void readstring(FILE *, char *, int);
int loadbooks(struct record *, char *);
int loadstudents(struct record **, char *);
int loaddata(struct record **, char *, char *);
int writedata(struct record *, char *, char *, int);

/********************************************************************************
//
//  Function name: main
//
//  DESCRIPTION:   Combines the data from two .csv files into a linked list of 
//                 struct records. Then writes the data into one .csv file, and 
//                 a subset of the data into another .csv file.
//
//  Parameters:    argc (int) : The number of command-line arguments 
//                              this program was invoked with
//
//                 argv (char **) : A pointer to an array of character
//                                  strings that contain the arguments
//                                  this program was invoked with
//
//  Return values:  0 : If the files were successfully accessed
//                 -1 : If an error occured when accessing the files
//
********************************************************************************/

int main(int argc, char *argv[])
{
    int term_val; /* The return value */
    struct record *first; /* A pointer to the first record in the list */

    /* The  column names for the output*/
    char header[] = "Student_ID,Last_Name,First_Name,Book_ID,Book_Title,Report_Submitted";

    first = NULL;

    /* Create a linked list using the data from two .csv files */
    term_val = loaddata(&first, "students.csv", "book_report_assignments.csv");

    if (term_val == 0)
    {
        /* Write all data to a .csv file */
        term_val = writedata(first, "full_information.csv", header, 1);

        if (term_val == 0)
        {
            /* Write a subset of the data to a .csv file */
            term_val = writedata(first, "pending_reports.csv", header, 0);
        }
    }

    return term_val;
}

/******************************************************************************
//
//  Function name: loaddata
//
//  DESCRIPTION:   Loads two .csv files into a linked list of struct records.
//
//  Parameters:    firstadr (struct record **) : The address of the first
//                                               record in the list
//
//                 filename1 (char *) : The first file to be loaded. This
//                                      file will add records to the linked
//                                      list
//
//                 filename2 (char *) : The second file to be loaded. This
//                                      file will add more information to 
//                                      each record.
//
//  Return values:  0 : If the files were successfully accessed
//                 -1 : If at least one of the files could not be accessed
//
******************************************************************************/

int loaddata(struct record **firstadr, char *filename1, char *filename2)
{
    int val;

    /* Create the linked list using data from filename1 */
    val = loadstudents(firstadr, filename1);

    /* Add the reamining data to each record from filename2 */
    if (val == 0)
        val = loadbooks(*firstadr, filename2);

    return val;
}

/********************************************************************************
//
//  Function name: loadstudents
//
//  DESCRIPTION:   Reads a .csv file of student data consisting of
//                 the Student ID, Last Name, and First Name.
//                 The data is saved into a linked list of struct 
//                 records.
//
//  Parameters:    firstaddress (struct record **) : The address of the first
//                                                   record in the linked list
//
//                 filename (char *) : The name of the file to be read from
//
//  Return values:  0 : If the file was successfully accessed
//                 -1 : If the file could not be accessed
//
*********************************************************************************/

int loadstudents(struct record **firstaddress, char *filename)
{
    int id; 
    FILE *file;
    struct record *node, *previous, *current;
    char lastname[20], firstname[20], trash[1000];

    /* Open the file for reading and save its location to a pointer */
    file = fopen(filename, "r");

    /* If the file can be opened, begin reading */
    if (file != NULL)
    {
        /* Discard the first line of the file (the column headers) */
        fgets(trash, 1000, file);

        /* Loop until EOF */
        while (!feof(file))
        {
            /* Set current and previous to point to the beginning of the list */
            current = *firstaddress;
            previous = current;

            /* Read and save the student id into the variable id; dscard the comma */
            fscanf(file, "%d", &id);
            fgetc(file);

            /* Read and save the last name and first name into string arrays */
            readstring(file, lastname, 0);
            readstring(file, firstname, 1);

            /* Ensure that the first name ends as excpected */
            if (!isalpha(firstname[strlen(firstname) - 1]))
                firstname[strlen(firstname) - 1] = '\0';

            /* Allocate space for a new record */
            node = (struct record*) malloc(sizeof(struct record));

            /* If the new record is the first record to be added */
            if (*firstaddress == NULL)
            {
                /* Add the data to the new record */
                node->student_id = id;
                strcpy(node->last_name, lastname);
                strcpy(node->first_name, firstname);

                node->next = NULL;

                /* Point the 'pointer to the first record' to the new record */
                *firstaddress = node;
            }
            else /* If the new record is not the first to be added */
            {
                /* Move to the end of the list */
                while(current != NULL)
                {
                    previous = current;
                    current = current->next;
                }

                /* Add the data to the new record */
                node->student_id = id;
                strcpy(node->last_name, lastname);
                strcpy(node->first_name, firstname);

                node->next = NULL;
                previous->next = node;
            }
        }

        fclose(file);
    }

    return (file != NULL) ? 0 : -1;
}

/****************************************************************************
//
//  Function name: loadbooks
//
//  DESCRIPTION:   Reads data from a .csv file consisting of the
//                 Book ID, Book Title, Student ID, and Report Submitted.
//                 The Student ID is matched with a record in a linked 
//                 list, and the remaining information is used to complete
//                 the record.
//
//  Parameters:    first (struct record *) : A pointer to the first record
//                                           in the list
//
//                 filename (char *) : The name of the file
//
//  Return values:  0 : If the file was successfully accessed
//                 -1 : If the file could not be accessed
//
***************************************************************************/

int loadbooks(struct record *first, char *filename)
{
    FILE *file;
    int bookid, studentid;
    struct record *current;
    char submitted[5], title[50], trash[1000];

    /* Open the file for reading and save its location to a pointer */
    file = fopen(filename, "r");

    if (file != NULL)
    {
        /* Discard the first line */
        fgets(trash, 1000, file);

        while (!feof(file))
        {
            /* Point current to the first record in the list */
            current = first;

            /* Read and save the book id */
            fscanf(file, "%d", &bookid);
            fgetc(file);

            /* Read and save the book title */
            readstring(file, title, 0);

            /* Read and save the student id */
            fscanf(file, "%d", &studentid);
            fgetc(file);

            /* Read and save the yes/no indication of if the report has been submitted */
            readstring(file, submitted, 1);

            if (!isalpha(submitted[strlen(submitted) - 1]))
                submitted[strlen(submitted) - 1] = '\0';

            /* Find the record with the matching student id */
            while (studentid != current->student_id)
                current = current->next;

            /* Save the data to the record */
            current->book_id = bookid;
            strcpy(current->title, title);
            strcpy(current->report_submitted, submitted);
        }

        fclose(file);
    }

    return (file != NULL) ? 0 : -1;
}

/**********************************************************************
//
//  Function name: writedata
//
//  DESCRIPTION:   Either writes each record, from the linked list
//                 of struct records, to a .csv file, or writes the
//                 subset of the list (records with 'NO' in the 
//                 'Report Submitted' column') to a .csv file.
//
//  Parameters:    first (struct record *) : A pointer to the first
//                                           record in the list
//
//                 header (char *) : The column names
//
//                 mode (int) : 0 - if the subset of the list should
//                                  be written
//                              1 - if the entire list should be 
//                                  be written
//
//  Return values:  0 : If the file was successfully created
//                 -1 : If the file could not be created
//
**********************************************************************/

int writedata(struct record *first, char *filename, char *header, int mode)
{
    FILE *file;
    struct record *current;

    /* Open the file for writing, and save its location to a pointer */
    file = fopen(filename, "w");

    if (file != NULL)
    {
        /* Write the column names to the file */
        fprintf(file, "%s\n", header);

        /* Point current to the first record in the list */
        current = first;

        /* Until reaching the end of the list... */
        while (current != NULL)
        {
            if (mode) /* Write the current record to the file */
            {
                fprintf(file, "%d,%s,%s,%d,%s,%s\n", 
                    current->student_id, current->last_name, current->first_name,
                    current->book_id, current->title, current->report_submitted);
            }
            else /* Only write the current record if 'NO' is entered in the Report Submitted column */
            {
                if (!strcmp(current->report_submitted, "NO"))
                {
                    fprintf(file, "%d,%s,%s,%d,%s,%s\n", 
                        current->student_id, current->last_name, current->first_name,
                        current->book_id, current->title, current->report_submitted);
                }
            }

            /* Point to the next record */
            current = current->next;
        }

        fclose(file);
    }

    return (file != NULL) ? 0 : -1;
}

/******************************************************************
//
//  Function name: readstring
//
//  DESCRIPTION:   Reads a string from a file and saves it into 
//                 an array.
//
//  Parameters:    file (FILE *) : A pointer to the file to be 
//                                 read from
//
//                 string (char *) : A pointer to the array to
//                                   save the string
//
//                 mode (int) : 0 - If the string is followed by
//                                  a comma
//                              1 - If the string is at the end
//                                  of the line
//
//  Return values:  none
//
*****************************************************************/

void readstring(FILE *file, char *string, int mode)
{
    char c;

    if (mode)
    {
        /* Save all printable characters (not '\n') to the string array */
        while (isprint(c = fgetc(file)))
        {
            *string = c;
            ++string;
        }
    }
    else
    {
        /* Save all characters, up to and not including the comma, to the string array */
        while ((c = fgetc(file)) != ',')
        {
            *string = c;
            ++string;
        }
    }

    *string = '\0';
}
