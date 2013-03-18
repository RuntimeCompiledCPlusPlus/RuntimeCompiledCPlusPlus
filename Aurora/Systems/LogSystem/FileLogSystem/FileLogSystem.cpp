//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "FileLogSystem.h"

// Currently we create the file on first real output, and only close it on shutdown

#include <stdarg.h>
#include <assert.h>

#pragma warning( disable : 4996 4800 )


FileLogSystem::FileLogSystem(void)
{
	m_fp = NULL;
	m_eVerbosity = eLV_EVENTS;
}

FileLogSystem::~FileLogSystem(void)
{
	CloseFile();
}

bool FileLogSystem::SetLogPath( const char * sPath, bool bTest )
{
	m_sPath = sPath;

	if (!bTest) return true;
	if (!OpenFile()) return false;
	if (!CloseFile()) return false;

	return true;
}

ELogVerbosity FileLogSystem::GetVerbosity() const
{
	return m_eVerbosity;
}

void FileLogSystem::SetVerbosity(ELogVerbosity eVerbosity)
{
	m_eVerbosity = eVerbosity;
}

FileLogSystem::TVerbosityPeeker FileLogSystem::GetVerbosityPeeker() const
{
	return (&m_eVerbosity);
}

void FileLogSystem::Log(ELogVerbosity eVerbosity, const char * format, ...)
{
	va_list args;
	va_start(args, format);
	LogInternal(eVerbosity, format, args);
}

void FileLogSystem::LogVa(va_list args, ELogVerbosity eVerbosity, const char * format)
{
	LogInternal(eVerbosity, format, args);
}

void FileLogSystem::LogInternal(ELogVerbosity eVerbosity, const char * format, va_list args)
{
	if (eVerbosity > m_eVerbosity || eVerbosity == eLV_NEVER) return;
	
	if (!m_fp) OpenFile();
	if (!m_fp) return;

	int result = vsnprintf(m_buff, LOGSYSTEM_MAX_BUFFER-1, format, args);
	// Make sure there's a limit to the amount of rubbish we can output
	m_buff[LOGSYSTEM_MAX_BUFFER-1] = '\0';
	fprintf(m_fp, "%s", m_buff);
	fflush(m_fp);
}

bool FileLogSystem::OpenFile()
{
	m_fp = fopen(m_sPath.c_str(),"wt");
	return m_fp;
}

bool FileLogSystem::CloseFile()
{
	if (!m_fp) return false;
	bool bResult = fclose(m_fp);
	m_fp = NULL;
	return bResult;
}
