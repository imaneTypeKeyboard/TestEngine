

#include "Log.h"


#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <sys/types.h>

Log * Log::m_pInstance = NULL;

#define MAX_QUERY_LEN 1024

Log::Log() :
    /* raLogfile(NULL),*/ logfile(NULL), gmLogfile(NULL), charLogfile(NULL),
    /* dberLogfile(NULL),*/ chatLogfile(NULL), /*arenaLogFile(NULL), sqlLogFile(NULL), sqlDevLogFile(NULL), DollarLogFile(NULL),*/
    m_gmlog_per_account(false), /*  m_enableLogDBLater(false), */
    m_enableLogDB(false), m_colored(false)
{
    Initialize();
}

Log::~Log()
{
    if( logfile != NULL )
        fclose(logfile);
    logfile = NULL;

    if( gmLogfile != NULL )
        fclose(gmLogfile);
    gmLogfile = NULL;

    if (charLogfile != NULL)
        fclose(charLogfile);
    charLogfile = NULL;

//    if( dberLogfile != NULL )
//        fclose(dberLogfile);
//    dberLogfile = NULL;

//    if (raLogfile != NULL)
//        fclose(raLogfile);
//    raLogfile = NULL;

    if (chatLogfile != NULL)
        fclose(chatLogfile);
    chatLogfile = NULL;

//    if (arenaLogFile != NULL)
//        fclose(arenaLogFile);
//    arenaLogFile = NULL;
//
//    if (sqlLogFile != NULL)
//        fclose(sqlLogFile);
//    sqlLogFile = NULL;
//
//    if (sqlDevLogFile != NULL)
//        fclose(sqlDevLogFile);
//    sqlDevLogFile = NULL;
//
//	if (DollarLogFile != NULL)
//		fclose(DollarLogFile);
//	DollarLogFile = NULL;
}
Log * Log::Instance()
{
	if (m_pInstance == 0)
	{
		m_pInstance = new Log();
	}
	return m_pInstance;
}
void Log::SetLogLevel(char *Level)
{
    int NewLevel =atoi((char*)Level);
    if ( NewLevel <0 )
        NewLevel = 0;
    m_logLevel = NewLevel;

    outString( "LogLevel is %u", m_logLevel );
}

void Log::SetLogFileLevel(char *Level)
{
    int NewLevel =atoi((char*)Level);
    if ( NewLevel <0 )
        NewLevel = 0;
    m_logFileLevel = NewLevel;

    outString( "LogFileLevel is %u", m_logFileLevel );
}

void Log::SetDBLogLevel(char *Level)
{
    int NewLevel = atoi((char*)Level);
    if ( NewLevel < 0 )
        NewLevel = 0;
    m_dbLogLevel = NewLevel;

    outString( "DBLogLevel is %u", m_dbLogLevel );
}

void Log::SetDebugLogMask(DebugLogFilters filter)
{
    m_DebugLogMask = filter;

    outString( "debug log mask is %u", m_DebugLogMask );
}

void Log::Initialize()
{
    /// Check whether we'll log GM commands/RA events/character outputs/chat stuffs
    m_dbChar = false;//sConfig->GetBoolDefault("LogDB.Char", false);
//    m_dbRA = false;//sConfig->GetBoolDefault("LogDB.RA", false);
    m_dbGM = false;//sConfig->GetBoolDefault("LogDB.GM", false);
    m_dbChat = false;//sConfig->GetBoolDefault("LogDB.Chat", false);

    /// Realm must be 0 by default
//    SetRealmID(0);

    /// Common log files data
    m_logsDir = "log";//sConfig->GetStringDefault("LogsDir", "");
    if (!m_logsDir.empty())
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");

	if ( -1 == access( m_logsDir.c_str(), F_OK ) ) {
		mkdir( m_logsDir.c_str(), 0755 );
		chmod( m_logsDir.c_str(), 0755 );
	}

    m_logsTimestamp = "_" + GetTimestampStr();

    /// Open specific log files
    logfile = openLogFile("LogFile", NULL, "a");
    //InitColors(sConfig->GetStringDefault("LogColors", ""));
    InitColors("2 11 5 4");

    m_gmlog_per_account = false;//sConfig->GetBoolDefault("GmLogPerAccount", false);
    if(!m_gmlog_per_account)
        gmLogfile = openLogFile("GMLogFile", NULL, "a");
    else
    {
        // GM log settings for per account case
        m_gmlog_filename_format = "";//sConfig->GetStringDefault("GMLogFile", "");
        if(!m_gmlog_filename_format.empty())
        {
            bool m_gmlog_timestamp = true;//sConfig->GetBoolDefault("GmLogTimestamp", false);

            size_t dot_pos = m_gmlog_filename_format.find_last_of(".");
            if(dot_pos!=m_gmlog_filename_format.npos)
            {
                if(m_gmlog_timestamp)
                    m_gmlog_filename_format.insert(dot_pos, m_logsTimestamp);

                m_gmlog_filename_format.insert(dot_pos, "_#%u");
            }
            else
            {
                m_gmlog_filename_format += "_#%u";

                if(m_gmlog_timestamp)
                    m_gmlog_filename_format += m_logsTimestamp;
            }

            m_gmlog_filename_format = m_logsDir + m_gmlog_filename_format;
        }
    }

    charLogfile = openLogFile("CharLogFile", NULL, "a");
//    dberLogfile = openLogFile("DBErrorLogFile", NULL, "a");
//    raLogfile = openLogFile("RaLogFile", NULL, "a");
    chatLogfile = openLogFile("ChatLogFile", NULL, "a");
//    arenaLogFile = openLogFile("ArenaLogFile", NULL, "a");
//    sqlLogFile = openLogFile("SQLDriverLogFile", NULL, "a");
//    sqlDevLogFile = openLogFile("SQLDeveloperLogFile", NULL, "a");
//    DollarLogFile = openLogFile("DollarLogFile", NULL, "a");

    // Main log file settings
    m_logLevel     = LOGL_DEBUG;//sConfig->GetIntDefault("LogLevel", LOGL_NORMAL);
    m_logFileLevel = LOGL_DEBUG;//sConfig->GetIntDefault("LogFileLevel", LOGL_NORMAL);
    m_dbLogLevel   = LOGL_DEBUG;//sConfig->GetIntDefault("DBLogLevel", LOGL_NORMAL);
//    m_sqlDriverQueryLogging  = false;//sConfig->GetBoolDefault("SQLDriverQueryLogging", false);

    m_DebugLogMask = LOG_FILTER_ALL;//DebugLogFilters(sConfig->GetIntDefault("DebugLogMask", LOG_FILTER_NONE));

    // Char log settings
//    m_charLog_Dump = false;//sConfig->GetBoolDefault("CharLogDump", false);
//    m_charLog_Dump_Separate = false;//sConfig->GetBoolDefault("CharLogDump.Separate", false);
//    if (m_charLog_Dump_Separate)
//    {
//        m_dumpsDir = "";//sConfig->GetStringDefault("CharLogDump.SeparateDir", "");
//        if (!m_dumpsDir.empty())
//            if ((m_dumpsDir.at(m_dumpsDir.length() - 1) != '/') && (m_dumpsDir.at(m_dumpsDir.length() - 1) != '\\'))
//                m_dumpsDir.append("/");
//    }
	
	m_logsTimestamp = time(NULL);
}

FILE* Log::openLogFile(char const* configFileName, char const* configTimeStampFlag, char const* mode)
{
    std::string logfn = configFileName;//sConfig->GetStringDefault(configFileName, "");
    if(logfn.empty())
        return NULL;

    if(configTimeStampFlag && true) //sConfig->GetBoolDefault(configTimeStampFlag, false))
    {
        size_t dot_pos = logfn.find_last_of(".");
        if(dot_pos!=logfn.npos)
            logfn.insert(dot_pos, m_logsTimestamp);
        else
            logfn += m_logsTimestamp;
    }

    return fopen((m_logsDir+logfn).c_str(), mode);
}

FILE* Log::openGmlogPerAccount(uint account)
{
    if(m_gmlog_filename_format.empty())
        return NULL;

    char namebuf[256];
    snprintf(namebuf, 256, m_gmlog_filename_format.c_str(), account);
    return fopen(namebuf, "a");
}

void Log::outTimestamp(FILE* file)
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    fprintf(file, "%-4d-%02d-%02d %02d:%02d:%02d,", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
}

void Log::InitColors(const std::string& str)
{
    if(str.empty())
    {
        m_colored = false;
        return;
    }

    int color[4];

    std::istringstream ss(str);

    for (uint8_t i = 0; i < LogLevels; ++i)
    {
        ss >> color[i];

        if(!ss)
            return;

        if(color[i] < 0 || color[i] >= Colors)
            return;
    }

    for (uint8_t i = 0; i < LogLevels; ++i)
        m_colors[i] = ColorTypes(color[i]);

    m_colored = true;
}

void Log::SetColor(bool stdout_stream, ColorTypes color)
{
//    #if PLATFORM == PLATFORM_WINDOWS
//    static WORD WinColorFG[Colors] =
//    {
//        0,                                                  // BLACK
//        FOREGROUND_RED,                                     // RED
//        FOREGROUND_GREEN,                                   // GREEN
//        FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
//        FOREGROUND_BLUE,                                    // BLUE
//        FOREGROUND_RED |                    FOREGROUND_BLUE, // MAGENTA
//        FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
//        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // WHITE
//                                                            // YELLOW
//        FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
//                                                            // RED_BOLD
//        FOREGROUND_RED |                                      FOREGROUND_INTENSITY,
//                                                            // GREEN_BOLD
//        FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
//        FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
//                                                            // MAGENTA_BOLD
//        FOREGROUND_RED |                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
//                                                            // CYAN_BOLD
//        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
//                                                            // WHITE_BOLD
//        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
//    };
//
//    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
//    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
//    #else
    enum ANSITextAttr
    {
        TA_NORMAL=0,
        TA_BOLD=1,
        TA_BLINK=5,
        TA_REVERSE=7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK=30, FG_RED,  FG_GREEN, FG_BROWN, FG_BLUE,
        FG_MAGENTA,  FG_CYAN, FG_WHITE, FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK=40, BG_RED,  BG_GREEN, BG_BROWN, BG_BLUE,
        BG_MAGENTA,  BG_CYAN, BG_WHITE
    };

    static uint8_t UnixColorFG[Colors] =
    {
        FG_BLACK,                                           // BLACK
        FG_RED,                                             // RED
        FG_GREEN,                                           // GREEN
        FG_BROWN,                                           // BROWN
        FG_BLUE,                                            // BLUE
        FG_MAGENTA,                                         // MAGENTA
        FG_CYAN,                                            // CYAN
        FG_WHITE,                                           // WHITE
        FG_YELLOW,                                          // YELLOW
        FG_RED,                                             // LRED
        FG_GREEN,                                           // LGREEN
        FG_BLUE,                                            // LBLUE
        FG_MAGENTA,                                         // LMAGENTA
        FG_CYAN,                                            // LCYAN
        FG_WHITE                                            // LWHITE
    };

    fprintf((stdout_stream? stdout : stderr), "\x1b[%d%sm", UnixColorFG[color], (color >= YELLOW && color < Colors ? ";1" : ""));
//    #endif
}

void Log::ResetColor(bool stdout_stream)
{
//    #if PLATFORM == PLATFORM_WINDOWS
//    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE );
//    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
//    #else
    fprintf(( stdout_stream ? stdout : stderr ), "\x1b[0m");
//    #endif
}

std::string Log::GetTimestampStr()
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d_%02d-%02d-%02d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

std::string Log::GetTimestampStr(time_t t)
{
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d_%02d-%02d-%02d", aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

void Log::outDB(LogTypes type, const char * str)
{
    if (!str || type >= MAX_LOG_TYPES)
         return;

    std::string new_str(str);
    if (new_str.empty())
        return;
    //todo
//    LoginDatabase.EscapeString(new_str);
//
//    LoginDatabase.PExecute("INSERT INTO logs (time, realm, type, string) "
//        "VALUES (" UI64FMTD ", %u, %u, '%s');", uint64(time(0)), realm, type, new_str.c_str());
}

void Log::outString(const char * str, ...)
{
    if (!str)
        return;

    if (m_enableLogDB)
    {
        // we don't want empty strings in the DB
        std::string s(str);
        if (s.empty() || s == " ")
            return;

        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_STRING, nnew_str);
        va_end(ap2);
    }

    if (m_colored)
        SetColor(true, m_colors[LOGL_NORMAL]);

    va_list ap;

    va_start(ap, str);
    vfprintf(stdout, str, ap);
    va_end(ap);

    if (m_colored)
        ResetColor(true);

    printf("\n");
    if(logfile)
    {
        outTimestamp(logfile);
        va_start(ap, str);
        vfprintf(logfile, str, ap);
        fprintf(logfile, "\n");
        va_end(ap);

        fflush(logfile);
    }
    fflush(stdout);
}

//void Log::outString()
//{
//    printf("\n");
//    if (logfile)
//    {
//        outTimestamp(logfile);
//        fprintf(logfile, "\n");
//        fflush(logfile);
//    }
//    fflush(stdout);
//}
//
//void Log::outCrash(const char * err, ...)
//{
//    if (!err)
//        return;
//
//    if (m_enableLogDB)
//    {
//        va_list ap2;
//        va_start(ap2, err);
//        char nnew_str[MAX_QUERY_LEN];
//        vsnprintf(nnew_str, MAX_QUERY_LEN, err, ap2);
//        outDB(LOG_TYPE_CRASH, nnew_str);
//        va_end(ap2);
//    }
//
//    if (m_colored)
//        SetColor(false, LRED);
//
//    va_list ap;
//
//    va_start(ap, err);
//    vfprintf(stderr, err, ap);
//    va_end(ap);
//
//    if (m_colored)
//        ResetColor(false);
//
//    fprintf(stderr, "\n");
//    if (logfile)
//    {
//        outTimestamp(logfile);
//        fprintf(logfile, "CRASH ALERT: ");
//
//        va_start(ap, err);
//        vfprintf(logfile, err, ap);
//        va_end(ap);
//
//        fprintf(logfile, "\n");
//        fflush(logfile);
//    }
//    fflush(stderr);
//}

void Log::outError(const char * err, ...)
{
    if (!err)
        return;

    if (m_enableLogDB)
    {
        va_list ap2;
        va_start(ap2, err);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, err, ap2);
        outDB(LOG_TYPE_ERROR, nnew_str);
        va_end(ap2);
    }

    if (m_colored)
        SetColor(false, LRED);

    va_list ap;

    va_start(ap, err);
    vfprintf(stderr, err, ap);
    va_end(ap);

    if (m_colored)
        ResetColor(false);

    fprintf( stderr, "\n");
    if (logfile)
    {
        outTimestamp(logfile);
//        fprintf(logfile, "ERROR: ");

        va_start(ap, err);
        vfprintf(logfile, err, ap);
        va_end(ap);

        fprintf(logfile, "\n");
        fflush(logfile);
    }
    fflush(stderr);
}

void Log::outGmSystem(const char * strlv, const char * str, ...)
{
    if (!str)
        return;

    // TODO: support accountid
    if (m_enableLogDB && m_dbGM)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_GM, nnew_str);
        va_end(ap2);
    }

	uint account = 0;
    if (m_gmlog_per_account)
    {
        if (FILE* per_file = openGmlogPerAccount (account))
        {
            outTimestamp(per_file);
            fprintf(per_file, strlv, "");
            va_list ap;
            va_start(ap, str);
            vfprintf(per_file, str, ap);
            fprintf(per_file, "\n" );
            va_end(ap);
            fclose(per_file);
        }
    }
    else if (gmLogfile)
    {
        outTimestamp(gmLogfile);
//        fprintf(gmLogfile, strlv, "");
        va_list ap;
        va_start(ap, str);
        vfprintf(gmLogfile, str, ap);
        fprintf(gmLogfile, "\n" );
        va_end(ap);
        fflush(gmLogfile);
    }

    fflush(stdout);
}

//void Log::outArena(const char * str, ...)
//{
//    if (!str)
//        return;
//
//    if (arenaLogFile)
//    {
//        va_list ap;
//        outTimestamp(arenaLogFile);
//        va_start(ap, str);
//        vfprintf(arenaLogFile, str, ap);
//        fprintf(arenaLogFile, "\n");
//        va_end(ap);
//        fflush(arenaLogFile);
//    }
//}
//
//void Log::outDollar(const char* str, ...)
//{
//    if (!str)
//        return;
//
//    if (DollarLogFile)
//    {
//        outTimestamp(DollarLogFile);
//
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(DollarLogFile, str, ap);
//        va_end(ap);
//
//        fprintf(DollarLogFile, "\n");
//        fflush(DollarLogFile);
//    }
//
//    fflush(stdout);
//}
//
//
//void Log::outSQLDriver(const char* str, ...)
//{
//    if (!str)
//        return;
//
//    va_list ap;
//    va_start(ap, str);
//    vfprintf(stdout, str, ap);
//    va_end(ap);
//
//    printf("\n");
//
//    if (sqlLogFile)
//    {
//        outTimestamp(sqlLogFile);
//
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(sqlLogFile, str, ap);
//        va_end(ap);
//
//        fprintf(sqlLogFile, "\n");
//        fflush(sqlLogFile);
//    }
//
//    fflush(stdout);
//}
//
//void Log::outErrorDb(const char * err, ...)
//{
//    if (!err)
//        return;
//
//    if (m_colored)
//        SetColor(false, LRED);
//
//    va_list ap;
//
//    va_start(ap, err);
//    vfprintf(stderr, err, ap);
//    va_end(ap);
//
//    if (m_colored)
//        ResetColor(false);
//
//    fprintf( stderr, "\n" );
//
//    if (logfile)
//    {
//        outTimestamp(logfile);
//        fprintf(logfile, "ERROR: " );
//
//        va_start(ap, err);
//        vfprintf(logfile, err, ap);
//        va_end(ap);
//
//        fprintf(logfile, "\n" );
//        fflush(logfile);
//    }
//
//    if (dberLogfile)
//    {
//        outTimestamp(dberLogfile);
//        va_start(ap, err);
//        vfprintf(dberLogfile, err, ap);
//        va_end(ap);
//
//        fprintf(dberLogfile, "\n" );
//        fflush(dberLogfile);
//    }
//    fflush(stderr);
//}

void Log::outBasic(const char * str, ...)
{
    if (!str)
        return;

    if (m_enableLogDB && m_dbLogLevel > LOGL_NORMAL)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_BASIC, nnew_str);
        va_end(ap2);
    }

    if (m_logLevel > LOGL_NORMAL)
    {
        if (m_colored)
            SetColor(true, m_colors[LOGL_BASIC]);

        va_list ap;
        va_start(ap, str);
        vfprintf(stdout, str, ap);
        va_end(ap);

        if (m_colored)
            ResetColor(true);

        printf("\n");

        if (logfile)
        {
            outTimestamp(logfile);
            va_list ap;
            va_start(ap, str);
            vfprintf(logfile, str, ap);
            fprintf(logfile, "\n" );
            va_end(ap);
            fflush(logfile);
        }
    }
    fflush(stdout);
}

void Log::outDetail(const char * str, ...)
{
    if (!str)
        return;

    if (m_enableLogDB && m_dbLogLevel > LOGL_BASIC)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_DETAIL, nnew_str);
        va_end(ap2);
    }

    if (m_logLevel > LOGL_BASIC)
    {
        if (m_colored)
            SetColor(true, m_colors[LOGL_DETAIL]);

        va_list ap;
        va_start(ap, str);
        vfprintf(stdout, str, ap);
        va_end(ap);

        if (m_colored)
            ResetColor(true);

        printf("\n");

        if (logfile)
        {
            outTimestamp(logfile);
            va_list ap;
            va_start(ap, str);
            vfprintf(logfile, str, ap);
            va_end(ap);

            fprintf(logfile, "\n");
            fflush(logfile);
        }
    }

    fflush(stdout);
}

//void Log::outDebugInLine(const char * str, ...)
//{
//    if (!str)
//        return;
//
//    if (m_logLevel > LOGL_DETAIL)
//    {
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(stdout, str, ap);
//        va_end(ap);
//
//        //if(m_colored)
//        //    ResetColor(true);
//
//        if (logfile)
//        {
//            va_list ap;
//            va_start(ap, str);
//            vfprintf(logfile, str, ap);
//            va_end(ap);
//        }
//    }
//}

//void Log::outSQLDev(const char* str, ...)
//{
//    if (!str)
//        return;
//
//    va_list ap;
//    va_start(ap, str);
//    vfprintf(stdout, str, ap);
//    va_end(ap);
//
//    printf("\n");
//
//    if (sqlDevLogFile)
//    {
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(sqlDevLogFile, str, ap);
//        va_end(ap);
//
//        fprintf(sqlDevLogFile, "\n");
//        fflush(sqlDevLogFile);
//    }
//
//    fflush(stdout);
//}

void Log::outDebug(DebugLogFilters f, const char * str, ...)
{
    if (!(m_DebugLogMask & f))
        return;

    if (!str)
        return;

    if (m_enableLogDB && m_dbLogLevel > LOGL_DETAIL)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_DEBUG, nnew_str);
        va_end(ap2);
    }

    if( m_logLevel > LOGL_DETAIL )
    {
        if (m_colored)
            SetColor(true, m_colors[LOGL_DEBUG]);

        va_list ap;
        va_start(ap, str);
        vfprintf(stdout, str, ap);
        va_end(ap);

        if(m_colored)
            ResetColor(true);

        printf( "\n" );

        if (logfile)
        {
            outTimestamp(logfile);
            va_list ap;
            va_start(ap, str);
            vfprintf(logfile, str, ap);
            va_end(ap);

            fprintf(logfile, "\n" );
            fflush(logfile);
        }
    }
    fflush(stdout);
}

//void Log::outStaticDebug(const char * str, ...)
//{
//    if (!str)
//        return;
//
//    if (m_enableLogDB && m_dbLogLevel > LOGL_DETAIL)
//    {
//        va_list ap2;
//        va_start(ap2, str);
//        char nnew_str[MAX_QUERY_LEN];
//        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
//        outDB(LOG_TYPE_DEBUG, nnew_str);
//        va_end(ap2);
//    }
//
//    if( m_logLevel > LOGL_DETAIL )
//    {
//        if (m_colored)
//            SetColor(true, m_colors[LOGL_DEBUG]);
//
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(stdout, str, ap);
//        va_end(ap);
//
//        if(m_colored)
//            ResetColor(true);
//
//        printf( "\n" );
//
//        if (logfile)
//        {
//            outTimestamp(logfile);
//            va_list ap;
//            va_start(ap, str);
//            vfprintf(logfile, str, ap);
//            va_end(ap);
//
//            fprintf(logfile, "\n" );
//            fflush(logfile);
//        }
//    }
//    fflush(stdout);
//}

//void Log::outStringInLine(const char * str, ...)
//{
//    if (!str)
//        return;
//
//    va_list ap;
//
//    va_start(ap, str);
//    vfprintf(stdout, str, ap);
//    va_end(ap);
//
//    if (logfile)
//    {
//        va_start(ap, str);
//        vfprintf(logfile, str, ap);
//        va_end(ap);
//    }
//}

/*
void Log::outCommand(uint account, const char * str, ...)
{
    if (!str)
        return;

    // TODO: support accountid
    if (m_enableLogDB && m_dbGM)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_GM, nnew_str);
        va_end(ap2);
    }

//    if (m_logLevel > LOGL_NORMAL)
//    {
//        if (m_colored)
//            SetColor(true, m_colors[LOGL_BASIC]);
//
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(stdout, str, ap);
//        va_end(ap);
//
//        if (m_colored)
//            ResetColor(true);
//
//        printf("\n");
//
//        if (logfile)
//        {
//            outTimestamp(logfile);
//            va_list ap;
//            va_start(ap, str);
//            vfprintf(logfile, str, ap);
//            fprintf(logfile, "\n" );
//            va_end(ap);
//            fflush(logfile);
//        }
//    }

    if (m_gmlog_per_account)
    {
        if (FILE* per_file = openGmlogPerAccount (account))
        {
            outTimestamp(per_file);
            va_list ap;
            va_start(ap, str);
            vfprintf(per_file, str, ap);
            fprintf(per_file, "\n" );
            va_end(ap);
            fclose(per_file);
        }
    }
    else if (gmLogfile)
    {
        outTimestamp(gmLogfile);
        va_list ap;
        va_start(ap, str);
        vfprintf(gmLogfile, str, ap);
        fprintf(gmLogfile, "\n" );
        va_end(ap);
        fflush(gmLogfile);
    }

    fflush(stdout);
}
*/

void Log::outChar(const char * str, ...)
{
    if (!str)
        return;

    if (m_enableLogDB && m_dbChar)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_CHAR, nnew_str);
        va_end(ap2);
    }

	if (m_colored)
		SetColor(true, m_colors[LOGL_NORMAL]);

	va_list ap;
	va_start(ap, str);
	vfprintf(stdout, str, ap);
	va_end(ap);

	if (m_colored)
		ResetColor(true);

	printf("\n");
    fflush(stdout);

    if (charLogfile)
    {
        outTimestamp(charLogfile);
        va_list ap;
        va_start(ap, str);
        vfprintf(charLogfile, str, ap);
        fprintf(charLogfile, "\n" );
        va_end(ap);
        fflush(charLogfile);
    }
}

//void Log::outCharDump(const char * str, uint account_id, uint guid, const char * name)
//{
//    FILE *file = NULL;
//    if (m_charLog_Dump_Separate)
//    {
//        char fileName[29]; // Max length: name(12) + guid(11) + _.log (5) + \0
//        snprintf(fileName, 29, "%d_%s.log", guid, name);
//        std::string sFileName(m_dumpsDir);
//        sFileName.append(fileName);
//        file = fopen((m_logsDir + sFileName).c_str(), "w");
//    }
//    else
//        file = charLogfile;
//    if (file)
//    {
//        fprintf(file, "== START DUMP == (account: %u guid: %u name: %s )\n%s\n== END DUMP ==\n",
//            account_id, guid, name, str);
//        fflush(file);
//        if (m_charLog_Dump_Separate)
//            fclose(file);
//    }
//}

//void Log::outRemote(const char * str, ...)
//{
//    if (!str)
//        return;
//
//    if (m_enableLogDB && m_dbRA)
//    {
//        va_list ap2;
//        va_start(ap2, str);
//        char nnew_str[MAX_QUERY_LEN];
//        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
//        outDB(LOG_TYPE_RA, nnew_str);
//        va_end(ap2);
//    }
//
//    if (raLogfile)
//    {
//        outTimestamp(raLogfile);
//        va_list ap;
//        va_start(ap, str);
//        vfprintf(raLogfile, str, ap);
//        fprintf(raLogfile, "\n" );
//        va_end(ap);
//        fflush(raLogfile);
//    }
//}

void Log::outChat(const char * str, ...)
{
	
    if (!str)
        return;

    if (m_enableLogDB && m_dbChat)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(LOG_TYPE_CHAT, nnew_str);
        va_end(ap2);
    }

	if (m_colored)
		SetColor(true, m_colors[LOGL_NORMAL]);

	va_list ap;
	va_start(ap, str);
	vfprintf(stdout, str, ap);
	va_end(ap);

	if (m_colored)
		ResetColor(true);

	printf("\n");
    fflush(stdout);


    if (chatLogfile)
    {
        outTimestamp(chatLogfile);
        va_list ap;
        va_start(ap, str);
        vfprintf(chatLogfile, str, ap);
        fprintf(chatLogfile, "\n" );
        fflush(chatLogfile);
        va_end(ap);
    }
}


void Log::outOtherFile(const char *key, bool isDb, LogTypes logdbtype, FILE *pfile, const char *str, ...)
{
    if (!str)
        return;

    // TODO: support accountid
    if (m_enableLogDB && isDb)
    {
        va_list ap2;
        va_start(ap2, str);
        char nnew_str[MAX_QUERY_LEN];
        vsnprintf(nnew_str, MAX_QUERY_LEN, str, ap2);
        outDB(logdbtype, nnew_str);
        va_end(ap2);
    }

    if (pfile)
    {
        outTimestamp(pfile);
        fprintf(pfile, key, "");
        va_list ap;
        va_start(ap, str);
        vfprintf(pfile, str, ap);
        fprintf(pfile, "\n" );
        va_end(ap);
        fflush(pfile);
    }
}

void Log::ChangeLogFile(std::string name)
{
	//将旧日志文件改名
	std::string logname = m_logsDir + name;
	std::string storefile = m_logsDir + name + "." + GetTimestampStr(time(NULL) - 24*3600);

	// 移动到其他位置
	rename(logname.c_str(), storefile.c_str());
}

void Log::LogFileChange()
{
	ChangeLogFile("LogFile");
	ChangeLogFile("CharLogFile");
	ChangeLogFile("ChatLogFile");

    logfile = openLogFile("LogFile", NULL, "a");
    charLogfile = openLogFile("CharLogFile", NULL, "a");
    chatLogfile = openLogFile("ChatLogFile", NULL, "a");
}

void ime_log_script(const char *content, const char *filename, int line, const char *func)
{
	sLog->outBasic("%u,0,NORMAL,%s:%d:%s,desc=%s", LOGL_BASIC, filename, line, func, content);
}

void ime_detail_script(const char *content, const char *filename, int line, const char *func)
{
	sLog->outDetail("%u,0,DETAIL,%s:%d:%s,desc=%s", LOGL_DETAIL, filename, line, func, content);
}

void ime_debug_script(const char *content, const char *filename, int line, const char *func)
{
	sLog->outDebug(LOG_FILTER_ALL, "%u,0,DEBUG,%s:%d:%s,desc=%s", LOGL_DEBUG, filename, line, func, content);
}

void ime_error_script(const char *content, const char *filename, int line, const char *func)
{
	sLog->outError("%u,0,ERROR,%s:%d:%s,desc=%s", LOGL_NORMAL, filename, line, func, content);
}

void ime_warn_script(const char *content, const char *filename, int line, const char *func)
{
	sLog->outError("%u,0,WARN,%s:%d:%s,desc=%s", LOGL_NORMAL, filename, line, func, content);
}
