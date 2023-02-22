////////////////////////////////////////////////////////////////////////////////
// CppSQLite3 - A C++ wrapper around the SQLite3 embedded database library.
//
// Copyright (c) 2004..2007 Rob Groves. All Rights Reserved. rob.groves@btinternet.com
// 
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without a written
// agreement, is hereby granted, provided that the above copyright notice, 
// this paragraph and the following two paragraphs appear in all copies, 
// modifications, and distributions.
//
// IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
// PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
// EVEN IF THE AUTHOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF
// ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS". THE AUTHOR HAS NO OBLIGATION
// TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// V3.0		03/08/2004	-Initial Version for sqlite3
//
// V3.1		16/09/2004	-Implemented getXXXXField using sqlite3 functions
//						-Added CppSQLiteDB3::tableExists()
//
// V3.2		01/07/2005	-Fixed execScalar to handle a NULL result
//			12/07/2007	-Added int64 functions to CppSQLite3Query
//						-Throw exception from CppSQLite3DB::close() if error
//						-Trap above exception in CppSQLite3DB::~CppSQLite3DB()
//						-Fix to CppSQLite3DB::compile() as provided by Dave Rollins.
//						-sqlite3_prepare replaced with sqlite3_prepare_v2
//						-Added Name based parameter binding to CppSQLite3Statement.
////////////////////////////////////////////////////////////////////////////////
#include "CppSQLite3.h"
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#else
#include <dlfcn.h>
#endif // _WIN32

// Named constant for passing to CppSQLite3Exception when passing it a string
	// that cannot be deleted.
static const bool DONT_DELETE_MSG = false;

namespace CppSQLite3 {
	
	/*
	sqlite3_column_count
	sqlite3_column_name
	sqlite3_column_type
	sqlite3_column_decltype
	sqlite3_column_text
	sqlite3_column_int
	sqlite3_column_int64
	sqlite3_column_double
	sqlite3_column_blob
	sqlite3_column_bytes

	sqlite3_bind_text
	sqlite3_bind_int
	sqlite3_bind_int64
	sqlite3_bind_double
	sqlite3_bind_blob
	sqlite3_bind_null

	sqlite3_bind_parameter_count
	sqlite3_bind_parameter_index
	sqlite3_bind_parameter_name

	sqlite3_exec
	sqlite3_step
	sqlite3_changes
	sqlite3_reset
	sqlite3_finalize
	sqlite3_errcode
	sqlite3_errmsg
	sqlite3_close

	sqlite3_open_v2
	sqlite3_prepare_v2
	sqlite3_key
	sqlite3_rekey

	sqlite3_busy_timeout
	sqlite3_last_insert_rowid
	sqlite3_get_autocommit

	sqlite3_get_table
	sqlite3_free_table

	sqlite3_backup_init
	sqlite3_backup_step
	sqlite3_backup_finish

	sqlite3_mprintf
	sqlite3_vmprintf
	sqlite3_free
	*/
	
	using func_sqlite3_column_count = int(*)(sqlite3_stmt*);
	func_sqlite3_column_count _sqlite3_column_count = sqlite3_column_count;
	using func_sqlite3_column_name = const char* (*)(sqlite3_stmt*, int);
	func_sqlite3_column_name _sqlite3_column_name = sqlite3_column_name;
	using func_sqlite3_column_type = int(*)(sqlite3_stmt*, int);
	func_sqlite3_column_type _sqlite3_column_type = sqlite3_column_type;
	using func_sqlite3_column_decltype = const char* (*)(sqlite3_stmt*, int);
	func_sqlite3_column_decltype _sqlite3_column_decltype = sqlite3_column_decltype;
	using func_sqlite3_column_text = const unsigned char* (*)(sqlite3_stmt*, int);
	func_sqlite3_column_text _sqlite3_column_text = sqlite3_column_text;
	using func_sqlite3_column_int = int(*)(sqlite3_stmt*, int);
	func_sqlite3_column_int _sqlite3_column_int = sqlite3_column_int;
	using func_sqlite3_column_int64 = sqlite3_int64(*)(sqlite3_stmt*, int);
	func_sqlite3_column_int64 _sqlite3_column_int64 = sqlite3_column_int64;
	using func_sqlite3_column_double = double(*)(sqlite3_stmt*, int);
	func_sqlite3_column_double _sqlite3_column_double = sqlite3_column_double;
	using func_sqlite3_column_blob = const void* (*)(sqlite3_stmt*, int);
	func_sqlite3_column_blob _sqlite3_column_blob = sqlite3_column_blob;
	using func_sqlite3_column_bytes = int(*)(sqlite3_stmt*, int);
	func_sqlite3_column_bytes _sqlite3_column_bytes = sqlite3_column_bytes;
	
	using func_sqlite3_bind_text = int(*)(sqlite3_stmt*, int, const char*, int, void(*)(void*));
	func_sqlite3_bind_text _sqlite3_bind_text = sqlite3_bind_text;
	using func_sqlite3_bind_int = int(*)(sqlite3_stmt*, int, int);
	func_sqlite3_bind_int _sqlite3_bind_int = sqlite3_bind_int;
	using func_sqlite3_bind_int64 = int(*)(sqlite3_stmt*, int, sqlite3_int64);
	func_sqlite3_bind_int64 _sqlite3_bind_int64 = sqlite3_bind_int64;
	using func_sqlite3_bind_double = int(*)(sqlite3_stmt*, int, double);
	func_sqlite3_bind_double _sqlite3_bind_double = sqlite3_bind_double;
	using func_sqlite3_bind_blob = int(*)(sqlite3_stmt*, int, const void*, int, void(*)(void*));
	func_sqlite3_bind_blob _sqlite3_bind_blob = sqlite3_bind_blob;
	using func_sqlite3_bind_null = int(*)(sqlite3_stmt*, int);
	func_sqlite3_bind_null _sqlite3_bind_null = sqlite3_bind_null;
	
	using func_sqlite3_bind_parameter_count = int(*)(sqlite3_stmt*);
	func_sqlite3_bind_parameter_count _sqlite3_bind_parameter_count = sqlite3_bind_parameter_count;
	using func_sqlite3_bind_parameter_index = int(*)(sqlite3_stmt*, const char*);
	func_sqlite3_bind_parameter_index _sqlite3_bind_parameter_index = sqlite3_bind_parameter_index;
	using func_sqlite3_bind_parameter_name = const char* (*)(sqlite3_stmt*, int);
	func_sqlite3_bind_parameter_name _sqlite3_bind_parameter_name = sqlite3_bind_parameter_name;
	
	using func_sqlite3_exec = int(*)(sqlite3*, const char*, int(*)(void*, int, char**, char**), void*, char**);
	func_sqlite3_exec _sqlite3_exec = sqlite3_exec;
	using func_sqlite3_step = int(*)(sqlite3_stmt*);
	func_sqlite3_step _sqlite3_step = sqlite3_step;
	using func_sqlite3_changes = int(*)(sqlite3*);
	func_sqlite3_changes _sqlite3_changes = sqlite3_changes;
	using func_sqlite3_reset = int(*)(sqlite3_stmt*);
	func_sqlite3_reset _sqlite3_reset = sqlite3_reset;
	using func_sqlite3_finalize = int(*)(sqlite3_stmt*);
	func_sqlite3_finalize _sqlite3_finalize = sqlite3_finalize;
	using func_sqlite3_errcode = int(*)(sqlite3*);
	func_sqlite3_errcode _sqlite3_errcode = sqlite3_errcode;
	using func_sqlite3_errmsg = const char* (*)(sqlite3*);
	func_sqlite3_errmsg _sqlite3_errmsg = sqlite3_errmsg;
	using func_sqlite3_close = int(*)(sqlite3*);
	func_sqlite3_close _sqlite3_close = sqlite3_close;
	
	using func_sqlite3_open_v2 = int(*)(const char*, sqlite3**, int, const char*);
	func_sqlite3_open_v2 _sqlite3_open_v2 = sqlite3_open_v2;
	using func_sqlite3_prepare_v2 = int(*)(sqlite3*, const char*, int, sqlite3_stmt**, const char**);
	func_sqlite3_prepare_v2 _sqlite3_prepare_v2 = sqlite3_prepare_v2;
	
	using func_sqlite3_key = int(*)(sqlite3*, const void*, int);
	using func_sqlite3_rekey = int(*)(sqlite3*, const void*, int);
#if SQLITE_HAS_CODEC
	func_sqlite3_key _sqlite3_key = sqlite3_key;
	func_sqlite3_rekey _sqlite3_rekey = sqlite3_rekey;
#else
	func_sqlite3_key _sqlite3_key = nullptr;
	func_sqlite3_rekey _sqlite3_rekey = nullptr;
#endif
	
	using func_sqlite3_busy_timeout = int(*)(sqlite3*, int);
	func_sqlite3_busy_timeout _sqlite3_busy_timeout = sqlite3_busy_timeout;
	using func_sqlite3_last_insert_rowid = sqlite3_int64(*)(sqlite3*);
	func_sqlite3_last_insert_rowid _sqlite3_last_insert_rowid = sqlite3_last_insert_rowid;
	using func_sqlite3_get_autocommit = int(*)(sqlite3*);
	func_sqlite3_get_autocommit _sqlite3_get_autocommit = sqlite3_get_autocommit;
	
	using func_sqlite3_get_table = int(*)(sqlite3*, const char*, char***, int*, int*, char**);
	func_sqlite3_get_table _sqlite3_get_table = sqlite3_get_table;
	using func_sqlite3_free_table = void(*)(char**);
	func_sqlite3_free_table _sqlite3_free_table = sqlite3_free_table;
	
	using func_sqlite3_backup_init = sqlite3_backup * (*)(sqlite3*, const char*, sqlite3*, const char*);
	func_sqlite3_backup_init _sqlite3_backup_init = sqlite3_backup_init;
	using func_sqlite3_backup_step = int(*)(sqlite3_backup*, int);
	func_sqlite3_backup_step _sqlite3_backup_step = sqlite3_backup_step;
	using func_sqlite3_backup_finish = int(*)(sqlite3_backup*);
	func_sqlite3_backup_finish _sqlite3_backup_finish = sqlite3_backup_finish;
	
	using func_sqlite3_mprintf = char* (*)(const char*, ...);
	func_sqlite3_mprintf _sqlite3_mprintf = sqlite3_mprintf;
	using func_sqlite3_vmprintf = char* (*)(const char*, va_list);
	func_sqlite3_vmprintf _sqlite3_vmprintf = sqlite3_vmprintf;
	using func_sqlite3_free = void (*)(void*);
	func_sqlite3_free _sqlite3_free = sqlite3_free;



	bool init_mod(const char *mod)
	{
#ifdef _WIN32
		HMODULE mh = GetModuleHandleA(mod);
		if (mh == NULL)
			mh = LoadLibraryA(mod);
#define _get_sqlite3_function(lib, name)  GetProcAddress(lib, name)
#else
		void *mh = dlopen(mod, RTLD_NOW | RTLD_GLOBAL);
#define _get_sqlite3_function(lib, name)  dlsym(lib, name)
#endif // _WIN32
		if (mh == NULL)
			return false;

		func_sqlite3_column_count _tmp_sqlite3_column_count = (func_sqlite3_column_count)_get_sqlite3_function(mh, "sqlite3_column_count");
		if (_tmp_sqlite3_column_count == NULL)
			return false;
		func_sqlite3_column_name _tmp_sqlite3_column_name = (func_sqlite3_column_name)_get_sqlite3_function(mh, "sqlite3_column_name");
		if (_tmp_sqlite3_column_name == NULL)
			return false;
		func_sqlite3_column_type _tmp_sqlite3_column_type = (func_sqlite3_column_type)_get_sqlite3_function(mh, "sqlite3_column_type");
		if (_tmp_sqlite3_column_type == NULL)
			return false;
		func_sqlite3_column_decltype _tmp_sqlite3_column_decltype = (func_sqlite3_column_decltype)_get_sqlite3_function(mh, "sqlite3_column_decltype");
		if (_tmp_sqlite3_column_decltype == NULL)
			return false;
		func_sqlite3_column_text _tmp_sqlite3_column_text = (func_sqlite3_column_text)_get_sqlite3_function(mh, "sqlite3_column_text");
		if (_tmp_sqlite3_column_text == NULL)
			return false;
		func_sqlite3_column_int _tmp_sqlite3_column_int = (func_sqlite3_column_int)_get_sqlite3_function(mh, "sqlite3_column_int");
		if (_tmp_sqlite3_column_int == NULL)
			return false;
		func_sqlite3_column_int64 _tmp_sqlite3_column_int64 = (func_sqlite3_column_int64)_get_sqlite3_function(mh, "sqlite3_column_int64");
		if (_tmp_sqlite3_column_int64 == NULL)
			return false;
		func_sqlite3_column_double _tmp_sqlite3_column_double = (func_sqlite3_column_double)_get_sqlite3_function(mh, "sqlite3_column_double");
		if (_tmp_sqlite3_column_double == NULL)
			return false;
		func_sqlite3_column_blob _tmp_sqlite3_column_blob = (func_sqlite3_column_blob)_get_sqlite3_function(mh, "sqlite3_column_blob");
		if (_tmp_sqlite3_column_blob == NULL)
			return false;
		func_sqlite3_column_bytes _tmp_sqlite3_column_bytes = (func_sqlite3_column_bytes)_get_sqlite3_function(mh, "sqlite3_column_bytes");
		if (_tmp_sqlite3_column_bytes == NULL)
			return false;
		
		func_sqlite3_bind_text _tmp_sqlite3_bind_text = (func_sqlite3_bind_text)_get_sqlite3_function(mh, "sqlite3_bind_text");
		if (_tmp_sqlite3_bind_text == NULL)
			return false;
		func_sqlite3_bind_int _tmp_sqlite3_bind_int = (func_sqlite3_bind_int)_get_sqlite3_function(mh, "sqlite3_bind_int");
		if (_tmp_sqlite3_bind_int == NULL)
			return false;
		func_sqlite3_bind_int64 _tmp_sqlite3_bind_int64 = (func_sqlite3_bind_int64)_get_sqlite3_function(mh, "sqlite3_bind_int64");
		if (_tmp_sqlite3_bind_int64 == NULL)
			return false;
		func_sqlite3_bind_double _tmp_sqlite3_bind_double = (func_sqlite3_bind_double)_get_sqlite3_function(mh, "sqlite3_bind_double");
		if (_tmp_sqlite3_bind_double == NULL)
			return false;
		func_sqlite3_bind_blob _tmp_sqlite3_bind_blob = (func_sqlite3_bind_blob)_get_sqlite3_function(mh, "sqlite3_bind_blob");
		if (_tmp_sqlite3_bind_blob == NULL)
			return false;
		func_sqlite3_bind_null _tmp_sqlite3_bind_null = (func_sqlite3_bind_null)_get_sqlite3_function(mh, "sqlite3_bind_null");
		if (_tmp_sqlite3_bind_null == NULL)
			return false;
		
		func_sqlite3_bind_parameter_count _tmp_sqlite3_bind_parameter_count = (func_sqlite3_bind_parameter_count)_get_sqlite3_function(mh, "sqlite3_bind_parameter_count");
		if (_tmp_sqlite3_bind_parameter_count == NULL)
			return false;
		func_sqlite3_bind_parameter_index _tmp_sqlite3_bind_parameter_index = (func_sqlite3_bind_parameter_index)_get_sqlite3_function(mh, "sqlite3_bind_parameter_index");
		if (_tmp_sqlite3_bind_parameter_index == NULL)
			return false;
		func_sqlite3_bind_parameter_name _tmp_sqlite3_bind_parameter_name = (func_sqlite3_bind_parameter_name)_get_sqlite3_function(mh, "sqlite3_bind_parameter_name");
		if (_tmp_sqlite3_bind_parameter_name == NULL)
			return false;
		
		func_sqlite3_exec _tmp_sqlite3_exec = (func_sqlite3_exec)_get_sqlite3_function(mh, "sqlite3_exec");
		if (_tmp_sqlite3_exec == NULL)
			return false;
		func_sqlite3_step _tmp_sqlite3_step = (func_sqlite3_step)_get_sqlite3_function(mh, "sqlite3_step");
		if (_tmp_sqlite3_step == NULL)
			return false;
		func_sqlite3_changes _tmp_sqlite3_changes = (func_sqlite3_changes)_get_sqlite3_function(mh, "sqlite3_changes");
		if (_tmp_sqlite3_changes == NULL)
			return false;
		func_sqlite3_reset _tmp_sqlite3_reset = (func_sqlite3_reset)_get_sqlite3_function(mh, "sqlite3_reset");
		if (_tmp_sqlite3_reset == NULL)
			return false;
		func_sqlite3_finalize _tmp_sqlite3_finalize = (func_sqlite3_finalize)_get_sqlite3_function(mh, "sqlite3_finalize");
		if (_tmp_sqlite3_finalize == NULL)
			return false;
		func_sqlite3_errcode _tmp_sqlite3_errcode = (func_sqlite3_errcode)_get_sqlite3_function(mh, "sqlite3_errcode");
		if (_tmp_sqlite3_errcode == NULL)
			return false;
		func_sqlite3_errmsg _tmp_sqlite3_errmsg = (func_sqlite3_errmsg)_get_sqlite3_function(mh, "sqlite3_errmsg");
		if (_tmp_sqlite3_errmsg == NULL)
			return false;
		func_sqlite3_close _tmp_sqlite3_close = (func_sqlite3_close)_get_sqlite3_function(mh, "sqlite3_close");
		if (_tmp_sqlite3_close == NULL)
			return false;
		
		func_sqlite3_open_v2 _tmp_sqlite3_open_v2 = (func_sqlite3_open_v2)_get_sqlite3_function(mh, "sqlite3_open_v2");
		if (_tmp_sqlite3_open_v2 == NULL)
			return false;
		func_sqlite3_prepare_v2 _tmp_sqlite3_prepare_v2 = (func_sqlite3_prepare_v2)_get_sqlite3_function(mh, "sqlite3_prepare_v2");
		if (_tmp_sqlite3_prepare_v2 == NULL)
			return false;
		//这两个可能不存在，不校验返回值了
		func_sqlite3_key _tmp_sqlite3_key = (func_sqlite3_key)_get_sqlite3_function(mh, "sqlite3_key");
// 		if (_tmp_sqlite3_key == NULL)
// 			return false;
		func_sqlite3_rekey _tmp_sqlite3_rekey = (func_sqlite3_rekey)_get_sqlite3_function(mh, "sqlite3_rekey");
// 		if (_tmp_sqlite3_rekey == NULL)
// 			return false;
		
		func_sqlite3_busy_timeout _tmp_sqlite3_busy_timeout = (func_sqlite3_busy_timeout)_get_sqlite3_function(mh, "sqlite3_busy_timeout");
		if (_tmp_sqlite3_busy_timeout == NULL)
			return false;
		func_sqlite3_last_insert_rowid _tmp_sqlite3_last_insert_rowid = (func_sqlite3_last_insert_rowid)_get_sqlite3_function(mh, "sqlite3_last_insert_rowid");
		if (_tmp_sqlite3_last_insert_rowid == NULL)
			return false;
		func_sqlite3_get_autocommit _tmp_sqlite3_get_autocommit = (func_sqlite3_get_autocommit)_get_sqlite3_function(mh, "sqlite3_get_autocommit");
		if (_tmp_sqlite3_get_autocommit == NULL)
			return false;
		
		func_sqlite3_get_table _tmp_sqlite3_get_table = (func_sqlite3_get_table)_get_sqlite3_function(mh, "sqlite3_get_table");
		if (_tmp_sqlite3_get_table == NULL)
			return false;
		func_sqlite3_free_table _tmp_sqlite3_free_table = (func_sqlite3_free_table)_get_sqlite3_function(mh, "sqlite3_free_table");
		if (_tmp_sqlite3_free_table == NULL)
			return false;
		
		func_sqlite3_backup_init _tmp_sqlite3_backup_init = (func_sqlite3_backup_init)_get_sqlite3_function(mh, "sqlite3_backup_init");
		if (_tmp_sqlite3_backup_init == NULL)
			return false;
		func_sqlite3_backup_step _tmp_sqlite3_backup_step = (func_sqlite3_backup_step)_get_sqlite3_function(mh, "sqlite3_backup_step");
		if (_tmp_sqlite3_backup_step == NULL)
			return false;
		func_sqlite3_backup_finish _tmp_sqlite3_backup_finish = (func_sqlite3_backup_finish)_get_sqlite3_function(mh, "sqlite3_backup_finish");
		if (_tmp_sqlite3_backup_finish == NULL)
			return false;
		
		func_sqlite3_mprintf _tmp_sqlite3_mprintf = (func_sqlite3_mprintf)_get_sqlite3_function(mh, "sqlite3_mprintf");
		if (_tmp_sqlite3_mprintf == NULL)
			return false;
		func_sqlite3_vmprintf _tmp_sqlite3_vmprintf = (func_sqlite3_vmprintf)_get_sqlite3_function(mh, "sqlite3_vmprintf");
		if (_tmp_sqlite3_vmprintf == NULL)
			return false;
		func_sqlite3_free _tmp_sqlite3_free = (func_sqlite3_free)_get_sqlite3_function(mh, "sqlite3_free");
		if (_tmp_sqlite3_free == NULL)
			return false;
		
		_sqlite3_column_count = _tmp_sqlite3_column_count;
		_sqlite3_column_name = _tmp_sqlite3_column_name;
		_sqlite3_column_type = _tmp_sqlite3_column_type;
		_sqlite3_column_decltype = _tmp_sqlite3_column_decltype;
		_sqlite3_column_int = _tmp_sqlite3_column_int;
		_sqlite3_column_int64 = _tmp_sqlite3_column_int64;
		_sqlite3_column_double = _tmp_sqlite3_column_double;
		_sqlite3_column_text = _tmp_sqlite3_column_text;
		_sqlite3_column_blob = _tmp_sqlite3_column_blob;
		_sqlite3_column_bytes = _tmp_sqlite3_column_bytes;

		_sqlite3_bind_text = _tmp_sqlite3_bind_text;
		_sqlite3_bind_int = _tmp_sqlite3_bind_int;
		_sqlite3_bind_int64 = _tmp_sqlite3_bind_int64;
		_sqlite3_bind_double = _tmp_sqlite3_bind_double;
		_sqlite3_bind_blob = _tmp_sqlite3_bind_blob;
		_sqlite3_bind_null = _tmp_sqlite3_bind_null;

		_sqlite3_bind_parameter_count = _tmp_sqlite3_bind_parameter_count;
		_sqlite3_bind_parameter_index = _tmp_sqlite3_bind_parameter_index;
		_sqlite3_bind_parameter_name = _tmp_sqlite3_bind_parameter_name;
		
		_sqlite3_exec = _tmp_sqlite3_exec;
		_sqlite3_step = _tmp_sqlite3_step;
		_sqlite3_changes = _tmp_sqlite3_changes;
		_sqlite3_reset = _tmp_sqlite3_reset;
		_sqlite3_finalize = _tmp_sqlite3_finalize;
		_sqlite3_errcode = _tmp_sqlite3_errcode;
		_sqlite3_errmsg = _tmp_sqlite3_errmsg;
		_sqlite3_close = _tmp_sqlite3_close;
		
		_sqlite3_open_v2 = _tmp_sqlite3_open_v2;
		_sqlite3_prepare_v2 = _tmp_sqlite3_prepare_v2;
		_sqlite3_key = _tmp_sqlite3_key;
		_sqlite3_rekey = _tmp_sqlite3_rekey;
		
		_sqlite3_busy_timeout = _tmp_sqlite3_busy_timeout;
		_sqlite3_last_insert_rowid = _tmp_sqlite3_last_insert_rowid;
		_sqlite3_get_autocommit = _tmp_sqlite3_get_autocommit;

		_sqlite3_get_table = _tmp_sqlite3_get_table;
		_sqlite3_free_table = _tmp_sqlite3_free_table;
		
		_sqlite3_backup_init = _tmp_sqlite3_backup_init;
		_sqlite3_backup_step = _tmp_sqlite3_backup_step;
		_sqlite3_backup_finish = _tmp_sqlite3_backup_finish;

		_sqlite3_mprintf = _tmp_sqlite3_mprintf;
		_sqlite3_free = _tmp_sqlite3_free;
		_sqlite3_vmprintf = _tmp_sqlite3_vmprintf;
		
		return true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Prototypes for SQLite functions not included in SQLite DLL, but copied below
// from SQLite encode.c
////////////////////////////////////////////////////////////////////////////////
// int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
// int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

CppSQLite3Exception::CppSQLite3Exception(const int nErrCode,
									const char* szErrMess,
									bool bDeleteMsg/*=true*/) :
									mnErrCode(nErrCode)
{
	mpszErrMess = CppSQLite3::_sqlite3_mprintf("%s[%d]: %s",
								errorCodeAsString(nErrCode),
								nErrCode,
								szErrMess ? szErrMess : "");

	if (bDeleteMsg && szErrMess)
	{
		CppSQLite3::_sqlite3_free((void *)szErrMess);
	}
}

									
CppSQLite3Exception::CppSQLite3Exception(const CppSQLite3Exception&  e) :
									mnErrCode(e.mnErrCode)
{
	mpszErrMess = 0;
	if (e.mpszErrMess)
	{
		mpszErrMess = CppSQLite3::_sqlite3_mprintf("%s", e.mpszErrMess);
	}
}


const char* CppSQLite3Exception::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
		case SQLITE_OK          : return "SQLITE_OK";
		case SQLITE_ERROR       : return "SQLITE_ERROR";
		case SQLITE_INTERNAL    : return "SQLITE_INTERNAL";
		case SQLITE_PERM        : return "SQLITE_PERM";
		case SQLITE_ABORT       : return "SQLITE_ABORT";
		case SQLITE_BUSY        : return "SQLITE_BUSY";
		case SQLITE_LOCKED      : return "SQLITE_LOCKED";
		case SQLITE_NOMEM       : return "SQLITE_NOMEM";
		case SQLITE_READONLY    : return "SQLITE_READONLY";
		case SQLITE_INTERRUPT   : return "SQLITE_INTERRUPT";
		case SQLITE_IOERR       : return "SQLITE_IOERR";
		case SQLITE_CORRUPT     : return "SQLITE_CORRUPT";
		case SQLITE_NOTFOUND    : return "SQLITE_NOTFOUND";
		case SQLITE_FULL        : return "SQLITE_FULL";
		case SQLITE_CANTOPEN    : return "SQLITE_CANTOPEN";
		case SQLITE_PROTOCOL    : return "SQLITE_PROTOCOL";
		case SQLITE_EMPTY       : return "SQLITE_EMPTY";
		case SQLITE_SCHEMA      : return "SQLITE_SCHEMA";
		case SQLITE_TOOBIG      : return "SQLITE_TOOBIG";
		case SQLITE_CONSTRAINT  : return "SQLITE_CONSTRAINT";
		case SQLITE_MISMATCH    : return "SQLITE_MISMATCH";
		case SQLITE_MISUSE      : return "SQLITE_MISUSE";
		case SQLITE_NOLFS       : return "SQLITE_NOLFS";
		case SQLITE_AUTH        : return "SQLITE_AUTH";
		case SQLITE_FORMAT      : return "SQLITE_FORMAT";
		case SQLITE_RANGE       : return "SQLITE_RANGE";
		case SQLITE_ROW         : return "SQLITE_ROW";
		case SQLITE_DONE        : return "SQLITE_DONE";
		case CPPSQLITE_ERROR    : return "CPPSQLITE_ERROR";
		default: return "UNKNOWN_ERROR";
	}
}


CppSQLite3Exception::~CppSQLite3Exception()
{
	if (mpszErrMess)
	{
		CppSQLite3::_sqlite3_free(mpszErrMess);
		mpszErrMess = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Buffer::CppSQLite3Buffer()
{
	mpBuf = 0;
}


CppSQLite3Buffer::~CppSQLite3Buffer()
{
	clear();
}


void CppSQLite3Buffer::clear()
{
	if (mpBuf)
	{
		CppSQLite3::_sqlite3_free(mpBuf);
		mpBuf = 0;
	}

}

const char* CppSQLite3Buffer::formatva(const char* szFormat, va_list va)
{
	clear();
	mpBuf = CppSQLite3::_sqlite3_vmprintf(szFormat, va);
	return mpBuf;
}


const char* CppSQLite3Buffer::format(const char* szFormat, ...)
{
	clear();
	va_list va;
	va_start(va, szFormat);
	mpBuf = CppSQLite3::_sqlite3_vmprintf(szFormat, va);
	va_end(va);
	return mpBuf;
}


////////////////////////////////////////////////////////////////////////////////
// 
// CppSQLite3Binary::CppSQLite3Binary() :
// 						mpBuf(0),
// 						mnBinaryLen(0),
// 						mnBufferLen(0),
// 						mnEncodedLen(0),
// 						mbEncoded(false)
// {
// }
// 
// 
// CppSQLite3Binary::~CppSQLite3Binary()
// {
// 	clear();
// }
// 
// 
// void CppSQLite3Binary::setBinary(const unsigned char* pBuf, int nLen)
// {
// 	mpBuf = allocBuffer(nLen);
// 	memcpy(mpBuf, pBuf, nLen);
// }
// 
// 
// void CppSQLite3Binary::setEncoded(const unsigned char* pBuf)
// {
// 	clear();
// 
// 	mnEncodedLen = strlen((const char*)pBuf);
// 	mnBufferLen = mnEncodedLen + 1; // Allow for NULL terminator
// 
// 	mpBuf = (unsigned char*)malloc(mnBufferLen);
// 
// 	if (!mpBuf)
// 	{
// 		throw CppSQLite3Exception(CPPSQLITE_ERROR,
// 								"Cannot allocate memory",
// 								DONT_DELETE_MSG);
// 	}
// 
// 	memcpy(mpBuf, pBuf, mnBufferLen);
// 	mbEncoded = true;
// }
// 
// 
// const unsigned char* CppSQLite3Binary::getEncoded()
// {
// 	if (!mbEncoded)
// 	{
// 		unsigned char* ptmp = (unsigned char*)malloc(mnBinaryLen);
// 		memcpy(ptmp, mpBuf, mnBinaryLen);
// 		mnEncodedLen = sqlite3_encode_binary(ptmp, mnBinaryLen, mpBuf);
// 		free(ptmp);
// 		mbEncoded = true;
// 	}
// 
// 	return mpBuf;
// }
// 
// 
// const unsigned char* CppSQLite3Binary::getBinary()
// {
// 	if (mbEncoded)
// 	{
// 		// in/out buffers can be the same
// 		mnBinaryLen = sqlite3_decode_binary(mpBuf, mpBuf);
// 
// 		if (mnBinaryLen == -1)
// 		{
// 			throw CppSQLite3Exception(CPPSQLITE_ERROR,
// 									"Cannot decode binary",
// 									DONT_DELETE_MSG);
// 		}
// 
// 		mbEncoded = false;
// 	}
// 
// 	return mpBuf;
// }
// 
// 
// int CppSQLite3Binary::getBinaryLength()
// {
// 	getBinary();
// 	return mnBinaryLen;
// }
// 
// 
// unsigned char* CppSQLite3Binary::allocBuffer(int nLen)
// {
// 	clear();
// 
// 	// Allow extra space for encoded binary as per comments in
// 	// SQLite encode.c See bottom of this file for implementation
// 	// of SQLite functions use 3 instead of 2 just to be sure ;-)
// 	mnBinaryLen = nLen;
// 	mnBufferLen = 3 + (257*nLen)/254;
// 
// 	mpBuf = (unsigned char*)malloc(mnBufferLen);
// 
// 	if (!mpBuf)
// 	{
// 		throw CppSQLite3Exception(CPPSQLITE_ERROR,
// 								"Cannot allocate memory",
// 								DONT_DELETE_MSG);
// 	}
// 
// 	mbEncoded = false;
// 
// 	return mpBuf;
// }
// 
// 
// void CppSQLite3Binary::clear()
// {
// 	if (mpBuf)
// 	{
// 		mnBinaryLen = 0;
// 		mnBufferLen = 0;
// 		free(mpBuf);
// 		mpBuf = 0;
// 	}
// }


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Query::CppSQLite3Query()
{
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}


CppSQLite3Query::CppSQLite3Query(const CppSQLite3Query& rQuery)
{
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;

	mapNameIndex = rQuery.mapNameIndex;
}


CppSQLite3Query::CppSQLite3Query(sqlite3* pDB,
							sqlite3_stmt* pVM,
							bool bEof,
							bool bOwnVM/*=true*/)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = CppSQLite3::_sqlite3_column_count(mpVM);
	mbOwnVM = bOwnVM;

	mapNameIndex.clear();
}


CppSQLite3Query::~CppSQLite3Query()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Query& CppSQLite3Query::operator=(const CppSQLite3Query& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3Query&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	mapNameIndex = rQuery.mapNameIndex;
	return *this;
}


int CppSQLite3Query::numFields()
{
	checkVM();
	return mnCols;
}


const char* CppSQLite3Query::fieldValue(int nField)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return (const char*)CppSQLite3::_sqlite3_column_text(mpVM, nField);
}


const char* CppSQLite3Query::fieldValue(const char* szField)
{
	int nField = fieldIndex(szField);
	return (const char*)CppSQLite3::_sqlite3_column_text(mpVM, nField);
}


int CppSQLite3Query::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return CppSQLite3::_sqlite3_column_int(mpVM, nField);
	}
}


int CppSQLite3Query::getIntField(const char* szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}


unsigned int CppSQLite3Query::getUIntField(int nField, unsigned int nNullValue/*=0*/)
{
	return (unsigned int)getIntField(nField, nNullValue);
}


unsigned int CppSQLite3Query::getUIntField(const char* szField, unsigned int nNullValue /*= 0*/)
{
	return (unsigned int)getIntField(szField, nNullValue);
}

sqlite_int64 CppSQLite3Query::getInt64Field(int nField, sqlite_int64 nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return CppSQLite3::_sqlite3_column_int64(mpVM, nField);
	}
}


sqlite_int64 CppSQLite3Query::getInt64Field(const char* szField, sqlite_int64 nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getInt64Field(nField, nNullValue);
}

sqlite_uint64 CppSQLite3Query::getUInt64Field(int nField, sqlite_uint64 nNullValue/*=0*/)
{
	return (sqlite_uint64)getInt64Field(nField, nNullValue);
}

sqlite_uint64 CppSQLite3Query::getUInt64Field(const char* szField, sqlite_uint64 nNullValue/*=0*/)
{
	return (sqlite_uint64)getInt64Field(szField, nNullValue);
}

double CppSQLite3Query::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return CppSQLite3::_sqlite3_column_double(mpVM, nField);
	}
}


double CppSQLite3Query::getFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);
	return getFloatField(nField, fNullValue);
}


const char* CppSQLite3Query::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (const char*)CppSQLite3::_sqlite3_column_text(mpVM, nField);
	}
}


const char* CppSQLite3Query::getStringField(const char* szField, const char* szNullValue/*=""*/)
{
	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}


const unsigned char* CppSQLite3Query::getBlobField(int nField, int& nLen)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	nLen = CppSQLite3::_sqlite3_column_bytes(mpVM, nField);
	return (const unsigned char*)CppSQLite3::_sqlite3_column_blob(mpVM, nField);
}


const unsigned char* CppSQLite3Query::getBlobField(const char* szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}

std::string CppSQLite3Query::getBlobField(const char* szField)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField);
}

std::string CppSQLite3Query::getBlobField(int nField)
{
	std::string strRet;
	int nLen = 0;
	const unsigned char *data = getBlobField(nField, nLen);
	if (nLen)
		strRet.assign((const char *)data, nLen);

	return strRet;
}

bool CppSQLite3Query::fieldIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3Query::fieldIsNull(const char* szField)
{
	int nField = fieldIndex(szField);
	return (fieldDataType(nField) == SQLITE_NULL);
}


int CppSQLite3Query::fieldIndex(const char* szField)
{
	checkVM();

	if (szField)
	{
		auto iter = mapNameIndex.find(szField);
		if (iter != mapNameIndex.end())
			return iter->second;
		
		for (int nField = (int)mapNameIndex.size(); nField < mnCols; nField++)
		{
			const char* szTemp = CppSQLite3::_sqlite3_column_name(mpVM, nField);
			mapNameIndex.insert(std::make_pair(szTemp, nField));

			if (strcmp(szField, szTemp) == 0)
				return nField;
		}
	}

	throw CppSQLite3Exception(CPPSQLITE_ERROR,
							"Invalid field name requested",
							DONT_DELETE_MSG);
}


const char* CppSQLite3Query::fieldName(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CppSQLite3::_sqlite3_column_name(mpVM, nCol);
}


const char* CppSQLite3Query::fieldDeclType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CppSQLite3::_sqlite3_column_decltype(mpVM, nCol);
}


int CppSQLite3Query::fieldDataType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CppSQLite3::_sqlite3_column_type(mpVM, nCol);
}


bool CppSQLite3Query::eof()
{
	checkVM();
	return mbEof;
}


void CppSQLite3Query::nextRow()
{
	checkVM();

	int nRet = CppSQLite3::_sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = CppSQLite3::_sqlite3_finalize(mpVM);
		mpVM = 0;
		//const char* szError = sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet,
								(char*)"nextRow Err",
								DONT_DELETE_MSG);
	}
}


void CppSQLite3Query::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = CppSQLite3::_sqlite3_finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3Query::checkVM()
{
	if (mpVM == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Null Virtual Machine pointer",
								DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Table::CppSQLite3Table()
{
	mpaszResults = 0;
	mnRows = 0;
	mnCols = 0;
	mnCurrentRow = 0;
}


CppSQLite3Table::CppSQLite3Table(const CppSQLite3Table& rTable)
{
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
}


CppSQLite3Table::CppSQLite3Table(char** paszResults, int nRows, int nCols)
{
	mpaszResults = paszResults;
	mnRows = nRows;
	mnCols = nCols;
	mnCurrentRow = 0;
}


CppSQLite3Table::~CppSQLite3Table()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Table& CppSQLite3Table::operator=(const CppSQLite3Table& rTable)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
	return *this;
}


void CppSQLite3Table::finalize()
{
	if (mpaszResults)
	{
		CppSQLite3::_sqlite3_free_table(mpaszResults);
		mpaszResults = 0;
	}
}


int CppSQLite3Table::numFields()
{
	checkResults();
	return mnCols;
}


int CppSQLite3Table::numRows()
{
	checkResults();
	return mnRows;
}


const char* CppSQLite3Table::fieldValue(int nField)
{
	checkResults();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
	return mpaszResults[nIndex];
}


const char* CppSQLite3Table::fieldValue(const char* szField)
{
	checkResults();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			if (strcmp(szField, mpaszResults[nField]) == 0)
			{
				int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
				return mpaszResults[nIndex];
			}
		}
	}

	throw CppSQLite3Exception(CPPSQLITE_ERROR,
							"Invalid field name requested",
							DONT_DELETE_MSG);
}


int CppSQLite3Table::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(fieldValue(nField));
	}
}


int CppSQLite3Table::getIntField(const char* szField, int nNullValue/*=0*/)
{
	if (fieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(fieldValue(szField));
	}
}


double CppSQLite3Table::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(nField))
	{
		return fNullValue;
	}
	else
	{
		return atof(fieldValue(nField));
	}
}


double CppSQLite3Table::getFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(szField))
	{
		return fNullValue;
	}
	else
	{
		return atof(fieldValue(szField));
	}
}


const char* CppSQLite3Table::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if (fieldIsNull(nField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(nField);
	}
}


const char* CppSQLite3Table::getStringField(const char* szField, const char* szNullValue/*=""*/)
{
	if (fieldIsNull(szField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(szField);
	}
}


bool CppSQLite3Table::fieldIsNull(int nField)
{
	checkResults();
	return (fieldValue(nField) == 0);
}


bool CppSQLite3Table::fieldIsNull(const char* szField)
{
	checkResults();
	return (fieldValue(szField) == 0);
}


const char* CppSQLite3Table::fieldName(int nCol)
{
	checkResults();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return mpaszResults[nCol];
}


void CppSQLite3Table::setRow(int nRow)
{
	checkResults();

	if (nRow < 0 || nRow > mnRows-1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid row index requested",
								DONT_DELETE_MSG);
	}

	mnCurrentRow = nRow;
}


void CppSQLite3Table::checkResults()
{
	if (mpaszResults == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Null Results pointer",
								DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Statement::CppSQLite3Statement()
{
	mpDB = 0;
	mpVM = 0;
}


CppSQLite3Statement::CppSQLite3Statement(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
}


CppSQLite3Statement::CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}


CppSQLite3Statement::~CppSQLite3Statement()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Statement& CppSQLite3Statement::operator=(const CppSQLite3Statement& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3Statement&>(rStatement).mpVM = 0;
	return *this;
}


int CppSQLite3Statement::execDML()
{
	checkDB();
	checkVM();

	const char* szError=0;

	int nRet = CppSQLite3::_sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = CppSQLite3::_sqlite3_changes(mpDB);

		nRet = CppSQLite3::_sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			szError = CppSQLite3::_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
		}

		return nRowsChanged;
	}
	else
	{
		nRet = CppSQLite3::_sqlite3_reset(mpVM);
		szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}


CppSQLite3Query CppSQLite3Statement::execQuery()
{
	checkDB();
	checkVM();

	int nRet = CppSQLite3::_sqlite3_step(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, mpVM, true/*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, mpVM, false/*eof*/, false);
	}
	else
	{
		nRet = CppSQLite3::_sqlite3_reset(mpVM);
		const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::bind(int nParam, const char* szValue)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_text(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								"Error binding string param",
								DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::bind(int nParam, sqlite3_int64 nValue)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_int64(mpVM, nParam, nValue);
	
	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
			"Error binding int64 param",
			DONT_DELETE_MSG);
	}
}



void CppSQLite3Statement::bind(int nParam, const int nValue)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_int(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								"Error binding int param",
								DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::bind(int nParam, const double dValue)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_double(mpVM, nParam, dValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								"Error binding double param",
								DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::bind(int nParam, const void* blobValue, int nLen)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_blob(mpVM, nParam, blobValue, nLen, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								"Error binding blob param",
								DONT_DELETE_MSG);
	}
}

	
void CppSQLite3Statement::bindNull(int nParam)
{
	checkVM();
	int nRes = CppSQLite3::_sqlite3_bind_null(mpVM, nParam);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes,
								"Error binding NULL param",
								DONT_DELETE_MSG);
	}
}


int CppSQLite3Statement::bindParameterIndex(const char* szParam)
{
	checkVM();

	int nParam = CppSQLite3::_sqlite3_bind_parameter_index(mpVM, szParam);

// 	int nn = CppSQLite3::_sqlite3_bind_parameter_count(mpVM);
// 	const char* sz1 = CppSQLite3::_sqlite3_bind_parameter_name(mpVM, 1);
// 	const char* sz2 = CppSQLite3::_sqlite3_bind_parameter_name(mpVM, 2);

	if (!nParam)
	{
		CppSQLite3Buffer buffer;
		buffer.format("Parameter '%s' is not valid for this statement", szParam);
		throw CppSQLite3Exception(CPPSQLITE_ERROR, buffer, DONT_DELETE_MSG);
	}

	return nParam;
}


void CppSQLite3Statement::bind(const char* szParam, const char* szValue)
{
	int nParam = bindParameterIndex(szParam);
	bind(nParam, szValue);
}


void CppSQLite3Statement::bind(const char* szParam, const int nValue)
{
	int nParam = bindParameterIndex(szParam);
	bind(nParam, nValue);
}

void CppSQLite3Statement::bind(const char* szParam, const double dwValue)
{
	int nParam = bindParameterIndex(szParam);
	bind(nParam, dwValue);
}

void CppSQLite3Statement::bind(const char* szParam, const void* blobValue, int nLen)
{
	int nParam = bindParameterIndex(szParam);
	bind(nParam, blobValue, nLen);
}

void CppSQLite3Statement::bind(const char* szParam, sqlite3_int64 nValue)
{
	int nParam = bindParameterIndex(szParam);
	bind(nParam, nValue);
}

void CppSQLite3Statement::bindNull(const char* szParam)
{
	int nParam = bindParameterIndex(szParam);
	bindNull(nParam);
}


void CppSQLite3Statement::reset()
{
	if (mpVM)
	{
		int nRet = CppSQLite3::_sqlite3_reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3Statement::finalize()
{
	if (mpVM)
	{
		int nRet = CppSQLite3::_sqlite3_finalize(mpVM);
		mpVM = 0;

		if (nRet != SQLITE_OK)
		{
			const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
			throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3Statement::checkDB()
{
	if (mpDB == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Database not open",
								DONT_DELETE_MSG);
	}
}


void CppSQLite3Statement::checkVM()
{
	if (mpVM == 0)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Null Virtual Machine pointer",
								DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3DB::CppSQLite3DB()
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DB::CppSQLite3DB(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DB::~CppSQLite3DB()
{
	try
	{
		close();
	}
	catch (...)
	{
	}
}


CppSQLite3DB& CppSQLite3DB::operator=(const CppSQLite3DB& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}


void CppSQLite3DB::open(const char* szFile, bool readonly /*= false*/)
{
	int flags;
	if (readonly)
		flags = SQLITE_OPEN_READONLY;
	else
		flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

	int nRet = CppSQLite3::_sqlite3_open_v2(szFile, &mpDB, flags, 0);

	if (nRet != SQLITE_OK)
	{
		const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}

	setBusyTimeout(mnBusyTimeoutMs);
}


void CppSQLite3DB::Key(const void *pKey, int nKey)
{
	if (CppSQLite3::_sqlite3_key == nullptr)
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "sqlite3_key not supported", DONT_DELETE_MSG);
	
	checkDB();

	int nRet = CppSQLite3::_sqlite3_key(mpDB, pKey, nKey);
	if (nRet != SQLITE_OK)
	{
		const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}

void CppSQLite3DB::Rekey(const void* pKey, int nKey)
{
	if (CppSQLite3::_sqlite3_rekey == nullptr)
		throw CppSQLite3Exception(CPPSQLITE_ERROR, "sqlite3_rekey not supported", DONT_DELETE_MSG);
	
	checkDB();

	int nRet = CppSQLite3::_sqlite3_rekey(mpDB, pKey, nKey);
	if (nRet != SQLITE_OK)
	{
		const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}


void CppSQLite3DB::close()
{
	if (mpDB)
	{
		if (CppSQLite3::_sqlite3_close(mpDB) == SQLITE_OK)
		{
			mpDB = 0;
		}
		else
		{
			throw CppSQLite3Exception(CPPSQLITE_ERROR,
									"Unable to close database",
									DONT_DELETE_MSG);
		}
	}
}


CppSQLite3Statement CppSQLite3DB::compileStatement(const char* szSQL)
{
	checkDB();

	sqlite3_stmt* pVM = compile(szSQL);
	return CppSQLite3Statement(mpDB, pVM);
}


bool CppSQLite3DB::tableExists(const char* szTable)
{
	int nRet = execScalarFormat("select count(*) from sqlite_master where type='table' and name='%s'", szTable);
	return (nRet > 0);
}

std::string CppSQLite3DB::getTableSQL(const char* szTable)
{
	std::string tablesql;
	CppSQLite3Query query = execQueryFormat("select sql from sqlite_master where type='table' and name='%s'", szTable);
	if (!query.eof())
		tablesql = query.getStringField(0);
	return tablesql;
}

int CppSQLite3DB::execDMLFormat(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	CppSQLite3Buffer buffer;
	buffer.formatva(fmt, va);
	va_end(va);

	return execDML(buffer);
}

int CppSQLite3DB::execDML(const char* szSQL)
{
	checkDB();

	char* szError=0;

	int nRet = CppSQLite3::_sqlite3_exec(mpDB, szSQL, 0, 0, &szError);

	if (nRet == SQLITE_OK)
	{
		return CppSQLite3::_sqlite3_changes(mpDB);
	}
	else
	{
		throw CppSQLite3Exception(nRet, szError);
	}
}

sqlite3* CppSQLite3DB::getDatabaseHandle()
{
	checkDB();

	return mpDB;
}


void CppSQLite3DB::backupDB(const char *dbpath)
{
	CppSQLite3DB mBackupDB;
	mBackupDB.open(dbpath);
	backupDB(mBackupDB);
}


void CppSQLite3DB::backupDB(CppSQLite3DB &mTargetDB)
{
	int rc = SQLITE_ERROR;
	sqlite3_backup* db_backup = CppSQLite3::_sqlite3_backup_init(mTargetDB.getDatabaseHandle(), "main", getDatabaseHandle(), "main");
	if(NULL != db_backup)
	{
		rc = CppSQLite3::_sqlite3_backup_step(db_backup, -1);
		rc = CppSQLite3::_sqlite3_backup_finish(db_backup);
		if (rc != SQLITE_OK)
			throw CppSQLite3Exception(CPPSQLITE_ERROR,
			"Unable to backup",
			DONT_DELETE_MSG);
	}
	else
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
			"Unable to init backup",
			DONT_DELETE_MSG);
	}
}


CppSQLite3Query CppSQLite3DB::execQueryFormat(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	CppSQLite3Buffer buffer;
	buffer.formatva(fmt, va);
	va_end(va);

	return execQuery(buffer);
}


CppSQLite3Query CppSQLite3DB::execQuery(const char* szSQL)
{
	checkDB();

	sqlite3_stmt* pVM = compile(szSQL);

	int nRet = CppSQLite3::_sqlite3_step(pVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(mpDB, pVM, true/*eof*/);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(mpDB, pVM, false/*eof*/);
	}
	else
	{
		nRet = CppSQLite3::_sqlite3_finalize(pVM);
		const char* szError= CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet, (char*)szError, DONT_DELETE_MSG);
	}
}

int CppSQLite3DB::execScalarFormat(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	CppSQLite3Buffer buffer;
	buffer.formatva(fmt, va);
	va_end(va);

	return execScalar(buffer);
}

int CppSQLite3DB::execScalar(const char* szSQL, int nNullValue/*=0*/)
{
	CppSQLite3Query q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Invalid scalar query",
								DONT_DELETE_MSG);
	}

	return q.getIntField(0, nNullValue);
}



CppSQLite3Table CppSQLite3DB::getTable(const char* szSQL)
{
	checkDB();

	char* szError=0;
	char** paszResults=0;
	int nRet;
	int nRows(0);
	int nCols(0);

	nRet = CppSQLite3::_sqlite3_get_table(mpDB, szSQL, &paszResults, &nRows, &nCols, &szError);

	if (nRet == SQLITE_OK)
	{
		return CppSQLite3Table(paszResults, nRows, nCols);
	}
	else
	{
		throw CppSQLite3Exception(nRet, szError);
	}
}


sqlite_int64 CppSQLite3DB::lastRowId()
{
	return CppSQLite3::_sqlite3_last_insert_rowid(mpDB);
}


void CppSQLite3DB::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	CppSQLite3::_sqlite3_busy_timeout(mpDB, mnBusyTimeoutMs);
}


void CppSQLite3DB::checkDB()
{
	if (!mpDB)
	{
		throw CppSQLite3Exception(CPPSQLITE_ERROR,
								"Database not open",
								DONT_DELETE_MSG);
	}
}

sqlite3_stmt* CppSQLite3DB::compile(const char* szSQL)
{
	checkDB();

	const char* szTail=0;
	sqlite3_stmt* pVM;

	int nRet = CppSQLite3::_sqlite3_prepare_v2(mpDB, szSQL, -1, &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{
		const char* szError = CppSQLite3::_sqlite3_errmsg(mpDB);
		throw CppSQLite3Exception(nRet,
								(char*)szError,
								DONT_DELETE_MSG);
	}

	return pVM;
}

bool CppSQLite3DB::IsAutoCommitOn()
{
	checkDB();
	return CppSQLite3::_sqlite3_get_autocommit(mpDB) ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// SQLite encode.c reproduced here, containing implementation notes and source
// for sqlite3_encode_binary() and sqlite3_decode_binary() 
////////////////////////////////////////////////////////////////////////////////

/*
** 2002 April 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains helper routines used to translate binary data into
** a null-terminated string (suitable for use in SQLite) and back again.
** These are convenience routines for use by people who want to store binary
** data in an SQLite database.  The code in this file is not used by any other
** part of the SQLite library.
**
** $Id: encode.c,v 1.10 2004/01/14 21:59:23 drh Exp $
*/

/*
** How This Encoder Works
**
** The output is allowed to contain any character except 0x27 (') and
** 0x00.  This is accomplished by using an escape character to encode
** 0x27 and 0x00 as a two-byte sequence.  The escape character is always
** 0x01.  An 0x00 is encoded as the two byte sequence 0x01 0x01.  The
** 0x27 character is encoded as the two byte sequence 0x01 0x03.  Finally,
** the escape character itself is encoded as the two-character sequence
** 0x01 0x02.
**
** To summarize, the encoder works by using an escape sequences as follows:
**
**       0x00  ->  0x01 0x01
**       0x01  ->  0x01 0x02
**       0x27  ->  0x01 0x03
**
** If that were all the encoder did, it would work, but in certain cases
** it could double the size of the encoded string.  For example, to
** encode a string of 100 0x27 characters would require 100 instances of
** the 0x01 0x03 escape sequence resulting in a 200-character output.
** We would prefer to keep the size of the encoded string smaller than
** this.
**
** To minimize the encoding size, we first add a fixed offset value to each 
** byte in the sequence.  The addition is modulo 256.  (That is to say, if
** the sum of the original character value and the offset exceeds 256, then
** the higher order bits are truncated.)  The offset is chosen to minimize
** the number of characters in the string that need to be escaped.  For
** example, in the case above where the string was composed of 100 0x27
** characters, the offset might be 0x01.  Each of the 0x27 characters would
** then be converted into an 0x28 character which would not need to be
** escaped at all and so the 100 character input string would be converted
** into just 100 characters of output.  Actually 101 characters of output - 
** we have to record the offset used as the first byte in the sequence so
** that the string can be decoded.  Since the offset value is stored as
** part of the output string and the output string is not allowed to contain
** characters 0x00 or 0x27, the offset cannot be 0x00 or 0x27.
**
** Here, then, are the encoding steps:
**
**     (1)   Choose an offset value and make it the first character of
**           output.
**
**     (2)   Copy each input character into the output buffer, one by
**           one, adding the offset value as you copy.
**
**     (3)   If the value of an input character plus offset is 0x00, replace
**           that one character by the two-character sequence 0x01 0x01.
**           If the sum is 0x01, replace it with 0x01 0x02.  If the sum
**           is 0x27, replace it with 0x01 0x03.
**
**     (4)   Put a 0x00 terminator at the end of the output.
**
** Decoding is obvious:
**
**     (5)   Copy encoded characters except the first into the decode 
**           buffer.  Set the first encoded character aside for use as
**           the offset in step 7 below.
**
**     (6)   Convert each 0x01 0x01 sequence into a single character 0x00.
**           Convert 0x01 0x02 into 0x01.  Convert 0x01 0x03 into 0x27.
**
**     (7)   Subtract the offset value that was the first character of
**           the encoded buffer from all characters in the output buffer.
**
** The only tricky part is step (1) - how to compute an offset value to
** minimize the size of the output buffer.  This is accomplished by testing
** all offset values and picking the one that results in the fewest number
** of escapes.  To do that, we first scan the entire input and count the
** number of occurances of each character value in the input.  Suppose
** the number of 0x00 characters is N(0), the number of occurances of 0x01
** is N(1), and so forth up to the number of occurances of 0xff is N(255).
** An offset of 0 is not allowed so we don't have to test it.  The number
** of escapes required for an offset of 1 is N(1)+N(2)+N(40).  The number
** of escapes required for an offset of 2 is N(2)+N(3)+N(41).  And so forth.
** In this way we find the offset that gives the minimum number of escapes,
** and thus minimizes the length of the output string.
*/

/*
** Encode a binary buffer "in" of size n bytes so that it contains
** no instances of characters '\'' or '\000'.  The output is 
** null-terminated and can be used as a string value in an INSERT
** or UPDATE statement.  Use sqlite3_decode_binary() to convert the
** string back into its original binary.
**
** The result is written into a preallocated output buffer "out".
** "out" must be able to hold at least 2 +(257*n)/254 bytes.
** In other words, the output will be expanded by as much as 3
** bytes for every 254 bytes of input plus 2 bytes of fixed overhead.
** (This is approximately 2 + 1.0118*n or about a 1.2% size increase.)
**
** The return value is the number of characters in the encoded
** string, excluding the "\000" terminator.
// */
// int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out){
//   int i, j, e, m;
//   int cnt[256];
//   if( n<=0 ){
//     out[0] = 'x';
//     out[1] = 0;
//     return 1;
//   }
//   memset(cnt, 0, sizeof(cnt));
//   for(i=n-1; i>=0; i--){ cnt[in[i]]++; }
//   m = n;
//   for(i=1; i<256; i++){
//     int sum;
//     if( i=='\'' ) continue;
//     sum = cnt[i] + cnt[(i+1)&0xff] + cnt[(i+'\'')&0xff];
//     if( sum<m ){
//       m = sum;
//       e = i;
//       if( m==0 ) break;
//     }
//   }
//   out[0] = e;
//   j = 1;
//   for(i=0; i<n; i++){
//     int c = (in[i] - e)&0xff;
//     if( c==0 ){
//       out[j++] = 1;
//       out[j++] = 1;
//     }else if( c==1 ){
//       out[j++] = 1;
//       out[j++] = 2;
//     }else if( c=='\'' ){
//       out[j++] = 1;
//       out[j++] = 3;
//     }else{
//       out[j++] = c;
//     }
//   }
//   out[j] = 0;
//   return j;
// }
// 
// /*
// ** Decode the string "in" into binary data and write it into "out".
// ** This routine reverses the encoding created by sqlite3_encode_binary().
// ** The output will always be a few bytes less than the input.  The number
// ** of bytes of output is returned.  If the input is not a well-formed
// ** encoding, -1 is returned.
// **
// ** The "in" and "out" parameters may point to the same buffer in order
// ** to decode a string in place.
// */
// int sqlite3_decode_binary(const unsigned char *in, unsigned char *out){
//   int i, c, e;
//   e = *(in++);
//   i = 0;
//   while( (c = *(in++))!=0 ){
//     if( c==1 ){
//       c = *(in++);
//       if( c==1 ){
//         c = 0;
//       }else if( c==2 ){
//         c = 1;
//       }else if( c==3 ){
//         c = '\'';
//       }else{
//         return -1;
//       }
//     }
//     out[i++] = (c + e)&0xff;
//   }
//   return i;
// }
