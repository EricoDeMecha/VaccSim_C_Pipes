#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include<signal.h>


/* Maximum name length */
#define NAME_LENGTH 30

/* File name of the data records */
#define FILE_NAME "applicants.dat"
#define BUSMAX  5
#define BUSBUSMAX 10
#define VACCINATED_FNAME  "vaccinated.dat"

/* Structure for each applicant data */
typedef struct s_applicant {
    char name[NAME_LENGTH];  /* applicant name */
    int date_of_birth;
    unsigned long phone;
    char is_free;
} t_applicant;

/* Structure for all applicant list */
typedef struct s_list {
    t_applicant *data;  /* pointer to the applicant data items */
    int capacity;        /* allocated size for items array */
    int size;            /* actual number of items at the list */
} t_list;


/* Function prototypes */

/* Reading one applicant data from the user */
/* Post: t_applicant appl is filled with data entered by the user */
void read_applicant(t_applicant * appl);

/* Printing one applicant data to the screen */
void display_applicant(const t_applicant * appl);

/* Initialization of the applicant list */
/* Post: list is initializes to capacity 10, size 0 */
void init_applicant_list(t_list * list);

/* Free memory allocated by applicant list data items (not the list record itself */
void clear_applicant_list(t_list * list);

/* Printing applicant list to the screen */
void display_applicant_list(const t_list * list);

/* Load applicant list from the given file name
 * Pre: list should be previously initialized.
 * Post: all records from the file will be added at the end of the list.
 */
void load_applicant_list(t_list * list, const char * fname);

/* Save applicant list to the given file name */
void save_applicant_list(const t_list * list, const char * fname);

/* Searches for applicant with given name at the list
 * Return index of proper record if proper name found, -1 otherwise */
int find_applicant(const t_list * list, const char * name);

/* Add one applicant record to the end of the list */
/* Reallocates memory (increase capacity by 10) if list is full */
void add_applicant(t_list * list, t_applicant * appl);

/* Delete applicant record at given index from the list */
/* For valid index (>=0 and < list->size) shifts all ascending records, decreases list size*/
/* If index is invalid (< 0 or > list->size) do nothing  */
void delete_applicant(t_list * list, int index);

/* Display main menu, get user entry, validate user entry, returns valid menu number */
int get_menu_choice();
/*Query load bus response*/
bool get_load_bus_resp(const char* stmt);
/*Save  vaccination list*/
void save_vaccination_list(const t_list * list, const char * fname);
int main()
{
    pid_t pr1 , pr2;// two processes for the two busses
    int pip[2];//

    if(pipe(pip) == -1){
        perror("\n Failed to initialize pipe \n");
    }

    t_applicant appl;
    t_list list; /* applicant list */
    int choice;  /* menu choice */
    char name[NAME_LENGTH];
    int index;

    /* Initialize list */
    init_applicant_list(&list);

    /* Load applicant list from the file at startup */
    load_applicant_list(&list, FILE_NAME);

    /*Check if to load bus here*/
    /*Morning before daily data entry */

    if(list.size > BUSMAX){
        printf("\n Applicants are %d. Quorum to vaccinate has been reached\n", list.size);
        if(list.size >= BUSBUSMAX){
            const char* stmt1 = "Would you like to load the two buses(y/n):";
            if(get_load_bus_resp(stmt1)){
                pr1 = fork();
                if(pr1 < 0){
                    perror("pr1 fork() failed\n");
                    exit(EXIT_FAILURE);
                }else if(pr1 == 0){
                    /* Bus 1 started */
                    printf("\nFIGHT's UP\n");// signal before loading
                    fflush(stdout);
                    close(pip[0]); // close read end of the pipe
                    write(pip[1],&list, (list.size * sizeof(appl)));// write the data through the pipe
                    exit(EXIT_SUCCESS);
                }else if(pr1 > 0){
                    wait(NULL); // wait for the child process to finish
                    close(pip[1]);
                    t_list vac_list;
                    init_applicant_list(&vac_list);
                    read(pip[0], &vac_list, (list.size * sizeof(appl)));
                    printf("List  to be Vaccinated\n");
                    fflush(stdout);
                    for(int i = 0; i < vac_list.size; i++){
                        display_applicant(&vac_list.data[i]);
                    }
                    save_vaccination_list(&vac_list, VACCINATED_FNAME);
                    printf("VACCINATED!!!\n");
                    fflush(stdout);
                    close(pip[0]);// close the pipe
                }
            }
        }else{
            const char* stmt2 = "Would you like to load only 1 bus(y/n):";
            if(get_load_bus_resp(stmt2)){

                /*load  one bus only: max 5*/
                pr2 = fork();
                if(pr2 < 0){
                    perror("pr1 fork() failed\n");
                    exit(EXIT_FAILURE);
                }else if(pr2 == 0){
                    /* Bus 1 started */
                    printf("\nFIGHT's UP\n");// signal before loading
                    fflush(stdout);
                    close(pip[0]); // close read end of the pipe
                    write(pip[1],&list, (list.size * sizeof(appl)));// write the data through the pipe
                    exit(EXIT_SUCCESS);
                }else if(pr2 > 0){
                    wait(NULL); // wait for the child process to finish
                    close(pip[1]);
                    t_list vac_list;
                    init_applicant_list(&vac_list);
                    read(pip[0], &vac_list, (list.size * sizeof(appl)));
                    printf("List  to be Vaccinated\n");
                    fflush(stdout);
                    for(int i = 0; i < vac_list.size; i++){
                        display_applicant(&vac_list.data[i]);
                    }
                    save_vaccination_list(&vac_list, VACCINATED_FNAME);
                    printf("VACCINATED!!!\n");
                    fflush(stdout);
                    close(pip[0]);// close the pipe
                }
            }
        }
    }



    do {
        /* display menu and get user choice */
        choice = get_menu_choice();

        switch (choice) {
            case 1: /* add new record */
                printf("\n\n--- ADD NEW APPLICANT ---\n");
                read_applicant(&appl);
                add_applicant(&list, &appl);
                break;
            case 2: /* modify record */
                printf("\n\n--- MODIFY APPLICANT ---\n");
                printf("Please enter applicant name: ");
                scanf("%[^\n]%*c", name);
                /* looking for applicant by name */
                index = find_applicant(&list, name);
                if (index == -1) {
                    printf("Applicant name %s not found!\n\n", name);
                }
                else {
                    /* Display applicant record */
                    display_applicant(&list.data[index]);
                    printf("\nEnter new record data\n");
                    /* Enter new data record */
                    read_applicant(&list.data[index]);
                }
                break;
            case 3: /* delete record */
                printf("\n\n--- DELETE APPLICANT ---\n");
                printf("Please enter applicant name: ");
                scanf("%[^\n]%*c", name);
                /* looking for applicant by name */
                index = find_applicant(&list, name);
                if (index == -1) {
                    printf("Applicant name %s not found!\n\n", name);
                }
                else {
                    printf("Applicant found.\n");
                    display_applicant(&list.data[index]);
                    /* Delete applicant with given name */
                    delete_applicant(&list, index);
                    printf("Applicant deleted.\n");
                }
                break;
            case 4: /* display all list */
                printf("\n\n--- DISPLAY LIST ---\n");
                display_applicant_list(&list);
                break;
            default:
                break;
        }
    } while (choice != 0);

    /* Save list into the file before exiting */
    save_applicant_list(&list, FILE_NAME);

    /* clear allocated memory */
    clear_applicant_list(&list);
    return 0;
}


/* Function implementations */

/* Reading one applicant data from the user */
/* Post: t_applicant appl is filled with data entered by the user */
void read_applicant(t_applicant * appl) {
    char ch;

    /* do nothing if appl is null */
    if (appl == NULL) return;

    printf("Please enter applicant name: ");
    scanf("%[^\n]%*c", appl->name);
    printf("Year of birth : ");
    scanf("%d", &(appl->date_of_birth));
    printf("Phone number  : ");
    scanf("%lu", &(appl->phone));
    /* skip rest of the line */
    while ((ch = getc(stdin)) && ch != '\n') {}

    do {
        printf("Would you like to get it free? (y/n) : ");
        scanf("%c", &(appl->is_free));

        /* skip rest of the line */
        while ((ch = getc(stdin)) && ch != '\n') {}

    } while (!(appl->is_free == 'y' || appl->is_free == 'n'));

}

/*Get  load  bus response */
bool get_load_bus_resp(const char* stmt){
    printf("\n %s >", stmt);
    char resp;
    scanf(" %c", &resp);
    if(resp == 'Y' || resp == 'y'){
        return  true;
    }else if(resp == 'N' || resp == 'n'){
        return  false;
    }else{
        printf("\n Wrong response  \n");
        return  false;
    }
}
/* Printing one applicant data to the screen */
void display_applicant(const t_applicant * appl) {
    printf("Name          : %s\n", appl->name);
    printf("Year of birth : %d\n", appl->date_of_birth);
    printf("Phone number  : %lu\n", appl->phone);
    printf("Free vaccine  :");
    if (appl->is_free == 'y')
        printf(" yes\n");
    else if (appl->is_free == 'n')
        printf(" no\n");
    else
        printf(" ?\n"); /* error field value */
}

/* Initialization of the applicant list */
/* Post: list is initializes to capacity 10, size 0 */
void init_applicant_list(t_list * list) {
    /* do nothing if list is null */
    if (list == NULL) return;

    list->capacity = 1;
    list->size = 0;
    list->data = (t_applicant*) malloc (sizeof(t_applicant) * (list->capacity));
}

/* Add one applicant record to the end of the list */
/* Reallocates memory (increase capacity by 10) if list is full */
void add_applicant(t_list * list, t_applicant * appl) {
    t_applicant * old_data;
    int i;

    /* do nothing if appl is null or list is null */
    if (appl == NULL || list == NULL) return;

    /* If list is full */
    if (list->size == list->capacity) {
        /* save pointer of old data items */
        old_data = list->data;

        /* allocate new memory - increase capacity by 10 */
        list->capacity += 10;
        list->data = (t_applicant*) malloc (sizeof(t_applicant) * (list->capacity));

        /* copy all existing applicant data records */
        for (i = 0; i < list->size; i++) {
            strcpy(list->data[i].name, old_data[i].name);
            list->data[i].date_of_birth = old_data[i].date_of_birth;
            list->data[i].phone = old_data[i].phone;
            list->data[i].is_free = old_data[i].is_free;
        }

        /* free memory allocated by old data items */
        free(old_data);
    }

    /* copy applicant data at the end of the list */
    strcpy(list->data[list->size].name, appl->name);
    list->data[list->size].date_of_birth = appl->date_of_birth;
    list->data[list->size].phone = appl->phone;
    list->data[list->size].is_free = appl->is_free;

    /* increase list size */
    list->size++;
}

/* Searches for applicant with given name at the list
 * Return index of proper record if proper name found, -1 otherwise */
int find_applicant(const t_list * list, const char * name) {
    int i;

    if (list == NULL)
        return -1;

    for (i = 0; i < list->size; i++) {
        if (strcmp(list->data[i].name, name) == 0)
            return i;       /* applicant name found */
    }
    return -1; /* name not found */
}

/* Printing applicant list to the screen */
void display_applicant_list(const t_list * list) {
    int i;

     /* do nothing if list is null */
    if (list == NULL) return;

    /* print table header */
    printf("----------------------------------------------------------------------\n");
    printf("%-30s %15s %15s %s\n", "Name", "Year of birth", "Phone", "Is Free");
    printf("----------------------------------------------------------------------\n");

    /* display list data */
    for (i = 0; i < list->size; i++) {
        printf("%-30s %15d %15lu", list->data[i].name, list->data[i].date_of_birth, list->data[i].phone);
        if (list->data[i].is_free == 'y')
            printf("   yes\n");
        else if (list->data[i].is_free == 'n')
            printf("    no\n");
        else
            printf("    ? \n");
    }
    printf("----------------------------------------------------------------------\n\n");
}

/* Free memory allocated by applicant list data items (not the list record itself */
void clear_applicant_list(t_list * list) {
    if (list == NULL) return;

    free(list->data);
    list->capacity = 0;
    list->size = 0;
}

/* Save applicant list to the given file name */
void save_applicant_list(const t_list * list, const char * fname) {
    FILE * fp;
    int i;

    /* do nothing if list is null */
    if (list == NULL) return;

    /* Try to open file with given name for writing */
    fp = fopen(fname, "w");
    if (!fp) {
        printf("Can't open file %s for writing\n!", fname);
        return;
    }

    /* save applicant data list: one record per line separated by commas */
    for (i = 0; i < list->size; i++) {
        fprintf(fp,"%s,%d,%lu,%c\n", list->data[i].name, list->data[i].date_of_birth,
                list->data[i].phone, list->data[i].is_free);
    }

    /* close resulting file */
    fclose(fp);
}


/* Save applicant list to the given file name */
void save_vaccination_list(const t_list * list, const char * fname) {
    FILE * fp;
    int i;

    /* do nothing if list is null */
    if (list == NULL) return;

    /* Try to open file with given name for writing */
    fp = fopen(fname, "w");
    if (!fp) {
        printf("Can't open file %s for writing\n!", fname);
        return;
    }

    /* save vaccination data list: one record per line separated by commas */
    const  char* vaccinated = "VACCINATED";
    for (i = 0; i < list->size; i++) {
        fprintf(fp,"%s,%d,%lu,%c,%s\n", list->data[i].name, list->data[i].date_of_birth,
                list->data[i].phone, list->data[i].is_free, vaccinated);
    }

    /* close resulting file */
    fclose(fp);
}

/* Load applicant list from the given file name
 * Pre: list should be previously initialized.
 * Post: all records from the file will be added at the end of the list.
 */
void load_applicant_list(t_list * list, const char * fname) {
    FILE * fp;
    t_applicant appl;
    char ch;

    /* do nothing if list pointer is null */
    if (list == NULL) return;

    /* Try to open file with given name for reading */
    fp = fopen(fname, "r");
    if (!fp) {
        printf("Can't open file %s for reading\n!", fname);
        return;
    }

    /* load applicant data list: one record per line separated by commas */
    while (fscanf(fp, "%[^,]%*c", appl.name) == 1 && !feof(fp)) {
        fscanf(fp,"%d", &appl.date_of_birth);
        fscanf(fp,"%c", &ch); /* skip ',' */
        fscanf(fp,"%lu", &appl.phone);
        fscanf(fp,"%c", &ch); /* skip ',' */
        fscanf(fp,"%c", &appl.is_free);
        /* skip '\n' */
        while (!feof(fp) && ((ch = fgetc(fp)) != '\n')) {}

        /* Add applicant at the end of the list */
        add_applicant(list, &appl);
    }

    /* close source file */
    fclose(fp);
}

/* Delete applicant record at given index from the list */
/* For valid index (>=0 and < list->size) shifts all ascending records, decreases list size*/
/* If index is invalid (< 0 or > list->size) do nothing  */
void delete_applicant(t_list * list, int index) {
    int i;

    /* return if index is invalid or list is NULL */
    if (list == NULL || index < 0 || index >= list->size)
        return;

    /* shift all records after index by one position to the left */
    for (i = index; i < list->size - 1; i++) {
        strcpy(list->data[i].name, list->data[i + 1].name);
        list->data[i].date_of_birth = list->data[i + 1].date_of_birth;
        list->data[i].phone = list->data[i + 1].phone;
        list->data[i].is_free = list->data[i + 1].is_free;
    }

    /* update list size */
    list->size--;
}

/* Display main menu, get user entry, validate user entry, returns valid menu number */
int get_menu_choice() {
    int choice, ch;

    printf("\n\n-----------------\n");
    printf("REGISTRATION MENU\n");
    printf("1. Add new record\n");
    printf("2. Modify record\n");
    printf("3. Delete record\n");
    printf("4. Display list\n");
    printf("0. Exit program\n");

    printf("> ");
    scanf("%d", &choice);
    while (choice < 0 || choice > 4) {
        printf("Invalid choice. Reenter please > ");
        scanf("%d", &choice);
    }
    /* skip '\n' */
    while ((ch = fgetc(stdin)) != '\n') {}

    return choice;
}

