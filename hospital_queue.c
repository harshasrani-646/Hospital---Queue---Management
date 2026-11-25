
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_LEN 50
#define CONDITION_LEN 60
#define FILE_NAME "records.txt"

typedef struct Patient {
    int id;
    char name[NAME_LEN];
    int age;
    char condition[CONDITION_LEN];
    int emergency;
    int status;
    struct Patient *nextQueue;
    struct Patient *nextRecord;
} Patient;

typedef struct {
    Patient *front;
    Patient *rear;
} Queue;

Queue normalQueue = {NULL, NULL};
Queue emergencyQueue = {NULL, NULL};
Patient *records = NULL;

void clearInput(void);
void readLine(char *buffer, int size);
int patientIdExists(int id);
Patient *findPatient(int id);
void enqueue(Queue *q, Patient *p);
Patient *dequeue(Queue *q);
void addToRecords(Patient *p);
void rebuildQueues(void);
void saveRecords(void);
void loadRecords(void);
void registerPatient(void);
void callNextPatient(void);
void displayQueue(void);
void searchPatient(void);
void displayAllRecords(void);
void cancelRegistration(void);
void displayStatistics(void);
void freeMemory(void);
void printQueue(const Queue *q, const char *title);
void removeFromQueue(Queue *q, Patient *target);

void clearInput(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

int patientIdExists(int id) {
    Patient *temp = records;

    while (temp != NULL) {
        if (temp->id == id)
            return 1;

        temp = temp->nextRecord;
    }

    return 0;
}

Patient *findPatient(int id) {
    Patient *temp = records;

    while (temp != NULL) {
        if (temp->id == id)
            return temp;

        temp = temp->nextRecord;
    }

    return NULL;
}

void enqueue(Queue *q, Patient *p) {
    p->nextQueue = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = p;
    } else {
        q->rear->nextQueue = p;
        q->rear = p;
    }
}

Patient *dequeue(Queue *q) {
    if (q->front == NULL)
        return NULL;

    Patient *removed = q->front;
    q->front = q->front->nextQueue;

    if (q->front == NULL)
        q->rear = NULL;

    removed->nextQueue = NULL;

    return removed;
}

void addToRecords(Patient *p) {
    p->nextRecord = NULL;

    if (records == NULL) {
        records = p;
    } else {
        Patient *temp = records;

        while (temp->nextRecord != NULL)
            temp = temp->nextRecord;

        temp->nextRecord = p;
    }
}

void rebuildQueues(void) {
    Patient *temp = records;

    normalQueue.front = normalQueue.rear = NULL;
    emergencyQueue.front = emergencyQueue.rear = NULL;

    while (temp != NULL) {
        temp->nextQueue = NULL;

        if (temp->status == 0) {
            if (temp->emergency)
                enqueue(&emergencyQueue, temp);
            else
                enqueue(&normalQueue, temp);
        }

        temp = temp->nextRecord;
    }
}

void saveRecords(void) {
    FILE *fp = fopen(FILE_NAME, "w");

    if (fp == NULL) {
        printf("Warning: Could not save records to file.\n");
        return;
    }

    Patient *temp = records;

    while (temp != NULL) {
        fprintf(fp, "%d|%s|%d|%s|%d|%d\n",
                temp->id,
                temp->name,
                temp->age,
                temp->condition,
                temp->emergency,
                temp->status);

        temp = temp->nextRecord;
    }

    fclose(fp);
}

void loadRecords(void) {
    FILE *fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
        return;

    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL) {
        Patient *p = (Patient *)malloc(sizeof(Patient));

        if (p == NULL) {
            printf("Not enough memory to load records.\n");
            break;
        }

        if (sscanf(line, "%d|%49[^|]|%d|%59[^|]|%d|%d",
                   &p->id,
                   p->name,
                   &p->age,
                   p->condition,
                   &p->emergency,
                   &p->status) == 6) {

            p->nextQueue = NULL;
            p->nextRecord = NULL;

            addToRecords(p);
        } else {
            free(p);
        }
    }

    fclose(fp);
    rebuildQueues();
}

void registerPatient(void) {
    Patient *p = (Patient *)malloc(sizeof(Patient));

    if (p == NULL) {
        printf("Memory allocation failed. Patient was not registered.\n");
        return;
    }

    printf("\n--- Patient Registration ---\n");

    printf("Enter patient ID: ");

    if (scanf("%d", &p->id) != 1 || p->id <= 0) {
        printf("Invalid patient ID.\n");
        clearInput();
        free(p);
        return;
    }

    clearInput();

    if (patientIdExists(p->id)) {
        printf("This patient ID already exists. Use a unique ID.\n");
        free(p);
        return;
    }

    printf("Enter patient name: ");
    readLine(p->name, NAME_LEN);

    if (strlen(p->name) == 0 || strchr(p->name, '|') != NULL) {
        printf("Name cannot be empty or contain the | character.\n");
        free(p);
        return;
    }

    printf("Enter age: ");

    if (scanf("%d", &p->age) != 1 || p->age < 0 || p->age > 120) {
        printf("Invalid age.\n");
        clearInput();
        free(p);
        return;
    }

    clearInput();

    printf("Enter condition/reason for visit: ");
    readLine(p->condition, CONDITION_LEN);

    if (strlen(p->condition) == 0 ||
        strchr(p->condition, '|') != NULL) {
        printf("Condition cannot be empty or contain the | character.\n");
        free(p);
        return;
    }

    printf("Patient type (1 = Emergency, 0 = Normal): ");

    if (scanf("%d", &p->emergency) != 1 ||
        (p->emergency != 0 && p->emergency != 1)) {
        printf("Invalid patient type.\n");
        clearInput();
        free(p);
        return;
    }

    clearInput();

    p->status = 0;
    p->nextQueue = NULL;
    p->nextRecord = NULL;

    addToRecords(p);

    if (p->emergency)
        enqueue(&emergencyQueue, p);
    else
        enqueue(&normalQueue, p);

    saveRecords();

    printf("\nPatient registered successfully.\n");
    printf("Queue type: %s\n",
           p->emergency ? "Emergency" : "Normal");
}

void callNextPatient(void) {
    Patient *p = dequeue(&emergencyQueue);

    if (p == NULL)
        p = dequeue(&normalQueue);

    if (p == NULL) {
        printf("\nNo patients are waiting.\n");
        return;
    }

    p->status = 1;

    saveRecords();

    printf("\n--- Next Patient ---\n");
    printf("Patient ID : %d\n", p->id);
    printf("Name       : %s\n", p->name);
    printf("Age        : %d\n", p->age);
    printf("Condition  : %s\n", p->condition);
    printf("Category   : %s\n",
           p->emergency ? "Emergency" : "Normal");
    printf("Status     : Consultation started/completed in this demo\n");
}

void printQueue(const Queue *q, const char *title) {
    Patient *temp = q->front;

    printf("\n%s\n", title);
    printf("--------------------------------------------------\n");

    if (temp == NULL) {
        printf("No patients in this queue.\n");
        return;
    }

    printf("%-8s %-20s %-6s %-12s\n",
           "ID", "Name", "Age", "Category");

    while (temp != NULL) {
        printf("%-8d %-20.20s %-6d %-12s\n",
               temp->id,
               temp->name,
               temp->age,
               temp->emergency ? "Emergency" : "Normal");

        temp = temp->nextQueue;
    }
}

void displayQueue(void) {
    printf("\n========== CURRENT WAITING QUEUE ==========\n");

    printQueue(&emergencyQueue,
               "EMERGENCY QUEUE (served first)");

    printQueue(&normalQueue,
               "NORMAL QUEUE (FIFO order)");
}

void searchPatient(void) {
    int id;

    printf("\nEnter patient ID to search: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clearInput();
        return;
    }

    clearInput();

    Patient *p = findPatient(id);

    if (p == NULL) {
        printf("Patient record not found.\n");
        return;
    }

    printf("\n--- Patient Details ---\n");
    printf("ID        : %d\n", p->id);
    printf("Name      : %s\n", p->name);
    printf("Age       : %d\n", p->age);
    printf("Condition : %s\n", p->condition);

    printf("Category  : %s\n",
           p->emergency ? "Emergency" : "Normal");

    printf("Status    : %s\n",
           p->status == 0 ? "Waiting" :
           (p->status == 1 ? "Completed" : "Cancelled"));
}

void displayAllRecords(void) {
    Patient *temp = records;

    printf("\n================ ALL PATIENT RECORDS ================\n");

    if (temp == NULL) {
        printf("No patient records available.\n");
        return;
    }

    printf("%-8s %-18s %-5s %-14s %-12s %-12s\n",
           "ID", "Name", "Age", "Category", "Status", "Condition");

    printf("--------------------------------------------------------------------------\n");

    while (temp != NULL) {
        const char *statusText =
            temp->status == 0 ? "Waiting" :
            (temp->status == 1 ? "Completed" : "Cancelled");

        printf("%-8d %-18.18s %-5d %-14s %-12s %-12.12s\n",
               temp->id,
               temp->name,
               temp->age,
               temp->emergency ? "Emergency" : "Normal",
               statusText,
               temp->condition);

        temp = temp->nextRecord;
    }
}

void removeFromQueue(Queue *q, Patient *target) {
    Patient *current = q->front;
    Patient *previous = NULL;

    while (current != NULL) {
        if (current == target) {

            if (previous == NULL)
                q->front = current->nextQueue;
            else
                previous->nextQueue = current->nextQueue;

            if (q->rear == current)
                q->rear = previous;

            current->nextQueue = NULL;
            return;
        }

        previous = current;
        current = current->nextQueue;
    }
}

void cancelRegistration(void) {
    int id;

    printf("\nEnter patient ID to cancel: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clearInput();
        return;
    }

    clearInput();

    Patient *p = findPatient(id);

    if (p == NULL) {
        printf("Patient record not found.\n");
        return;
    }

    if (p->status != 0) {
        printf("Only waiting patients can be cancelled.\n");
        return;
    }

    if (p->emergency)
        removeFromQueue(&emergencyQueue, p);
    else
        removeFromQueue(&normalQueue, p);

    p->status = 2;

    saveRecords();

    printf("Patient registration cancelled successfully.\n");
}

void displayStatistics(void) {
    int total = 0, waiting = 0;
    int completed = 0, cancelled = 0;
    int emergency = 0, normal = 0;

    Patient *temp = records;

    while (temp != NULL) {
        total++;

        if (temp->status == 0)
            waiting++;
        else if (temp->status == 1)
            completed++;
        else if (temp->status == 2)
            cancelled++;

        if (temp->emergency)
            emergency++;
        else
            normal++;

        temp = temp->nextRecord;
    }

    printf("\n========== HOSPITAL STATISTICS ==========\n");
    printf("Total registered patients : %d\n", total);
    printf("Currently waiting         : %d\n", waiting);
    printf("Consultation completed    : %d\n", completed);
    printf("Cancelled registrations   : %d\n", cancelled);
    printf("Emergency registrations   : %d\n", emergency);
    printf("Normal registrations      : %d\n", normal);
}

void freeMemory(void) {
    Patient *temp = records;

    while (temp != NULL) {
        Patient *next = temp->nextRecord;
        free(temp);
        temp = next;
    }

    records = NULL;

    normalQueue.front = normalQueue.rear = NULL;
    emergencyQueue.front = emergencyQueue.rear = NULL;
}

int main(void) {
    int choice;

    loadRecords();

    do {
        printf("\n===============================================\n");
        printf("       HOSPITAL QUEUE MANAGEMENT SYSTEM\n");
        printf("===============================================\n");

        printf("1. Register New Patient\n");
        printf("2. Call Next Patient\n");
        printf("3. View Waiting Queue\n");
        printf("4. Search Patient by ID\n");
        printf("5. Display All Patient Records\n");
        printf("6. Cancel Patient Registration\n");
        printf("7. Display Statistics\n");
        printf("8. Save Records\n");
        printf("0. Exit\n");

        printf("-----------------------------------------------\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Please enter a valid menu number.\n");
            clearInput();
            continue;
        }

        clearInput();

        switch (choice) {
            case 1:
                registerPatient();
                break;

            case 2:
                callNextPatient();
                break;

            case 3:
                displayQueue();
                break;

            case 4:
                searchPatient();
                break;

            case 5:
                displayAllRecords();
                break;

            case 6:
                cancelRegistration();
                break;

            case 7:
                displayStatistics();
                break;

            case 8:
                saveRecords();
                printf("Records saved.\n");
                break;

            case 0:
                saveRecords();
                printf("Records saved. Thank you for using the system.\n");
                break;

            default:
                printf("Invalid choice. Please try again.\n");
        }

    } while (choice != 0);

    freeMemory();

    return 0;
}
