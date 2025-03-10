#ifndef ERROR_LOGGING_H
#define ERROR_LOGGING_H

typedef enum
{
    ERROR_TYPE_NV11 = 0,
    ERROR_TYPE_TEMPERATURE = 1,
    ERROR_TYPE_MOTOR = 2,
    ERROR_TYPE_DROP_SENSOR = 3,
} error_type_t;


typedef struct
{
    error_type_t   type;
    uint32_t        error;
} error_log_t;

void InitErrorLogging(const char* filePath);
void SaveErrorLog(error_type_t type, uint32_t error);
uint32_t GetTotalLog();
void GetErrorLog(uint32_t index, error_log_t* errorInfo);
void ClearLogging();

#endif  /* ERROR_LOGGING_H */
