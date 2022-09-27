
/********************************************************************************
 * 
 * $Id: common.cpp,v 2.0 2002/05/03 02:17:48 dvi175 Exp $
 *
 * File: Common.cpp
 *
 * Copyright (c) 2000-2002 Davicom Inc.  All rights reserved.
 *
 ********************************************************************************/

#include	"common.h"


PEXCEPTION_DATA	_gpExpData;

#ifdef	PERFORMANCE_LOG
int			_gnPerfLogPos=0;
LARGE_INTEGER	_gPerfTemp;
PERF_TYPE	_gszPerfLogs[MAX_PERF_LOGS];

void	DumpPerfmanceLogs(void)
{
	int	i;

	NKDbgPrintfW(
			TEXT("Current Perf Pos = %d\n"), _gnPerfLogPos);

	for(i=0;i<MAX_PERF_LOGS;i++)
	{
		if(_gszPerfLogs[i].dwLabel&PROBE_ON)
			NKDbgPrintfW(
				TEXT("%02x: %08d On\n"), 
				_gszPerfLogs[i].dwLabel&0xffff,
				_gszPerfLogs[i].dwCounter);
		else
			NKDbgPrintfW(
				TEXT("%02x: %08d Off\n"), 
				_gszPerfLogs[i].dwLabel,
				_gszPerfLogs[i].dwCounter);

	}
}
#endif
