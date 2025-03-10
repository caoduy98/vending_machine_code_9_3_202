
#include "platform.h"
#include "ff.h"
#include "error_logging.h"


static FRESULT OpenForAppend(FIL* fp, const char* path)
{
    FRESULT fr;

    /* Opens an existing file. If not exist, creates a new file. */
    fr = f_open(fp, path, FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK)
    {
        /* Seek to end of the file to append data */
        fr = f_lseek(fp, f_size(fp));
        if (fr != FR_OK)
        {
            f_close(fp);
        }
    }
    return fr;
}


void InitErrorLogging(const char* filePath)
{

}


void SaveErrorLog(error_type_t type, uint32_t error)
{
    uint32_t buffer[5];
    uint32_t bw = 0;
    FIL file;

    if (OpenForAppend(&file, "error_logging.txt") != FR_OK)
    {
        Dbg_Println("Error to open error_logging.txt file for write");
        return;
    }

    buffer[0] = (uint32_t)type;
    buffer[1] = error;
    buffer[2] = year;
    buffer[3] = sales;
    if (f_write(&file, (uint8_t*)buffer, 16, &bw) != FR_OK)
    {
        Dbg_Println("Error to write to error_logging.txt file");
        f_close(&file);
        return;
    }
    if (bw != 16)
    {
        Dbg_Println("The written bytes are incorrect");
        f_close(&file);
        return;
    }
    if (f_close(&file) != FR_OK)
    {
        Dbg_Println("Error to close the error_logging.txt file");
        return;
    }
}


uint32_t GetTotalLog()
{

}


void GetErrorLog(uint32_t index, error_log_t* errorInfo)
{

}


void ClearLogging()
{

}


