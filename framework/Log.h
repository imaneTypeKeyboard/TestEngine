
#ifndef _LOG_H
#define _LOG_H
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#  define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))

enum DebugLogFilters
{
    LOG_FILTER_NONE                     = 0x00000000,
    LOG_FILTER_UNITS                    = 0x00000001,   // Anything related to units that doesn't fit in other categories. ie. creature formations
    LOG_FILTER_PETS                     = 0x00000002,
    LOG_FILTER_VEHICLES                 = 0x00000004,
    LOG_FILTER_TSCR                     = 0x00000008,   // C++ AI, instance scripts, etc.
    LOG_FILTER_DATABASE_AI              = 0x08000010,   // SmartAI, EventAI, CreatureAI
    LOG_FILTER_MAPSCRIPTS               = 0x00000020,
    LOG_FILTER_NETWORKIO                = 0x00000040,   // Anything packet/netcode related
    LOG_FILTER_SPELLS_AURAS             = 0x00000080,
    LOG_FILTER_ACHIEVEMENTSYS           = 0x00000100,
    LOG_FILTER_CONDITIONSYS             = 0x00000200,
    LOG_FILTER_POOLSYS                  = 0x00000400,
    LOG_FILTER_AUCTIONHOUSE             = 0x00000800,
    LOG_FILTER_BATTLEGROUND             = 0x00001000,   // Anything related to arena's and battlegrounds
    LOG_FILTER_OUTDOORPVP               = 0x00002000,
    LOG_FILTER_CHATSYS                  = 0x00004000,
    LOG_FILTER_LFG                      = 0x00008000,
    LOG_FILTER_MAPS                     = 0x00010000,   // Maps, instances, grids, cells, visibility
    LOG_FILTER_PLAYER_LOADING           = 0x00020000,   // Debug output from Player::_Load functions
    LOG_FILTER_PLAYER_ITEMS             = 0x00040000,   // Anything item related
    LOG_FILTER_PLAYER_SKILLS            = 0x00080000,   // Skills related
    LOG_FILTER_LOOT                     = 0x00100000,   // Loot related
    LOG_FILTER_GUILD                    = 0x00200000,   // Guild related
    LOG_FILTER_TRANSPORTS               = 0x00400000,   // Transport related
	LOG_FILTER_ALL						= 0xffffffff	// All
};

enum LogTypes
{
    LOG_TYPE_STRING = 0,
    LOG_TYPE_ERROR  = 1,
    LOG_TYPE_BASIC  = 2,
    LOG_TYPE_DETAIL = 3,
    LOG_TYPE_DEBUG  = 4,
    LOG_TYPE_CHAR   = 5,
    LOG_TYPE_WORLD  = 6,
    LOG_TYPE_RA     = 7,
    LOG_TYPE_GM     = 8,
    LOG_TYPE_CRASH  = 9,
    LOG_TYPE_CHAT   = 10,
    MAX_LOG_TYPES
};

enum LogLevel
{
    LOGL_NORMAL = 0,
    LOGL_BASIC,
    LOGL_DETAIL,
    LOGL_DEBUG
};

const int LogLevels = int(LOGL_DEBUG)+1;

enum ColorTypes
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE
};

const int Colors = int(WHITE)+1;

class Log
{

    Log();
    ~Log();

    public:

    	static Log * Instance();
        void Initialize();
		void LogFileChange();
		void ChangeLogFile(std::string name);

        void InitColors(const std::string& init_str);
        void SetColor(bool stdout_stream, ColorTypes color);
        void ResetColor(bool stdout_stream);

        void outDB( LogTypes type, const char * str );

        void outString( const char * str, ... )                 ATTR_PRINTF(2, 3);
//        void outString( );
//        void outStringInLine( const char * str, ... )           ATTR_PRINTF(2, 3);
        void outError( const char * err, ... )                  ATTR_PRINTF(2, 3);
//        void outCrash( const char * err, ... )                  ATTR_PRINTF(2, 3);
        void outBasic( const char * str, ... )                  ATTR_PRINTF(2, 3);
        void outDetail( const char * str, ... )                 ATTR_PRINTF(2, 3);
//        void outSQLDev( const char * str, ... )                 ATTR_PRINTF(2, 3);
        void outDebug(DebugLogFilters f, const char* str, ...)  ATTR_PRINTF(3, 4);
//        void outStaticDebug( const char * str, ... )            ATTR_PRINTF(2, 3);
//        void outDebugInLine( const char * str, ... )            ATTR_PRINTF(2, 3);
//        void outErrorDb( const char * str, ... )                ATTR_PRINTF(2, 3);
        void outChar( const char * str, ... )                   ATTR_PRINTF(2, 3);
//        void outCommand( uint32_t account, const char * str, ...) ATTR_PRINTF(3, 4);
//        void outRemote( const char * str, ... )                 ATTR_PRINTF(2, 3);
        void outChat( const char * str, ... )                   ATTR_PRINTF(2, 3);
//        void outArena( const char * str, ... )                  ATTR_PRINTF(2, 3);
//        void outSQLDriver( const char* str, ... )               ATTR_PRINTF(2, 3);
//        void outCharDump( const char * str, uint32_t account_id, uint32_t guid, const char * name );

//        void outDollar( const char* str, ... )               ATTR_PRINTF(2, 3);
		void outGmSystem(const char * strlv, const char * str, ...);
		void outOtherFile(const char *key, bool isDb, LogTypes logdbtype, FILE *pfile, const char *str, ...);

        static void outTimestamp(FILE* file);
        static std::string GetTimestampStr();
        static std::string GetTimestampStr(time_t);

        void SetLogLevel(char * Level);
        void SetLogFileLevel(char * Level);
		void SetDebugLogMask(DebugLogFilters filter);

        void SetDBLogLevel(char * Level);
//        void SetSQLDriverQueryLogging(bool newStatus) { m_sqlDriverQueryLogging = newStatus; }
//        void SetRealmID(uint32_t id) { realm = id; }

        bool IsOutDebug() const { return m_logLevel > 2 || (m_logFileLevel > 2 && logfile); }
//        bool IsOutCharDump() const { return m_charLog_Dump; }

        bool GetLogDB() const { return m_enableLogDB; }
//        bool GetLogDBLater() const { return m_enableLogDBLater; }
        void SetLogDB(bool enable) { m_enableLogDB = enable; }
//        void SetLogDBLater(bool value) { m_enableLogDBLater = value; }
//        bool GetSQLDriverQueryLogging() const { return m_sqlDriverQueryLogging; }
    private:
        FILE* openLogFile(char const* configFileName, char const* configTimeStampFlag, char const* mode);
        FILE* openGmlogPerAccount(uint32_t account);

//        FILE* raLogfile;
        FILE* logfile;
        FILE* gmLogfile;
        FILE* charLogfile;
//        FILE* dberLogfile;
        FILE* chatLogfile;
//        FILE* arenaLogFile;
//        FILE* sqlLogFile;
//        FILE* sqlDevLogFile;
//		FILE* DollarLogFile;

        // cache values for after initilization use (like gm log per account case)
        std::string m_logsDir;
        std::string m_logsTimestamp;

        // gm log control
        bool m_gmlog_per_account;
        std::string m_gmlog_filename_format;

//        bool m_enableLogDBLater;
        bool m_enableLogDB;
//        uint32_t realm;

        // log coloring
        bool m_colored;
        ColorTypes m_colors[4];

        // log levels:
        // false: errors only, true: full query logging
//        bool m_sqlDriverQueryLogging;

        // log levels:
        // 0 minimum/string, 1 basic/error, 2 detail, 3 full/debug
        uint8_t m_dbLogLevel;
        uint8_t m_logLevel;
        uint8_t m_logFileLevel;
        bool m_dbChar;
//        bool m_dbRA;
        bool m_dbGM;
        bool m_dbChat;
//        bool m_charLog_Dump;
//        bool m_charLog_Dump_Separate;
//        std::string m_dumpsDir;

        DebugLogFilters m_DebugLogMask;

        static Log * m_pInstance;
};
#ifdef WIN32
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define sLog Log::Instance()

#define IME_TEST_LOG(opt,fmt, ...)		sLog->outChar("%s,"fmt, opt,  ##__VA_ARGS__)

//#define IME_DOLLAR_LOG(fmt, ...)		sLog->outDollar("%s:%d:%s- "fmt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_CHAR_LOG(userid,opt,fmt, ...)		sLog->outChar("%u,%u,%s,%s:%d:%s,"fmt, LOGL_NORMAL, userid, opt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_USER_LOG(opt,userid,fmt, ...)	sLog->outChar("%u,%u,%s,%s:%d:%s,"fmt, LOGL_NORMAL, userid, opt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__);	
#define IME_USER_ERROR(opt,userid,fmt, ...)	sLog->outChar("%u,%u,%s,%s:%d:%s,"fmt, LOGL_NORMAL, userid, opt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__);	
#define IME_USER_DEBUG(opt,userid,fmt, ...)	sLog->outChar("%u,%u,%s,%s:%d:%s,"fmt, LOGL_NORMAL, userid, opt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__);	

#define IME_LOG(fmt, ...)		sLog->outBasic("%u,0,NORMAL,%s:%d:%s,desc="fmt, LOGL_BASIC, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
//#define IME_LOG_TEST(fmt, ...)		sLog->outBasic("%s:%d:%s- "fmt, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_DETAIL(fmt, ...)		sLog->outDetail("%u,0,DETAIL,%s:%d:%s,desc="fmt, LOGL_DETAIL, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_DEBUG(fmt, ...)		sLog->outDebug(LOG_FILTER_ALL, "%u,0,DEBUG,%s:%d:%s,desc="fmt, LOGL_DEBUG, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_ERROR(fmt, ...)		sLog->outError("%u,0,ERROR,%s:%d:%s,desc="fmt, LOGL_NORMAL, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_WARN(fmt, ...)		sLog->outError("%u,0,WARN,%s:%d:%s,desc="fmt, LOGL_NORMAL, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)

#define IME_SQLDRIVER(fmt, ...)		sLog->outBasic("%u,0,NORMAL,%s:%d:%s,desc="fmt, LOGL_BASIC, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)
#define IME_SQLERROR(fmt, ...)		sLog->outError("%u,0,ERROR,%s:%d:%s,desc="fmt, LOGL_NORMAL, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)

#define IME_SYSTEM_ERROR(fmt, ...)		IME_ERROR(fmt, ##__VA_ARGS__)
#define IME_SYSTEM_LOG(cmd, fmt, ...)		IME_LOG(fmt, ##__VA_ARGS__)	

#define IME_SIMPLE_LOG(fmt, ...)		sLog->outBasic(fmt, ##__VA_ARGS__)
#define IME_SIMPLE_ERROR(fmt, ...)		sLog->outError(fmt, ##__VA_ARGS__)

#define IME_CHAT_LOG(fmt, ...)		sLog->outBasic("%u,0,CHAT,%s:%d:%s,desc="fmt, LOGL_NORMAL, FILENAME, __LINE__, __FUNCTION__,  ##__VA_ARGS__)

void ime_log_script(const char *content, const char *filename, int line, const char *func);
void ime_detail_script(const char *content, const char *filename, int line, const char *func);
void ime_debug_script(const char *content, const char *filename, int line, const char *func);
void ime_error_script(const char *content, const char *filename, int line, const char *func);
void ime_warn_script(const char *content, const char *filename, int line, const char *func);

#endif

