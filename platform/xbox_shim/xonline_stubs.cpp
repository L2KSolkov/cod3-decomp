#include "xlive.h"

extern "C" {

HRESULT __stdcall XOnlineMatchSearch(
    DWORD, DWORD, DWORD, const XONLINE_ATTRIBUTE*, DWORD, HANDLE,
    XONLINETASK_HANDLE* phTask)
{
    if (phTask != NULL)
        *phTask = NULL;
    return E_FAIL;
}
HRESULT __stdcall XOnlineMatchSearchGetResults(
    XONLINETASK_HANDLE, _XONLINE_MATCH_SEARCHRESULT** prgpSearchResults,
    DWORD* pdwReturnedResults)
{
    if (prgpSearchResults != NULL)
        *prgpSearchResults = NULL;
    if (pdwReturnedResults != NULL)
        *pdwReturnedResults = 0;
    return E_FAIL;
}

HRESULT __stdcall XOnlineMatchSearchParse(
    _XONLINE_MATCH_SEARCHRESULT*, DWORD, const void*, void*)
{
    return E_FAIL;
}

HRESULT __stdcall XOnlineMatchSearchResultsLen(
    DWORD, DWORD, const void*)
{
    return E_FAIL;
}

}
