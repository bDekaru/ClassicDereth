#ifndef CHESS_MATCH_H
#define CHESS_MATCH_H

#include "ChessDefines.h"
#include "Frame.h"

#include <optional>
#include <functional>
#include <stack>
#include <array>
#include <utility>
#include <atomic>
#include <future>
#include <chrono>

class CPlayerWeenie;
class CWeenieObject;

namespace GDLE::Chess
{

inline ChessColour InverseColour(ChessColour const colour) { return colour ? White : Black; }

class ChessPieceCoord : public PackObj
{
public:
	ChessPieceCoord()
		: m_x(-1), m_y(-1) { }
	ChessPieceCoord(int32_t const x, int32_t const y)
		: m_x(x), m_y(y) { }
	explicit ChessPieceCoord(int32_t const offset)
		: m_x(offset % CHESS_BOARD_SIZE), m_y(offset / CHESS_BOARD_SIZE) { }

	DECLARE_PACKABLE()

	int32_t GetX() const { return m_x; }
	int32_t GetY() const { return m_y; }
	int32_t GetOffset() const { return (m_y * CHESS_BOARD_SIZE) + m_x; }
	ChessPieceRank GetRank() const { return static_cast<ChessPieceRank>(GetOffset() >> 3); }

	bool operator==(ChessPieceCoord const& r) const
	{
		return GetX() == r.GetX() && GetY() == r.GetY();
	}

	bool IsValid() const
	{
		if (m_x < 0 || m_x >= CHESS_BOARD_SIZE)
			return false;
		if (m_y < 0 || m_y >= CHESS_BOARD_SIZE)
			return false;
		return true;
	}

	void Move(int32_t const x, int32_t const y)
	{
		m_x = x;
		m_y = y;
	}

	void MoveOffset(int32_t const x, int32_t const y)
	{
		m_x += x;
		m_y += y;
	}

	void MoveOffset(Vector2 const& offset) { MoveOffset(offset.first, offset.second); }

private:
	int32_t m_x;
	int32_t m_y;
};

class GameMoveData : public PackObj
{
public:
	GameMoveData(ChessMoveType const type, ChessColour const colour, ChessPieceCoord from, ChessPieceCoord to)
		: m_type(type), m_colour(colour), m_from(std::move(from)), m_to(std::move(to)) { }

	DECLARE_PACKABLE()

	ChessMoveType GetMoveType() const { return m_type; }
	ChessColour GetColour() const { return m_colour; }
	ChessPieceCoord const& GetFrom() const { return m_from; }
	ChessPieceCoord const& GetTo() const { return m_to; }

private:
	ChessMoveType m_type;
	ChessColour m_colour;
	ChessPieceCoord m_from;
	ChessPieceCoord m_to;
};

class BasePiece
{
public:
	BasePiece(ChessPieceType const type, ChessColour const colour, ChessPieceCoord to)
		: m_type(type), m_colour(colour), m_coord(std::move(to)), m_guid() { }

	virtual ~BasePiece() = default;

	ChessPieceType GetType() const { return m_type; }
	ChessColour GetColour() const { return m_colour; }
	ChessPieceCoord const& GetCoord() const { return m_coord; }
	uint32_t GetGuid() const { return m_guid; }

	void SetType(ChessPieceType const type) { m_type = type; }
	void SetCoord(ChessPieceCoord const& to) { m_coord = to; }
	void SetGuid(uint32_t guid) { m_guid = guid; }

	bool CanAttackRaycast(ChessPieceCoord const& victim) const;

	virtual bool CanGoToRaycast(int8_t dx, int8_t dy) const = 0;
	virtual bool CanAttackRaycast(int8_t const dx, int8_t const dy) const { return CanGoToRaycast(dx, dy); }

protected:
	ChessPieceType m_type;
	ChessColour m_colour;
	ChessPieceCoord m_coord;
	
	uint32_t m_guid;
};

class PawnPiece : public BasePiece
{
public:
	PawnPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(Pawn, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
	bool CanAttackRaycast(int8_t dx, int8_t dy) const override;
};

class RookPiece : public BasePiece
{
public:
	RookPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(Rook, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
};

class KnightPiece : public BasePiece
{
public:
	KnightPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(Knight, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
};

class BishopPiece : public BasePiece
{
public:
	BishopPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(Bishop, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
};

class QueenPiece : public BasePiece
{
public:
	QueenPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(Queen, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
};

class KingPiece : public BasePiece
{
public:
	KingPiece(ChessColour const colour, ChessPieceCoord const& to)
		: BasePiece(King, colour, to) { }

	bool CanGoToRaycast(int8_t dx, int8_t dy) const override;
};

// holds all of the information that can change during a half turn
class ChessMove
{
public:
	ChessMove(ChessMoveFlag const flags, ChessColour const colour, ChessPieceType const type,
		ChessPieceCoord from, ChessPieceCoord to, ChessPieceType const promotion, ChessPieceType const captured,
		uint32_t const move, uint32_t const halfMove, std::array<uint32_t, CHESS_COLOUR_COUNT> const castling,
		std::optional<ChessPieceCoord> enPassantCoord, uint32_t const guid, uint32_t const capturedGuid)
		: flags(flags), m_colour(colour), m_type(type), m_from(std::move(from)), m_to(std::move(to)),
			m_promotion(promotion),m_captured(captured), m_move(move), m_halfMove(halfMove), m_castling(castling),
			m_enPassantCoord(std::move(enPassantCoord)), m_guid(guid), m_capturedGuid(capturedGuid) { }

	ChessMoveFlag GetFlags() const { return flags; }
	ChessColour GetColour() const { return m_colour; }
	ChessPieceType GetType() const { return m_type; }
	ChessPieceCoord GetFromCoord() const { return m_from; }
	ChessPieceCoord GetToCoord() const { return m_to; }
	ChessPieceType GetPromotion() const { return m_promotion; }
	ChessPieceType GetCapture() const { return m_captured; }
	uint32_t GetMove() const { return m_move; }
	uint32_t GetHalfMove() const { return m_halfMove; }
	std::array<uint32_t, CHESS_COLOUR_COUNT> GetCastling() const { return m_castling; }
	std::optional<ChessPieceCoord> GetEnPassantCoord() const { return m_enPassantCoord; }
	uint32_t GetGuid() const { return m_guid; }
	uint32_t GetCapturedGuid() const { return m_capturedGuid; }

private:
	ChessMoveFlag flags;
	ChessColour m_colour;
	ChessPieceType m_type;
	ChessPieceCoord m_from;
	ChessPieceCoord m_to;
	ChessPieceType m_promotion;
	ChessPieceType m_captured;
	uint32_t m_move;
	uint32_t m_halfMove;
	std::array<uint32_t, CHESS_COLOUR_COUNT> m_castling { };
	std::optional<ChessPieceCoord> m_enPassantCoord;
	uint32_t m_guid;
	uint32_t m_capturedGuid;
};

typedef std::vector<ChessMove> ChessMoveStore;

class ChessDelayedAction
{
public:
	ChessDelayedAction(ChessDelayedActionType const action)
		: m_action(action), m_colour(), m_stalemate() { }
	ChessDelayedAction(ChessDelayedActionType const action, ChessColour const colour)
		: m_action(action), m_colour(colour), m_stalemate() { }
	ChessDelayedAction(ChessDelayedActionType const action, ChessColour const colour, bool stalemate)
		: m_action(action), m_colour(colour), m_stalemate(stalemate) { }
	ChessDelayedAction(ChessDelayedActionType const action, ChessColour const colour, ChessPieceCoord from, ChessPieceCoord to)
		: m_action(action), m_colour(colour), m_from(std::move(from)), m_to(std::move(to)), m_stalemate() { }

	ChessDelayedActionType GetAction() const { return m_action; }
	ChessColour GetColour() const { return m_colour; }
	ChessPieceCoord const& GetFromCoord() const { return m_from; }
	ChessPieceCoord const& GetToCoord() const { return m_to; }
	bool GetStalemate() const { return m_stalemate; }

private:
	ChessDelayedActionType m_action;
	ChessColour m_colour;
	ChessPieceCoord m_from;
	ChessPieceCoord m_to;
	bool m_stalemate;
};

class ChessAiMoveResult
{
public:
	ChessAiMoveResult()
		: m_result(), m_profilingTime(), m_profilingCounter() { }

	ChessMoveResult GetResult() const { return m_result; }
	ChessPieceCoord const& GetFromCoord() const { return m_from; }
	ChessPieceCoord const& GetToCoord() const { return m_to; }
	uint32_t GetProfilingTime() const { return m_profilingTime; }
	uint32_t GetProfilingCounter() const { return m_profilingCounter; }

	void SetResult(ChessMoveResult const result, ChessPieceCoord const& from, ChessPieceCoord const& to)
	{
		m_result = result;
		m_from   = from;
		m_to     = to;
	}

	void SetProfilingTime(uint32_t const ms) { m_profilingTime = ms; }
	void SetProfilingCounter(uint32_t const counter) { m_profilingCounter = counter; }

private:
	ChessMoveResult m_result;
	ChessPieceCoord m_from;
	ChessPieceCoord m_to;

	uint32_t m_profilingTime;    // time taken in milliseconds to calculate ai move
	uint32_t m_profilingCounter; // minimax recursion count
};

class ChessAiMove
{
public:
	ChessAiMoveResult operator()(class ChessMatch* match) const;
};

class ChessAiAsyncTurnKey
{
	friend class ChessAiMove;
	friend class ChessMatch;

	// no constructor for you!
	ChessAiAsyncTurnKey() = default;
	ChessAiAsyncTurnKey(ChessAiAsyncTurnKey&) = default;
	ChessAiAsyncTurnKey& operator=(ChessAiAsyncTurnKey&) = default;
	ChessAiAsyncTurnKey(ChessAiAsyncTurnKey&&) = default;
	ChessAiAsyncTurnKey& operator=(ChessAiAsyncTurnKey&&) = default;
	ChessAiAsyncTurnKey(std::initializer_list<ChessAiAsyncTurnKey>&) { }
	ChessAiAsyncTurnKey(std::initializer_list<ChessAiAsyncTurnKey>&&) { }
};

class ChessLogic
{
public:
	ChessLogic();
	~ChessLogic();

	// prevent copying
	ChessLogic(ChessLogic&) = delete;
	ChessLogic& operator=(ChessLogic&) = delete;
	ChessLogic(ChessLogic&&) = delete;
	ChessLogic& operator=(ChessLogic&&) = delete;

	ChessColour GetTurn() const { return m_turn; }
	BasePiece* GetPiece(ChessPieceCoord const& from) const
	{
		if (!from.IsValid())
			return nullptr;
		return m_board[(from.GetY() * CHESS_BOARD_SIZE) + from.GetX()];
	}
	BasePiece* GetPiece(uint32_t pieceGuid) const;
	ChessMove const& GetLastMove() const { return m_history.top(); }

	void WalkPieces(std::function<void(BasePiece*)> const& func) const;

	ChessMoveResult Move(ChessColour colour, ChessPieceCoord const& from, ChessPieceCoord const& to);

	// turn key
	ChessMoveResult AsyncCalculateAiSimpleMove(ChessAiAsyncTurnKey, ChessPieceCoord& from, ChessPieceCoord& to);
	ChessMoveResult AsyncCalculateAiComplexMove(ChessAiAsyncTurnKey, ChessPieceCoord& from, ChessPieceCoord& to, uint32_t& counter);

private:
	BasePiece* AddPiece(ChessColour const colour, ChessPieceType const type, ChessPieceCoord const& to) { return AddPiece(colour, type, to.GetX(), to.GetY()); }
	BasePiece* AddPiece(ChessColour colour, ChessPieceType type, uint8_t x, uint8_t y);
	BasePiece* GetPiece(ChessColour colour, ChessPieceType type) const;
	void RemovePiece(ChessPieceCoord const& victim);
	void RemovePiece(BasePiece* piece);
	void MovePiece(ChessPieceCoord const& from, ChessPieceCoord const& to);

	float MinimaxAlphaBeta(uint8_t depth, float alpha, float beta, bool isMaximisingPlayer, uint32_t& counter);
	float EvaluateBoard() const;

	void GenerateMoves(BasePiece* piece, bool single, ChessMoveStore& storage) const;
	void GenerateMoves(ChessColour colour, ChessMoveStore& storage) const;
	bool CanAttack(ChessColour attacker, ChessPieceCoord const& victim) const;
	bool InCheck() const;
	bool InCheckmate() const;

	void BuildMove(ChessMoveStore& storage, uint32_t result, ChessColour colour, ChessPieceType type,
		ChessPieceCoord const& from, ChessPieceCoord const& to) const;
	ChessMoveResult FinaliseMove(ChessMove const& move);
	void InternalMove(ChessMove const& move);
	void UndoMove(uint32_t count);

	ChessColour m_turn;
	uint32_t m_move;
	uint32_t m_halfMove;
	std::array<uint32_t, CHESS_COLOUR_COUNT> m_castling { ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle, ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle };
	std::optional<ChessPieceCoord> m_enPassantCoord;
	std::array<BasePiece*, CHESS_BOARD_SIZE * CHESS_BOARD_SIZE> m_board { };
	std::stack<ChessMove> m_history;
};

class ChessSide
{
public:
	ChessSide(uint32_t const guid, ChessColour const colour)
		: m_guid(guid), m_colour(colour), m_stalemate() { }

	uint32_t GetGuid() const { return m_guid; }
	ChessColour GetColour() const { return m_colour; }
	bool GetStalemate() const { return m_stalemate; }
	bool IsAi() const { return !m_guid; }
	CPlayerWeenie* GetPlayer() const;

	void SetStalemate(bool const on) { m_stalemate = on; }

private:
	uint32_t m_guid;
	ChessColour m_colour;
	bool m_stalemate;
};

typedef std::queue<ChessDelayedAction> DelayedActionQueue;
typedef std::set<uint32_t> WeenieMotionStore;

class ChessMatch
{
public:
	explicit ChessMatch(CWeenieObject* game);
	~ChessMatch();

	// prevent copying
	ChessMatch(ChessMatch&) = delete;
	ChessMatch& operator=(ChessMatch&) = delete;
	ChessMatch(ChessMatch&&) = delete;
	ChessMatch& operator=(ChessMatch&&) = delete;

	uint32_t GetGuid() const { return m_guid; }
	ChessState GetState() const { return m_state; }
	ChessAiState GetAiState() const { return m_aiState; }

	bool IsInMatch(uint32_t guid) const;
	std::optional<ChessColour> GetColour(uint32_t guid) const;

	void Update();

	void AddSide(uint32_t guid, ChessColour colour);
	void AddAi();

	void Join(CPlayerWeenie* player);
	void MoveEnqueue(CPlayerWeenie* player, ChessPieceCoord const& from, ChessPieceCoord const& to);
	void MovePassEnqueue(CPlayerWeenie* player);
	void QuitEnqueue(CPlayerWeenie* player);
	void StalemateEnqueue(CPlayerWeenie* player, bool on);

	// turn key
	void AsyncMoveAiSimple(ChessAiAsyncTurnKey, ChessAiMoveResult& result);
	void AsyncMoveAiComplex(ChessAiAsyncTurnKey, ChessAiMoveResult& result);

	void PieceReady(uint32_t pieceGuid);

	static void SendJoinGameResponse(CPlayerWeenie* player, uint32_t guid, std::optional<ChessColour> colour);

private:
	std::optional<ChessColour> GetFreeColour() const;

	void Start();
	void Finish(int32_t winner);
	void FinishTurn();

	void StartAiMove();
	void FinishAIMove();
	void FinaliseWeenieMove(ChessMoveResult result);

	void MoveDelayed(ChessDelayedAction const& action);
	void MovePassDelayed(ChessDelayedAction const& action);
	void QuitDelayed(ChessColour colour);
	void StalemateDelayed(ChessDelayedAction const& action);

	void CalculateWeeniePosition(ChessPieceCoord const& coord, ChessColour colour, Position& position) const;
	void AddWeeniePiece(BasePiece* piece) const;
	void MoveWeeniePiece(BasePiece* piece);
	void AttackWeeniePiece(BasePiece* piece, uint32_t victim);
	void RemoveWeeniePiece(BasePiece* piece) const;
	void UpgradeWeeniePiece(BasePiece* piece) const;
	void AddPendingWeenieMotion(BasePiece* piece);

	void SendStartGame(CPlayerWeenie* player, ChessColour colour) const;
	void SendMoveResponse(CPlayerWeenie* player, ChessMoveResult result) const;
	void SendOpponentTurn(CPlayerWeenie* player, ChessColour colour, GameMoveData& move) const;
	void SendOpponentStalemateState(CPlayerWeenie* player, ChessColour colour, bool on) const;
	void SendGameOver(CPlayerWeenie* player, int32_t winner) const;

	uint32_t m_guid;
	Position m_position;
	ChessState m_state;
	std::array<ChessSide*, CHESS_COLOUR_COUNT> m_side { };
	ChessLogic m_logic;
	DelayedActionQueue m_actions;

	std::atomic<ChessAiState> m_aiState;
	std::future<ChessAiMoveResult> m_aiFuture;

	ChessMoveResult m_moveResult;
	bool m_waitingForWeenieMotion;
	WeenieMotionStore m_weenieMotion;

	std::optional<std::chrono::time_point<std::chrono::steady_clock>> m_nextRangeCheck;
};

} // GDLE::Chess

#endif // CHESS_MATCH_H
