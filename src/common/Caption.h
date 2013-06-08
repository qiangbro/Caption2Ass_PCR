//------------------------------------------------------------------------------
// Caption.h
//------------------------------------------------------------------------------
#ifndef __CAPTION_H__
#define __CAPTION_H__

#include "CaptionDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CAPTION_EXPORTS

//DLLの初期化
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI InitializeCP();

//DLLの初期化 UNICODE対応
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI InitializeUNICODE();

//DLLの開放
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI UnInitializeCP();

//188バイトTS1パケット
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI AddTSPacketCP(BYTE* pbPacket);

//内部データクリア
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI ClearCP();

//字幕情報取得
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI GetTagInfoCP(LANG_TAG_INFO_DLL** ppList, DWORD* pdwListCount);

//字幕データ本文取得
//戻り値：エラーコード
__declspec(dllexport)
DWORD WINAPI GetCaptionDataCP(unsigned char ucLangTag, CAPTION_DATA_DLL** ppList, DWORD* pdwListCount);

#else /* CAPTION_EXPORTS */

typedef DWORD (WINAPI* InitializeCP)();

typedef DWORD (WINAPI* InitializeUNICODECP)();

typedef DWORD (WINAPI* UnInitializeCP)();

typedef DWORD (WINAPI* AddTSPacketCP)(BYTE* pbPacket);

typedef DWORD (WINAPI* ClearCP)();

typedef DWORD (WINAPI* GetTagInfoCP)(LANG_TAG_INFO_DLL** ppList, DWORD* pdwListCount);

typedef DWORD (WINAPI* GetCaptionDataCP)(unsigned char ucLangTag, CAPTION_DATA_DLL** ppList, DWORD* pdwListCount);

#endif /* CAPTION_EXPORTS */

#ifdef __cplusplus
}
#endif

#endif // __CAPTION_H__
