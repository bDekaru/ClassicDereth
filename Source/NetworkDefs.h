
#pragma once

struct BlobHeader_s //Size: 0x14
{
	uint32_t dwSequence; //0x00
	uint32_t dwFlags; //0x04
	uint32_t dwCRC; //0x08
	WORD wRecID; //0x0C
	WORD wTime; //0x0E
	WORD wSize; //0x10
	WORD wTable; //0x12
};

struct BlobPacket_s
{
	BlobHeader_s header;
	BYTE data[];
};

#define DUPEBLOB(name, source) \
 BlobPacket_s *name = (BlobPacket_s *)new BYTE[ BLOBLEN(source) ]; \
 memcpy(name, source, BLOBLEN(source));
#define CREATEBLOB(name, size) \
 BlobPacket_s *name = (BlobPacket_s *)new BYTE[ sizeof(BlobHeader_s) + (size) ]; \
 name->header.wSize = (WORD)(size);
#define DELETEBLOB(name) delete [] ((BYTE *)name)
#define BLOBLEN(x) (sizeof(BlobHeader_s) + (x)->header.wSize)

struct FragHeader_s
{
	//uint32_t dwSequence;
	//uint32_t dwID;
	uint64_t id;
	WORD wCount;
	WORD wSize;
	WORD wIndex;
	WORD wGroup;
};

struct FragPacket_s
{
	FragHeader_s header;
	BYTE data[];
};

//Blob types.
#define BT_NULL 0x00000000 //Used on account login packets.
#define BT_RESENT 0x00000001 //
#define BT_REQUESTLOST 0x00001000 // What value is this now?
// #define BT_FLUSH 0x00000004 // What value is this now?
#define BT_DENY 0x00002000 //
#define BT_ACKSEQUENCE 0x00004000 //
#define BT_DISCONNECT 0x00008000 //
#define BT_CONNECTIONACK 0x00080000 //
// #define BT_CRCUPDATE 0x00000100 Not sure what this is now //
#define BT_FRAGMENTS 0x00000004 // 0x00000200 //
#define BT_RESETTIME 0x00000400 //
//#define BT_STRANSFER 0x00000800 0x100? //Server transfers. Used on login servers, primarily.
#define BT_LOGIN 0x00010000 //
//#define BT_WTRANSFER 0x00020000 0x100? //Server transfers. Used on world servers, primarily.
#define BT_LOGINREPLY 0x00040000 //
// #define BT_WAKE 0x00080000 //
#define BT_TIMEUPDATE 0x01000000 //
#define BT_ECHOREQUEST 0x02000000 //hi u
#define BT_ECHORESPONSE 0x04000000 //hi u
#define BT_FLOW 0x08000000 //hi u
#define BT_ERROR 0x00800000 //

#define BT_NETERROR 0x00100000

#define BT_EPHEMERAL 0x80000000

#define BT_USES_CRC 0x2

#define OBJECT_MSG 10 // old value was 0x03?
#define PRIVATE_MSG 9 // old value was 0x04?
#define EVENT_MSG 5 // old value was 0x07?

#define MAX_BLOB_LEN 0x1E4
#define MAX_FRAGMENT_LEN 0x1C0
#define MAX_MESSAGE_LEN (0x1C0 * 0x10)

#define ST_SERVER_ERRORS 0x00000008

#define STR_LOGIN_FAILED 0x04DF9C54
#define STR_LOGIN_SERVER_FULL 0x00F9982C
#define STR_LOGIN_CLIENT_VERSION 0x00A7E948
#define STR_LOGIN_ACCOUNT_ONLINE 0x0C559B1E
#define STR_LOGIN_INVALID_AUTH 0x082E3779
