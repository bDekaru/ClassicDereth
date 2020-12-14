#ifndef CHESS_DEFINES_H
#define CHESS_DEFINES_H

namespace GDLE::Chess
{

#define CHESS_BOARD_SIZE 8

enum ChessColour
{
	White,
	Black
};

#define CHESS_COLOUR_COUNT    2
#define CHESS_COLOUR_INVALID -1

#define CHESS_WINNER_STALEMATE -1
#define CHESS_WINNER_END_GAME  -2 // finishes match with no winners

enum ChessPieceType
{
	Empty,
	Pawn,
	Rook,
	Knight,
	Bishop,
	Queen,
	King,
	Count
};

enum ChessPieceRank
{
	Rank8,
	Rank7,
	Rank6,
	Rank5,
	Rank4,
	Rank3,
	Rank2,
	Rank1
};

enum ChessState
{
	ChessStateWaitingForPlayers,
	ChessStateInProgress,
	ChessStateFinished
};

enum ChessMoveType
{
	MoveTypeInvalid,
	MoveTypePass,
	MoveTypeResign,
	MoveTypeStalemate,
	MoveTypeGrid,
	MoveTypeFromTo,
	MoveTypeSelectedPiece
};

enum ChessMoveResult
{
	NoMoveResult                    = 0x0000,
	OKMoveToEmptySquare             = 0x0001,
	OKMoveToOccupiedSquare          = 0x0002,
	OKMoveEnPassant                 = OKMoveToEmptySquare | OKMoveToOccupiedSquare,
	OKMoveMask                      = 0x03FF,
	OKMoveCheck                     = 0x0400,
	OKMoveCheckmate                 = 0x0800,
	OKMovePromotion                 = 0x1000,
	OKMoveToEmptySquareCheck        = OKMoveCheck | OKMoveToEmptySquare,
	OKMoveToOccupiedSquareCheck     = OKMoveCheck | OKMoveToOccupiedSquare,
	OKMoveEnPassantCheck            = OKMoveCheck | OKMoveEnPassant,
	OKMovePromotionCheck            = OKMovePromotion | OKMoveCheck,
	OKMoveToEmptySquareCheckmate    = OKMoveCheckmate | OKMoveToEmptySquare,
	OKMoveToOccupiedSquareCheckmate = OKMoveCheckmate | OKMoveToOccupiedSquare,
	OKMoveEnPassantCheckmate        = OKMoveCheckmate | OKMoveEnPassant,
	OKMovePromotionCheckmate        = OKMovePromotion | OKMoveCheckmate,

	BadMoveInvalidCommand           = -1,
	BadMoveNotPlaying               = -2,
	BadMoveNotYourTurn              = -3,
	BadMoveDirection                = -100,
	BadMoveDistance                 = -101,
	BadMoveNoPiece                  = -102,
	BadMoveNotYours                 = -103,
	BadMoveDestination              = -104,
	BadMoveWouldClobber             = -105,
	BadMoveSelfCheck                = -106,
	BadMoveWouldCollide             = -107,
	BadMoveCantCastleOutOfCheck     = -108,
	BadMoveCantCastleThroughCheck   = -109,
	BadMoveCantCastleAfterMoving    = -110,
	BadMoveInvalidBoardState        = -111
};

enum ChessMoveFlag
{
	ChessMoveFlagNormal           = 0x01,
	ChessMoveFlagCapture          = 0x02,
	ChessMoveFlagBigPawn          = 0x04,
	ChessMoveFlagEnPassantCapture = 0x08,
	ChessMoveFlagPromotion        = 0x10,
	ChessMoveFlagKingSideCastle   = 0x20,
	ChessMoveFlagQueenSideCastle  = 0x40
};

enum ChessAiState
{
	ChessAiStateNone,
	ChessAiStateWaitingToStart,   // work has been requested, future object will be initialised and made valid next update
	ChessAiStateWaitingForWorker, // future object is valid but has yet to start a worker thread
	ChessAiStateInProgress,       // worker is currently calculating ai move
	ChessAiStateWaitingForFinish  // worker has finished calculating ai move, future get will not block or block for a short time
};

enum ChessDelayedActionType
{
	DelayedActionTypeStart,
	DelayedActionTypeMove,
	DelayedActionTypeMovePass,
	DelayedActionTypeStalemate,
	DelayedActionTypeQuit
};

static const int32_t PieceWorth[Count] =
{
	0, 10, 50, 30, 30, 90, 900
};

typedef std::pair<int32_t /*x*/, int32_t /*y*/> Vector2;

class RookCastleFlag
{
public:
	RookCastleFlag(Vector2 const vector, ChessMoveFlag const flag)
		: m_vector(vector), m_flag(flag) { }

	Vector2 const& GetVector() const { return m_vector; }
	ChessMoveFlag GetFlag() const { return m_flag; }

private:
	Vector2 m_vector;
	ChessMoveFlag m_flag;
};

typedef std::multimap<ChessColour, RookCastleFlag> RookFlagStore;
static const RookFlagStore RookFlags = 
{
	{ White, { { 0, 0 }, ChessMoveFlagQueenSideCastle } },
	{ White, { { 7, 0 }, ChessMoveFlagKingSideCastle  } },
	{ Black, { { 0, 7 }, ChessMoveFlagQueenSideCastle } },
	{ Black, { { 7, 7 }, ChessMoveFlagKingSideCastle  } }
};

// GplpwdLphWtgt
static const Vector2 PawnOffsets[CHESS_COLOUR_COUNT][4] =
{
	// white
	{ { 0,  1 }, { 0,  2 }, {  1,  1 }, { -1,  1 } },
	// black
	{ { 0, -1 }, { 0, -2 }, { -1, -1 }, {  1, -1 } }
};

// determines valid moves for a particular piece
static const std::multimap<ChessPieceType, Vector2> PieceOffsets =
{
	{ Rook,   {  0,  1 } }, { Rook,   {  1,  0 } }, { Rook,   {  0, -1 } }, { Rook,  { -1,  0 } },
	{ Knight, {  1,  2 } }, { Knight, {  2,  1 } }, { Knight, {  2, -1 } }, { Knight,{  1, -2 } },
	{ Knight, { -1, -2 } }, { Knight, { -2, -1 } }, { Knight, { -2,  1 } }, { Knight,{ -1,  2 } },
	{ Bishop, {  1,  1 } }, { Bishop, {  1, -1 } }, { Bishop, { -1, -1 } }, { Bishop,{ -1,  1 } },
	{ Queen,  {  0,  1 } }, { Queen,  {  1,  1 } }, { Queen,  {  1,  0 } }, { Queen, {  1, -1 } },
	{ Queen,  {  0, -1 } }, { Queen,  { -1, -1 } }, { Queen,  { -1,  0 } }, { Queen, { -1,  1 } },
	{ King,   {  0,  1 } }, { King,   {  1,  1 } }, { King,   {  1,  0 } }, { King,  {  1, -1 } },
	{ King,   {  0, -1 } }, { King,   { -1, -1 } }, { King,   { -1,  0 } }, { King,  { -1,  1 } }
};

// piece-square tables from Chess programming wiki
static const float PieceSquareTable[Count][CHESS_COLOUR_COUNT][CHESS_BOARD_SIZE * CHESS_BOARD_SIZE] =
{
	// empty
	{
		{ },
		{ }
	},
	// pawn
	{
		// white
		{
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,
			 1.0f,  1.0f,  2.0f,  3.0f,  3.0f,  2.0f,  1.0f,  1.0f,
			 0.5f,  0.5f,  1.0f,  2.5f,  2.5f,  1.0f,  0.5f,  0.5f,
			 0.0f,  0.0f,  0.0f,  2.0f,  2.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -1.0f,  0.0f,  0.0f, -1.0f, -0.5f,  0.5f,
			 0.5f,  1.0f,  1.0f, -2.0f, -2.0f,  1.0f,  1.0f,  0.5f,
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f
		},
		// black
		{
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  1.0f,  1.0f, -2.0f, -2.0f,  1.0f,  1.0f,  0.5f,
			 0.5f, -0.5f, -1.0f,  0.0f,  0.0f, -1.0f, -0.5f,  0.5f,
			 0.0f,  0.0f,  0.0f,  2.0f,  2.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  1.0f,  2.5f,  2.5f,  1.0f,  0.5f,  0.5f,
			 1.0f,  1.0f,  2.0f,  3.0f,  3.0f,  2.0f,  1.0f,  1.0f,
			 5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f
		}
	},
	// rook
	{
		// white
		{
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			 0.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f
		},
		// black
		{
			 0.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			-0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -0.5f,
			 0.5f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.5f,
			 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f
		}
	},
	// knight
	{
		// white
		{
			-5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f,
			-4.0f, -2.0f,  0.0f,  0.0f,  0.0f,  0.0f, -2.0f, -4.0f,
			-3.0f,  0.0f,  1.0f,  1.5f,  1.5f,  1.0f,  0.0f, -3.0f,
			-3.0f,  0.5f,  1.5f,  2.0f,  2.0f,  1.5f,  0.5f, -3.0f,
			-3.0f,  0.0f,  1.5f,  2.0f,  2.0f,  1.5f,  0.0f, -3.0f,
			-3.0f,  0.5f,  1.0f,  1.5f,  1.5f,  1.0f,  0.5f, -3.0f,
			-4.0f, -2.0f,  0.0f,  0.5f,  0.5f,  0.0f, -2.0f, -4.0f,
			-5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f
		},
		// black
		{ }
	},
	// bishop
	{
		// white
		{
			-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f,
			-1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
			-1.0f,  0.0f,  0.5f,  1.0f,  1.0f,  0.5f,  0.0f, -1.0f,
			-1.0f,  0.5f,  0.5f,  1.0f,  1.0f,  0.5f,  0.5f, -1.0f,
			-1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
			-1.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.5f, -1.0f,
			-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f
		},
		// black
		{
			-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f,
			-1.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.5f, -1.0f,
			-1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
			-1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f,
			-1.0f,  0.5f,  0.5f,  1.0f,  1.0f,  0.5f,  0.5f, -1.0f,
			-1.0f,  0.0f,  0.5f,  1.0f,  1.0f,  0.5f,  0.0f, -1.0f,
			-1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
			-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f
		}
	},
	// queen
	{
		// white
		{
			-2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f,
			-1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
			-1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f,
			-0.5f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f,
			 0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f,
			-1.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f,
			-1.0f,  0.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,
			-2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f
		},
		// black
		{ }
	},
	// king
		{
		// white
		{
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-2.0f, -3.0f, -3.0f, -4.0f, -4.0f, -3.0f, -3.0f, -2.0f,
			-1.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -1.0f,
			 2.0f,  2.0f,  0.0f,  0.0f,  0.0f,  0.0f,  2.0f,  2.0f,
			 2.0f,  3.0f,  1.0f,  0.0f,  0.0f,  1.0f,  3.0f,  2.0f
		},
		// black
		{
			 2.0f,  3.0f,  1.0f,  0.0f,  0.0f,  1.0f,  3.0f,  2.0f,
			 2.0f,  2.0f,  0.0f,  0.0f,  0.0f,  0.0f,  2.0f,  2.0f,
			-1.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -1.0f,
			-2.0f, -3.0f, -3.0f, -4.0f, -4.0f, -3.0f, -3.0f, -2.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f,
			-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f
		}
	}
};

#define DEBUG_SIMPLE_AI

}

#endif // CHESS_DEFINES_H
