
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#include "packet_types.h"

VOID _tMyPrintf(IN	LPCTSTR tracemsg, ...);

BOOL FindStartOffset(FILE *fp)
{
	BYTE buf[188 * 2] = {0};

	while (fread(buf, 188 * 2, 1, fp) == 1) {

		for (int i = 0; i < 188; i++) {
			if (buf[i] == 'G' && buf[i+188] == 'G') {
				fseek(fp, i, SEEK_SET);
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

BOOL resync(BYTE *pbPacket, FILE *fp)
{
	int pos;
	char *p;

	p = (char*)memchr(pbPacket, 'G', 188);
	if (p==NULL) {
		for (int i = 0; i < 20; i++){
			if (fread(pbPacket, 188, 1, fp) != 1) {
				fprintf(stderr, "Unexpected EOF\n");
				Sleep(2000);
				return FALSE;
			}
			p = (char*)memchr(pbPacket, 'G', 188);
			if (p==NULL)
				continue;
			else
				break;
		}
	}
	if (p == NULL) {
		fprintf(stderr, "Missing syncbyte\n");
		Sleep(2000);
		return FALSE;
	}
	pos = p - (char*)pbPacket;
	_fseeki64(fp, -(188-pos), SEEK_CUR);
	return true;
}

long long GetPTS(PBYTE pbPacket)
{
	long long PTS = 0;
	// Get PTS in PES Header(00 00 01 BD)
	for (int i = 4; i < 188 - 10; i++) {
		if ( pbPacket[i] == 0x00 &&
			pbPacket[i+1] == 0x00 &&
			pbPacket[i+2] == 0x01 &&
			pbPacket[i+3] == 0xBD) {
				PBYTE pData = &pbPacket[i+9];

// mod
//				PTS = (DWORD)(((DWORD)(*pData) & 0xE) >> 1) << 30;
				PTS = (long long)(((DWORD)(*pData) & 0xE) >> 1) << 30;
// mod
				pData++;

				PTS += (DWORD)(*pData) << 22;
				pData++;

				PTS += (DWORD)((DWORD)(*pData) >> 1) << 15;
				pData++;

				PTS += (DWORD)(*pData) << 7;
				pData++;

				PTS += (DWORD)(*pData) >> 1;

				PTS = PTS/90;

				return PTS;
		}
	}
	return 0;
}

extern	USHORT PMTPid;
extern	USHORT CaptionPid;
extern	USHORT PCRPid;

void parse_PAT(BYTE *pbPacket) 
{
	PAT_HEADER *pat = (PAT_HEADER *)(pbPacket + sizeof(_Packet_Header)+1);

	for (int i = 0; i < (188-13)/4; i++) {
		WORD wProgramID =	swap16(pat->PMT_Array[i].program_id);
		WORD wPID =			swap16(pat->PMT_Array[i].PID) & 0x1FFF;
		if (wProgramID == 0xFFFF)
			break;
		_tMyPrintf(_T("Program %d, PID: %03x\r\n"), wProgramID, wPID);

		if (wProgramID != 0 && PMTPid == 0) {	//the first PMTPid found
			PMTPid = wPID;
		}
	}

	//bPrintPMT = FALSE;

	_tMyPrintf(_T("Set PMT_PID to %x\r\n"), PMTPid);
	_tMyPrintf(_T("Press any key to start\r\n"));
	//getch();
	Sleep(2000);
}

void parse_PMT(BYTE *pbPacket)
{
	PMT_HEADER *pmt = (PMT_HEADER *)(pbPacket + sizeof(_Packet_Header)+1);

// mark10als
//	if (PCRPid == 0) {
//		PCRPid = swap16(pmt->pcrpid) & 0x1FFF;
//	}
// mark10als

	INT length = swap16(pmt->program_info_length) & 0x0FFF;
	BYTE *pData = (BYTE *)&pmt->program_info_length + 2;
	pData += length;	//read thrugh program_info

	while (pData < pbPacket + 184) {
		PMT_PID_Desc *pmt_pid = (PMT_PID_Desc*)&pData[0];

		if (pmt_pid->StreamTypeID == 0x6) {
			CaptionPid = (swap16(pmt_pid->EsPID) & 0x1FFF);
// mark10als
		if (PCRPid == 0) {
			PCRPid = swap16(pmt->pcrpid) & 0x1FFF;
		}
// mark10als
			break;
		}
		pData += ((swap16(pmt_pid->DescLen)&0x0FFF) + sizeof(PMT_PID_Desc));
	}
}

void parse_Packet_Header(Packet_Header * packet_header, BYTE *pbPacket)
{
	_Packet_Header *packet = (_Packet_Header *)pbPacket;

	packet_header->Sync =				packet->Sync;
	packet_header->TsErr =				(swap16(packet->PID)>>15) & 0x01;
	packet_header->PayloadStartFlag =	(swap16(packet->PID)>>14) & 0x01;
	packet_header->Priority =			(swap16(packet->PID)>>13) & 0x01;
	packet_header->PID =				(swap16(packet->PID) & 0x1FFF);
	packet_header->Scramble =			(packet->Counter&0xC0)>>6;
	packet_header->AdaptFlag =			(packet->Counter&0x20)>>5;
	packet_header->PayloadFlag =		(packet->Counter&0x10)>>4;
	packet_header->Counter =			(packet->Counter&0x0F);
}