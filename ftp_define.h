
#ifndef _FTP_DEFINE_H
#define _FTP_DEFINE_H

#define STATE_FTP_GET_LASTFILE                     0x50
#define STATE_FTP_REMOVE_LASTFILE                  STATE_FTP_GET_LASTFILE    + 1
#define STATE_FTP_GPRS_CONFIG                      STATE_FTP_REMOVE_LASTFILE + 1
#define STATE_FTP_GPRS_CONNECT                     STATE_FTP_GPRS_CONFIG  + 1
#define STATE_FTP_PROFILE                          STATE_FTP_GPRS_CONNECT + 1
#define STATE_FTP_SERVER_SET                       STATE_FTP_PROFILE      + 1
#define STATE_FTP_PORT_SET                         STATE_FTP_SERVER_SET   + 1
#define STATE_FTP_USER_SET                         STATE_FTP_PORT_SET     + 1
#define STATE_FTP_PW_SET                           STATE_FTP_USER_SET     + 1
#define STATE_FTP_NAME_FILE                        STATE_FTP_PW_SET       + 1
#define STATE_FTP_PATH_FILE                        STATE_FTP_NAME_FILE    + 1
#define STATE_FTP_GETFILE                          STATE_FTP_PATH_FILE    + 1
#define STATE_FTP_CHECK_GET                        STATE_FTP_GETFILE      + 1 
#define STATE_FTP_CHECK_GET_SUCCESS                STATE_FTP_CHECK_GET    + 1
#define STATE_FTP_RETURN                           STATE_FTP_CHECK_GET_SUCCESS    + 1

typedef enum
{
  NONE_UPDATE,
  READY_UPDATE,
}status_update_t;

#endif /* _FTP_DEFINE_H */