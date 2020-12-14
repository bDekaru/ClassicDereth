#include <StdAfx.h>
#include "ChessMatch.h"
#include "Player.h"
#include "World.h"
#include "WeenieFactory.h"
#include "MathLib.h"
#include "RandomRange.h"
#include "GamePiece.h"

#include <algorithm>
#include <utility>
#include <iterator>

namespace GDLE::Chess
{

DEFINE_PACK(ChessPieceCoord)
{
	pWriter->Write<int32_t>(m_x);
	pWriter->Write<int32_t>(m_y);
}

DEFINE_UNPACK(ChessPieceCoord)
{
	m_x = pReader->Read<int32_t>();
	m_y = pReader->Read<int32_t>();
	return true;
}

DEFINE_PACK(GameMoveData)
{
	pWriter->Write<ChessMoveType>(m_type);
	pWriter->Write<ChessColour>(m_colour);

	// there are other types to handle but client just ignores them
	switch (m_type)
	{
		case MoveTypeFromTo:
		{
			m_from.Pack(pWriter);
			m_to.Pack(pWriter);
			break;
		}
		default:
			break;
	}
}

DEFINE_UNPACK(GameMoveData)
{
	return true;
}

bool BasePiece::CanAttackRaycast(ChessPieceCoord const& victim) const
{
	int8_t const dx = m_coord.GetX() - victim.GetX();
	int8_t const dy = m_coord.GetY() - victim.GetY();
	return CanAttackRaycast(dx, dy);
}

bool PawnPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	bool const hasMoved = (m_colour ? Rank2 : Rank7) != dy;
	uint8_t const ady = abs(dy);
	return !dx && (ady == 1 || ady == 2 && !hasMoved);
}

bool PawnPiece::CanAttackRaycast(int8_t const dx, int8_t const dy) const
{
	return abs(dx) == 1 && dy == 1;
}

bool RookPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	return (dy != 0) ^ (dx != 0);
}

bool KnightPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	uint8_t const adx = abs(dx);
	uint8_t const ady = abs(dy);
	return adx == 1 && ady == 2 || adx == 2 && ady == 1;
}

bool BishopPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	return abs(dy) == abs(dx);
}

bool QueenPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	return !dx || !dy || abs(dx) == abs(dy);
}

bool KingPiece::CanGoToRaycast(int8_t const dx, int8_t const dy) const
{
	return abs(dx) < 2 && abs(dy) < 2;
}

// executed in a seperate worker thread
ChessAiMoveResult ChessAiMove::operator()(ChessMatch* match) const
{
	//using namespace std::chrono;
	time_point const start = steady_clock::now();

	ChessAiMoveResult result;

	#ifdef DEBUG_SIMPLE_AI
	match->AsyncMoveAiSimple({}, result);
	#else
	match->AsyncMoveAiComplex({}, result);
	#endif

	time_point const end = steady_clock::now();
	result.SetProfilingTime(std::chrono::duration_cast<milliseconds>(end - start).count());

	return result;
}

ChessLogic::ChessLogic()
	: m_turn(White), m_move(), m_halfMove()
{
	// setup white
	AddPiece(White, Rook, 0, 0);
	AddPiece(White, Knight, 1, 0);
	AddPiece(White, Bishop, 2, 0);
	AddPiece(White, Queen, 3, 0);
	AddPiece(White, King, 4, 0);
	AddPiece(White, Bishop, 5, 0);
	AddPiece(White, Knight, 6, 0);
	AddPiece(White, Rook, 7, 0);

	for (uint8_t i = 0; i < CHESS_BOARD_SIZE; i++)
		AddPiece(White, Pawn, i, 1);

	// setup black
	AddPiece(Black, Rook, 0, 7);
	AddPiece(Black, Knight, 1, 7);
	AddPiece(Black, Bishop, 2, 7);
	AddPiece(Black, King, 4, 7);
	AddPiece(Black, Queen, 3, 7);
	AddPiece(Black, Bishop, 5, 7);
	AddPiece(Black, Knight, 6, 7);
	AddPiece(Black, Rook, 7, 7);

	for (uint8_t i = 0; i < CHESS_BOARD_SIZE; i++)
		AddPiece(Black, Pawn, i, 6);
}

ChessLogic::~ChessLogic()
{
	WalkPieces([](BasePiece* piece)
	{
		delete piece;
	});
}

BasePiece* ChessLogic::AddPiece(ChessColour const colour, ChessPieceType const type, uint8_t const x, uint8_t const y)
{
	ChessPieceCoord to(x, y);
	assert(to.IsValid());

	BasePiece* piece = nullptr;
	switch (type)
	{
		case Pawn:
			piece = new PawnPiece(colour, to);
			break;
		case Rook:
			piece = new RookPiece(colour, to);
			break;
		case Knight:
			piece = new KnightPiece(colour, to);
			break;
		case Bishop:
			piece = new BishopPiece(colour, to);
			break;
		case Queen:
			piece = new QueenPiece(colour, to);
			break;
		case King:
			piece = new KingPiece(colour, to);
			break;
		default:
			assert(false);
	}

	m_board[(y * CHESS_BOARD_SIZE) + x] = piece;
	return piece;
}

BasePiece* ChessLogic::GetPiece(ChessColour colour, ChessPieceType type) const
{
	auto const itr = std::find_if(std::begin(m_board), std::end(m_board),
		[colour, type](BasePiece* piece) { return piece && piece->GetColour() == colour && piece->GetType() == type; });
	return itr != std::end(m_board) ? *itr : nullptr;
}

BasePiece* ChessLogic::GetPiece(uint32_t const pieceGuid) const
{
	auto const itr = std::find_if(std::begin(m_board), std::end(m_board),
		[pieceGuid](BasePiece* piece) { return piece && piece->GetGuid() == pieceGuid; });
	return itr != std::end(m_board) ? *itr : nullptr;
}

void ChessLogic::RemovePiece(ChessPieceCoord const& victim)
{
	if (BasePiece* piece = GetPiece(victim))
		RemovePiece(piece);
}

void ChessLogic::RemovePiece(BasePiece* piece)
{
	assert(piece);

	ChessPieceCoord const coord = piece->GetCoord();
	delete piece;
	m_board[coord.GetOffset()] = nullptr;
}

void ChessLogic::MovePiece(ChessPieceCoord const& from, ChessPieceCoord const& to)
{
	BasePiece* fromPiece = GetPiece(from);
	fromPiece->SetCoord(to);

	RemovePiece(to);

	m_board[to.GetOffset()] = m_board[from.GetOffset()];
	m_board[from.GetOffset()] = nullptr;
}

void ChessLogic::WalkPieces(std::function<void(BasePiece*)> const& func) const
{
	for (uint8_t y = 0; y < CHESS_BOARD_SIZE; y++)
		for (uint8_t x = 0; x < CHESS_BOARD_SIZE; x++)
			if (BasePiece* piece = m_board[(y * CHESS_BOARD_SIZE) + x])
				func(piece);
}

ChessMoveResult ChessLogic::Move(ChessColour const colour, ChessPieceCoord const& from, ChessPieceCoord const& to)
{
	if (!from.IsValid())
		return BadMoveDestination;
	if (!to.IsValid())
		return BadMoveDestination;

	if (m_turn != colour)
		return BadMoveNotYourTurn;

	BasePiece* fromPiece = GetPiece(from);
	if (!fromPiece)
		return BadMoveNoPiece;

	if (fromPiece->GetColour() != colour)
		return BadMoveNotYours;

	ChessMoveStore storage;
	GenerateMoves(fromPiece, true, storage);
	
	auto const itr = std::find_if(std::begin(storage), std::end(storage),
		[from, to](ChessMove& move) { return move.GetFromCoord() == from && move.GetToCoord() == to; });

	// if this fails the client and server failed to find a common valid move
	if (itr == std::end(storage))
		return BadMoveDestination;

	return FinaliseMove(*itr);
}

ChessMoveResult ChessLogic::AsyncCalculateAiSimpleMove(ChessAiAsyncTurnKey, ChessPieceCoord& from, ChessPieceCoord& to)
{
	ChessColour const colour = m_turn;

	float bestBoardScore = 0;
	std::optional<ChessMove> bestMove;

	ChessMoveStore storage;
	GenerateMoves(m_turn, storage);

	for (ChessMove const& generatedMove : storage)
	{
		// no need to evaluate the board if the ai has checkmated the other player
		ChessMoveResult const result = FinaliseMove(generatedMove);
		if (result & OKMoveCheckmate)
		{
			from = generatedMove.GetFromCoord();
			to   = generatedMove.GetToCoord();
			return result;
		}

		float const boardScore = EvaluateBoard();
		if (boardScore > bestBoardScore)
		{
			bestMove.emplace(generatedMove);
			bestBoardScore = boardScore;
		}

		UndoMove(1);
	}

	// every generated move had the same board score, pick one at random
	// this shouldn't happen, just here to prevent crash
	if (!bestMove.has_value() && !storage.empty())
	{
		ChessMoveStore::iterator itr = std::begin(storage);
		std::advance(itr, getRandomNumber(storage.size() - 1));
		bestMove.emplace(*itr);
	}

	assert(bestMove.has_value());
	from = bestMove->GetFromCoord();
	to   = bestMove->GetToCoord();
	return FinaliseMove(*bestMove);
}

ChessMoveResult ChessLogic::AsyncCalculateAiComplexMove(ChessAiAsyncTurnKey, ChessPieceCoord& from, ChessPieceCoord& to, uint32_t& counter)
{
	uint32_t const depth = 3;
	bool const isMaximisingPlayer = true;

	ChessMoveStore storage;
	GenerateMoves(m_turn, storage);

	float bestBoardScore = -9999;
	std::optional<ChessMove> m_bestMove;

	for (ChessMove const& generatedMove : storage)
	{
		// no need to evaluate the board if the ai has checkmated the other player
		ChessMoveResult const result = FinaliseMove(generatedMove);
		if (result & OKMoveCheckmate)
		{
			from = generatedMove.GetFromCoord();
			to   = generatedMove.GetToCoord();
			return result;
		}

		float const boardScore = MinimaxAlphaBeta(depth - 1, -10000, 10000, !isMaximisingPlayer, counter);
		UndoMove(1);

		if (boardScore >= bestBoardScore)
		{
			m_bestMove.emplace(generatedMove);
			bestBoardScore = boardScore;
		}
	}

	// every generated move had the same board score, pick one at random
	// this shouldn't happen, just here to prevent crash
	if (!m_bestMove.has_value() && !storage.empty())
	{
		ChessMoveStore::iterator itr = std::begin(storage);
		std::advance(itr, getRandomNumber(storage.size() - 1));
		m_bestMove.emplace(*itr);
	}

	assert(m_bestMove.has_value());
	from = m_bestMove->GetFromCoord();
	to   = m_bestMove->GetToCoord();
	return FinaliseMove(*m_bestMove);
}

float ChessLogic::MinimaxAlphaBeta(uint8_t const depth, float alpha, float beta, bool const isMaximisingPlayer, uint32_t& counter)
{
	counter++;
	if (!depth)
		return -EvaluateBoard();

	ChessMoveStore storage;
	GenerateMoves(m_turn, storage);

	if (isMaximisingPlayer)
	{
		float bestBoardScore = -9999.f;
		for (ChessMove const& move : storage)
		{
			FinaliseMove(move);
			bestBoardScore = max(bestBoardScore, MinimaxAlphaBeta(depth - 1, alpha, beta, false, counter));
			UndoMove(1);

			alpha = max(alpha, bestBoardScore);
			if (beta <= alpha)
				return bestBoardScore;
		}
		return bestBoardScore;
	}
	else
	{
		float bestBoardScore = 9999.f;
		for (ChessMove const& move : storage)
		{
			FinaliseMove(move);
			bestBoardScore = max(bestBoardScore, MinimaxAlphaBeta(depth - 1, alpha, beta, true, counter));
			UndoMove(1);

			beta = max(beta, bestBoardScore);
			if (beta <= alpha)
				return bestBoardScore;
		}
		return bestBoardScore;
	}
}

float ChessLogic::EvaluateBoard() const
{
	float boardScore = 0.f;
	WalkPieces([&boardScore](BasePiece* piece)
	{
		// the knight and queen only have a single shared table
		ChessColour tableColour = piece->GetColour();
		if (piece->GetType() == Knight || piece->GetType() == Queen)
			tableColour = White;

		float value = 0.f;
		value += PieceSquareTable[piece->GetType()][tableColour][piece->GetCoord().GetOffset()];
		value += PieceWorth[piece->GetType()];

		boardScore += piece->GetColour() ? value : -value;
	});
	 
	return boardScore;
}

void ChessLogic::GenerateMoves(BasePiece* piece, bool const single, ChessMoveStore& storage) const
{
	ChessColour const colour = piece->GetColour();
	if (piece->GetType() == Pawn)
	{
		// single
		ChessPieceCoord const& from = piece->GetCoord();
		ChessPieceCoord to = from;
		to.MoveOffset(PawnOffsets[colour][0]);

		if (!GetPiece(to))
		{
			BuildMove(storage, ChessMoveFlagNormal, colour, piece->GetType(), from, to);

			// second
			to = from;
			to.MoveOffset(PawnOffsets[colour][1]);

			if (!GetPiece(to) && (colour ? Rank2 : Rank7) == from.GetRank())
				BuildMove(storage, ChessMoveFlagBigPawn, colour, piece->GetType(), from, to);
		}

		// capture
		for (uint8_t i = 2; i < 4; i++)
		{
			to = from;
			to.MoveOffset(PawnOffsets[colour][i]);
			if (!to.IsValid())
				continue;

			BasePiece* toPiece = GetPiece(to);
			if (toPiece && toPiece->GetColour() != colour)
				BuildMove(storage, ChessMoveFlagCapture, colour, piece->GetType(), from, to);
			else if (to == m_enPassantCoord)
				BuildMove(storage, ChessMoveFlagEnPassantCapture, colour, piece->GetType(), from, *m_enPassantCoord);
		}
	}
	else
	{
		auto const range = PieceOffsets.equal_range(piece->GetType());
		for (auto i = range.first; i != range.second; ++i)
		{
			ChessPieceCoord const& from = piece->GetCoord();
			ChessPieceCoord to = from;

			while (true)
			{
				to.MoveOffset((*i).second);
				if (!to.IsValid())
					break;

				if (BasePiece* toPiece = GetPiece(to))
				{
					if (toPiece->GetColour() != colour)
						BuildMove(storage, ChessMoveFlagCapture, colour, piece->GetType(), from, to);
					break;
				}

				BuildMove(storage, ChessMoveFlagNormal, colour, piece->GetType(), from, to);

				// Knights and Kings can't move more than once
				if (piece->GetType() == Knight || piece->GetType() == King)
					break;
			}
		}
	}

	// only check for castling during full board generation or for a single king
	if (!single || piece->GetType() == King)
	{
		if (m_castling[colour] & (ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle))
		{
			BasePiece* king = GetPiece(colour, King);
			ChessPieceCoord const& kingCoord = king->GetCoord();

			ChessColour const opColour = InverseColour(colour);

			if (m_castling[colour] & ChessMoveFlagKingSideCastle)
			{
				ChessPieceCoord castlingToK = kingCoord; // destination king
				castlingToK.MoveOffset(2, 0);
				ChessPieceCoord castlingToR = kingCoord; // destination rook
				castlingToR.MoveOffset(1, 0);

				if (!GetPiece(castlingToR)
					&& !GetPiece(castlingToK)
					&& !CanAttack(opColour, kingCoord)
					&& !CanAttack(opColour, castlingToR)
					&& !CanAttack(opColour, castlingToK))
					BuildMove(storage, ChessMoveFlagKingSideCastle, colour, King, kingCoord, castlingToK);
			}

			if (m_castling[colour] & ChessMoveFlagQueenSideCastle)
			{
				ChessPieceCoord castlingToK = kingCoord; // destination king
				castlingToK.MoveOffset(-2, 0);
				ChessPieceCoord castlingToR = kingCoord; // destination rook
				castlingToR.MoveOffset(-1, 0);
				ChessPieceCoord castlingToI = kingCoord; // intermediate
				castlingToI.MoveOffset(-3, 0);

				if (!GetPiece(castlingToR)
					&& !GetPiece(castlingToK)
					&& !GetPiece(castlingToI)
					&& !CanAttack(opColour, kingCoord)
					&& !CanAttack(opColour, castlingToR)
					&& !CanAttack(opColour, castlingToK))
					BuildMove(storage, ChessMoveFlagQueenSideCastle, colour, King, kingCoord, castlingToK);
			}
		}
	}
}

void ChessLogic::GenerateMoves(ChessColour const colour, ChessMoveStore& storage) const
{
	WalkPieces([this, colour, &storage](BasePiece* piece)
	{
		if (piece->GetColour() != colour)
			return;

		GenerateMoves(piece, false, storage);
	});
}

bool ChessLogic::CanAttack(ChessColour const attacker, ChessPieceCoord const& victim) const
{
	for (uint8_t x = 0; x < CHESS_BOARD_SIZE; x++)
	{
		for (uint8_t y = 0; y < CHESS_BOARD_SIZE; y++)
		{
			BasePiece* piece = m_board[(y * CHESS_BOARD_SIZE) + x];
			if (!piece)
				continue;

			if (piece->GetColour() != attacker)
				continue;

			if (piece->CanAttackRaycast(victim))
			{
				// the knight can jump over other pieces and the pawn can only attack a single space
				if (piece->GetType() == Knight || piece->GetType() == Pawn)
					return true;

				auto const range = PieceOffsets.equal_range(piece->GetType());
				for (auto i = range.first; i != range.second; ++i)
				{
					ChessPieceCoord const& from = piece->GetCoord();
					ChessPieceCoord to = from;

					while (true)
					{
						to.MoveOffset((*i).second);
						if (!to.IsValid())
							break;

						if (GetPiece(to))
						{
							if (to == victim)
								return true;
							break;
						}
					}
				}
			}   
		}
	}
		
	return false;
}

bool ChessLogic::InCheck() const
{
	BasePiece* king = GetPiece(m_turn, King);
	assert(king);
	return CanAttack(InverseColour(m_turn), king->GetCoord());
}

bool ChessLogic::InCheckmate() const
{
	ChessMoveStore storage;
	GenerateMoves(m_turn, storage);
	return InCheck() && storage.empty();
}

void ChessLogic::BuildMove(ChessMoveStore& storage, uint32_t result, ChessColour const colour,
	ChessPieceType const type, ChessPieceCoord const& from, ChessPieceCoord const& to) const
{
	BasePiece* fromPiece = GetPiece(from);
	BasePiece* toPiece   = GetPiece(to);

	// AC's Chess implementation doesn't support underpromotion
	ChessPieceType promotion = Empty;
	if (fromPiece->GetType() == Pawn
		&& (to.GetRank() == Rank8 || to.GetRank() == Rank1))
	{
		promotion = Queen;
		result |= ChessMoveFlagPromotion;
	}

	ChessPieceType captured = Empty;
	if (toPiece)
		captured = toPiece->GetType();
	else if (result & ChessMoveFlagEnPassantCapture)
		captured = Pawn;

	storage.emplace_back(static_cast<ChessMoveFlag>(result), colour, type, from, to, promotion,
		captured, m_move, m_halfMove, m_castling, m_enPassantCoord, fromPiece->GetGuid(),
		captured ? toPiece->GetGuid() : 0);
}

ChessMoveResult ChessLogic::FinaliseMove(ChessMove const& move)
{
	InternalMove(move);

	uint32_t result = move.GetFlags() & (ChessMoveFlagCapture | ChessMoveFlagEnPassantCapture) ? OKMoveToOccupiedSquare : OKMoveToEmptySquare;
	if (move.GetFlags() & ChessMoveFlagPromotion)
		result |= OKMovePromotion;

	// win conditions
	if (InCheck())
		result |= OKMoveCheck;
	if (InCheckmate())
		result |= OKMoveCheckmate;

	return static_cast<ChessMoveResult>(result);
}

void ChessLogic::InternalMove(ChessMove const& move)
{
	ChessMoveFlag const flags  = move.GetFlags();
	ChessPieceCoord const to   = move.GetToCoord();
	ChessPieceCoord const from = move.GetFromCoord();
	ChessColour const colour   = move.GetColour();
	ChessColour const opColour = InverseColour(colour);

	MovePiece(from, to);

	if (flags & ChessMoveFlagEnPassantCapture)
	{
		ChessPieceCoord enPassantCoord = to;
		enPassantCoord.MoveOffset(0, colour ? 2 : -2);
		RemovePiece(enPassantCoord);
	}

	if (flags & ChessMoveFlagPromotion)
	{
		BasePiece* pawnPiece = GetPiece(to);
		uint32_t const guid = pawnPiece->GetGuid();

		RemovePiece(pawnPiece);

		BasePiece* queenPiece = AddPiece(colour, Queen, to.GetX(), to.GetY());
		queenPiece->SetGuid(guid);
	}

	if (move.GetType() == King)
	{
		// if we castled, move the rook next to our king
		if (flags & (ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle))
		{
			ChessPieceCoord castlingTo = move.GetToCoord();
			ChessPieceCoord castlingFrom = castlingTo;
			
			if (flags & ChessMoveFlagKingSideCastle)
			{
				castlingTo.MoveOffset(-1, 0);
				castlingFrom.MoveOffset(1, 0);
			}
			if (flags & ChessMoveFlagQueenSideCastle)
			{
				castlingTo.MoveOffset(1, 0);
				castlingFrom.MoveOffset(-2, 0);
			}

			MovePiece(castlingFrom, castlingTo);
		}

		// turn off castling, our king has moved
		m_castling[colour] = 0;
	}

	std::function<void(ChessColour)> const doCastleCheck = [this, from](ChessColour const colour)
	{
		auto const range = RookFlags.equal_range(colour);
		for (auto i = range.first; i != range.second; ++i)
		{
			RookCastleFlag const rookCastleFlag = (*i).second;
			if (from.GetX() == rookCastleFlag.GetVector().first
				&& from.GetY() == rookCastleFlag.GetVector().second
				&& m_castling[colour] & rookCastleFlag.GetFlag())
			{
				m_castling[colour] &= ~rookCastleFlag.GetFlag();
				break;
			}
		}
	};

	// turn off castling if we have move one of our rooks
	if (m_castling[colour])
		doCastleCheck(colour);

	// turn off castling if we capture one of the opponents rooks
	if (m_castling[opColour])
		doCastleCheck(opColour);

	if (flags & ChessMoveFlagBigPawn)
	{
		ChessPieceCoord enPassantCoord = to;
		enPassantCoord.MoveOffset(0, colour ? 2 : -2);
		m_enPassantCoord = enPassantCoord;
	}
	else
		m_enPassantCoord.reset();

	m_history.push(move);

	if (colour == Black)
		m_move++;

	// reset 50 move rule counter if a pawn is moved or a piece is captured
	if (move.GetType() == Pawn
		|| flags & (ChessMoveFlagCapture | ChessMoveFlagEnPassantCapture))
		m_halfMove = 0;
	else
		m_halfMove++;

	m_turn = opColour;
}

void ChessLogic::UndoMove(uint32_t count)
{
	while (!m_history.empty() && count)
	{
		ChessMove const& move = m_history.top();

		// undo 'global' information
		m_turn           = InverseColour(move.GetColour());
		m_castling       = move.GetCastling();
		m_enPassantCoord = move.GetEnPassantCoord();
		m_halfMove       = move.GetHalfMove();
		m_move           = move.GetMove();

		MovePiece(move.GetToCoord(), move.GetFromCoord());

		ChessMoveFlag const flags = move.GetFlags();
		if (flags & ChessMoveFlagPromotion)
		{
			BasePiece* piece = AddPiece(m_turn, Pawn, move.GetToCoord());
			piece->SetGuid(move.GetGuid());
		}

		if (flags & ChessMoveFlagCapture)
		{
			BasePiece* piece = AddPiece(m_turn, move.GetCapture(), move.GetToCoord());
			piece->SetGuid(move.GetGuid());
		}

		if (flags & (ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle))
		{
			ChessPieceCoord castlingTo   = move.GetToCoord();
			ChessPieceCoord castlingFrom = castlingTo;

			if (flags & ChessMoveFlagKingSideCastle)
			{
				castlingTo.MoveOffset(1, 0);
				castlingFrom.MoveOffset(-1, 0);
			}
			if (flags & ChessMoveFlagQueenSideCastle)
			{
				castlingTo.MoveOffset(-2, 0);
				castlingFrom.MoveOffset(1, 0);
			}

			MovePiece(castlingTo, castlingFrom);
		}

		m_history.pop();
		count--;
	}
}

CPlayerWeenie* ChessSide::GetPlayer() const
{
	return g_pWorld->FindPlayer(m_guid);
}

ChessMatch::ChessMatch(CWeenieObject* game)
	: m_guid(game->GetID()), m_position(game->GetPosition()), m_state(), m_aiState(), m_moveResult(), m_waitingForWeenieMotion() { }

ChessMatch::~ChessMatch()
{
	for (ChessSide* side : m_side)
		delete side;
}

bool ChessMatch::IsInMatch(uint32_t const guid) const
{
	return GetColour(guid).has_value();
}

std::optional<ChessColour> ChessMatch::GetColour(uint32_t const guid) const
{
	for (ChessSide* side : m_side)
		if (side && side->GetGuid() == guid)
			return side->GetColour();
	return std::nullopt;
}

void ChessMatch::Update()
{
	switch (m_state)
	{
		case ChessStateInProgress:
		{
			switch (m_aiState)
			{
				case ChessAiStateWaitingToStart:
					StartAiMove();
					break;
				case ChessAiStateWaitingForFinish:
					FinishAIMove();
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	// don't handle any delayed actions while ai is working to prevent races
	if (m_aiState != ChessAiStateNone)
		return;

	// don't handle any delayed actions while weenie pieces are moving or attacking
	if (m_waitingForWeenieMotion)
		return;

	while (!m_actions.empty())
	{
		ChessDelayedAction action = m_actions.front();
		m_actions.pop();

		switch (action.GetAction())
		{
			case DelayedActionTypeStart:
				Start();
				break;
			case DelayedActionTypeMove:
				MoveDelayed(action);
				break;
			case DelayedActionTypeMovePass:
				MovePassDelayed(action);
				break;
			case DelayedActionTypeStalemate:
				StalemateDelayed(action);
				break;
			case DelayedActionTypeQuit:
				QuitDelayed(action.GetColour());
				break;
			default:
				break;
		}
	}

	if (m_nextRangeCheck)
	{
		time_point const now = steady_clock::now();
		if (m_nextRangeCheck <= now)
		{
			for (ChessSide* side : m_side)
			{
				if (!side)
					continue;

				if (side->IsAi())
					continue;

				CPlayerWeenie* player = side->GetPlayer();
				assert(player);

				// arbitrary distance, should there be some warning before reaching leash range? 
				float const distanceToGame = m_position.distance(player->GetPosition());
				if (abs(distanceToGame) > 40.f)
				{
					QuitDelayed(side->GetColour());
					return;
				}
			}

			m_nextRangeCheck = now + seconds(5);
		}
	}
}

void ChessMatch::AddSide(uint32_t guid, ChessColour const colour)
{
	assert(colour <= Black);
	m_side[colour] = new ChessSide(guid, colour);

	// spawn weenie pieces in the world for side
	m_logic.WalkPieces([this, colour](BasePiece* piece)
	{
		if (piece->GetColour() == colour)
			AddWeeniePiece(piece);
	});

	if (m_side[InverseColour(colour)])
		m_actions.emplace(DelayedActionTypeStart);
}

void ChessMatch::AddAi()
{
	if (m_state != ChessStateWaitingForPlayers)
		return;

	std::optional<ChessColour> const colour = GetFreeColour();
	if (!colour.has_value())
		return;

	AddSide(0, *colour);
}

void ChessMatch::Join(CPlayerWeenie* player)
{
	std::optional<ChessColour> colour = GetFreeColour();
	if (colour.has_value())
	{
		using namespace std::chrono;
		if (!m_nextRangeCheck)
			m_nextRangeCheck = steady_clock::now() + seconds(5);

		AddSide(player->GetID(), *colour);
	}

	SendJoinGameResponse(player, m_guid, colour);
}

void ChessMatch::MoveEnqueue(CPlayerWeenie* player, ChessPieceCoord const& from, ChessPieceCoord const& to)
{
	if (m_state != ChessStateInProgress)
		return;

	std::optional<ChessColour> colour = GetColour(player->GetID());
	assert(colour.has_value());

	m_actions.emplace(DelayedActionTypeMove, *colour, from, to);
}

void ChessMatch::MovePassEnqueue(CPlayerWeenie* player)
{
	if (m_state != ChessStateInProgress)
		return;

	std::optional<ChessColour> const colour = GetColour(player->GetID());
	assert(colour.has_value());

	m_actions.emplace(DelayedActionTypeMovePass, *colour);
}

void ChessMatch::QuitEnqueue(CPlayerWeenie* player)
{
	if (m_state != ChessStateWaitingForPlayers
		&& m_state != ChessStateInProgress)
		return;

	std::optional<ChessColour> const colour = GetColour(player->GetID());
	assert(colour.has_value());

	m_actions.emplace(DelayedActionTypeQuit, *colour);
}

void ChessMatch::StalemateEnqueue(CPlayerWeenie* player, bool const on)
{
	if (m_state != ChessStateInProgress)
		return;

	std::optional<ChessColour> const colour = GetColour(player->GetID());
	assert(colour.has_value());

	m_actions.emplace(DelayedActionTypeStalemate, *colour);
}

void ChessMatch::AsyncMoveAiSimple(ChessAiAsyncTurnKey, ChessAiMoveResult& result)
{
	assert(m_aiState == ChessAiStateWaitingForWorker);
	m_aiState = ChessAiStateInProgress;

	ChessPieceCoord from, to;
	ChessMoveResult const moveResult = m_logic.AsyncCalculateAiSimpleMove({}, from, to);
	result.SetResult(moveResult, from, to);

	m_aiState = ChessAiStateWaitingForFinish;
}

void ChessMatch::AsyncMoveAiComplex(ChessAiAsyncTurnKey, ChessAiMoveResult& result)
{
	assert(m_aiState == ChessAiStateWaitingForWorker);
	m_aiState = ChessAiStateInProgress;

	ChessPieceCoord from, to;
	uint32_t counter = 0;
	ChessMoveResult const moveResult = m_logic.AsyncCalculateAiComplexMove({}, from, to, counter);
	result.SetResult(moveResult, from, to);
	result.SetProfilingCounter(counter);

	m_aiState = ChessAiStateWaitingForFinish;
}

void ChessMatch::PieceReady(uint32_t const pieceGuid)
{
	if ((m_moveResult & OKMovePromotion) != 0)
	{
		BasePiece* piece = m_logic.GetPiece(pieceGuid);
		assert(piece);
		UpgradeWeeniePiece(piece);
	}

	m_weenieMotion.erase(pieceGuid);
	if (m_weenieMotion.empty())
	{
		m_waitingForWeenieMotion = false;
		FinishTurn();
	}
}

void ChessMatch::SendJoinGameResponse(CPlayerWeenie* player, uint32_t guid, std::optional<ChessColour> colour)
{
	BinaryWriter joinGameResponse;
	joinGameResponse.Write<uint32_t>(0x0281);
	joinGameResponse.Write<uint32_t>(guid);
	joinGameResponse.Write<int32_t>(colour.value_or(static_cast<ChessColour>(CHESS_COLOUR_INVALID)));
	player->SendNetMessage(joinGameResponse.GetData(), joinGameResponse.GetSize(), PRIVATE_MSG);
}

std::optional<ChessColour> ChessMatch::GetFreeColour() const
{
	for (uint8_t i = 0; i < CHESS_COLOUR_COUNT; i++)
		if (!m_side[i])
			return static_cast<ChessColour>(i);
	return std::nullopt;
}

void ChessMatch::Start()
{
	assert(m_state == ChessStateWaitingForPlayers);
	m_state = ChessStateInProgress;

	for (ChessSide* side : m_side)
	{
		if (side->IsAi())
			continue;

		CPlayerWeenie* player = side->GetPlayer();
		assert(player);
		SendStartGame(player, m_logic.GetTurn());
	}
}

void ChessMatch::Finish(int32_t const winner)
{
	assert(m_state == ChessStateWaitingForPlayers || m_state == ChessStateInProgress);

	for (ChessSide* side : m_side)
	{
		if (!side)
			continue;

		if (side->IsAi())
			continue;

		CPlayerWeenie* player = side->GetPlayer();
		assert(player);

		SendGameOver(player, winner);

		if (winner != CHESS_WINNER_END_GAME)
		{
			int32_t const total = player->m_Qualities.GetInt(CHESS_TOTALGAMES_INT, 0);
			player->m_Qualities.SetInt(CHESS_TOTALGAMES_INT, total + 1);
		}

		if (winner)
		{
			ChessColour const winnerColour = static_cast<ChessColour>(winner);
			if (winnerColour == side->GetColour())
			{
				int32_t const won = player->m_Qualities.GetInt(CHESS_GAMESWON_INT, 0);
				player->m_Qualities.SetInt(CHESS_GAMESLOST_INT, won + 1);
			}
			else
			{
				int32_t const lost = player->m_Qualities.GetInt(CHESS_GAMESLOST_INT, 0);
				player->m_Qualities.SetInt(CHESS_GAMESLOST_INT, lost + 1);
			}
		}
	}

	while (!m_actions.empty())
		m_actions.pop();

	m_logic.WalkPieces([this](BasePiece* piece)
	{
		RemoveWeeniePiece(piece);
	});

	m_state = ChessStateFinished;
	m_nextRangeCheck.reset();
}

void ChessMatch::FinishTurn()
{
	if (ChessSide* side = m_side[InverseColour(m_logic.GetTurn())])
		if (!side->IsAi())
			SendMoveResponse(side->GetPlayer(), m_moveResult);

	if (ChessSide* side = m_side[m_logic.GetTurn()])
	{
		if (side->IsAi())
			m_aiState = ChessAiStateWaitingToStart;
		else
		{
			ChessMove const& move = m_logic.GetLastMove();
			GameMoveData data(MoveTypeFromTo, move.GetColour(), move.GetFromCoord(), move.GetToCoord());
			SendOpponentTurn(side->GetPlayer(), move.GetColour(), data);
		}
	}
}

void ChessMatch::StartAiMove()
{
	assert(m_aiState == ChessAiStateWaitingToStart);
	m_aiState = ChessAiStateWaitingForWorker;

	// execute ai work on a seperate thread
	ChessAiMove aiMove;
	m_aiFuture = std::async(std::launch::async, aiMove, this);
}

void ChessMatch::FinishAIMove()
{
	assert(m_aiState == ChessAiStateWaitingForFinish);
	m_aiState = ChessAiStateNone;

	ChessAiMoveResult result = m_aiFuture.get();
	m_moveResult = result.GetResult();
	FinaliseWeenieMove(result.GetResult());

	LOG_PRIVATE(Data, Normal, "Calculated Chess AI move in %u ms with %u minimax calculations.\n",
		result.GetProfilingTime(), result.GetProfilingCounter());
}

void ChessMatch::FinaliseWeenieMove(ChessMoveResult const result)
{
	ChessMove const& move = m_logic.GetLastMove();

	// need to use destination coordinate as m_logic has already moved the piece
	BasePiece* piece = m_logic.GetPiece(move.GetToCoord());

	if ((result & OKMoveToEmptySquare) != 0)
	{
		MoveWeeniePiece(piece);

		ChessMoveFlag const flags = move.GetFlags();
		if (flags & (ChessMoveFlagKingSideCastle | ChessMoveFlagQueenSideCastle))
		{
			ChessPieceCoord castlingTo = move.GetToCoord();
			if (flags & ChessMoveFlagKingSideCastle)
				castlingTo.MoveOffset(-1, 0);
			if (flags & ChessMoveFlagQueenSideCastle)
				castlingTo.MoveOffset(1, 0);

			BasePiece* rookPiece = m_logic.GetPiece(castlingTo);
			assert(rookPiece);

			MoveWeeniePiece(rookPiece);
		}
	}
	else if ((result & OKMoveToOccupiedSquare) != 0)
		AttackWeeniePiece(piece, move.GetCapturedGuid());
}

void ChessMatch::MoveDelayed(ChessDelayedAction const& action)
{
	if (m_logic.GetTurn() != action.GetColour())
		return;

	CPlayerWeenie* player = m_side[action.GetColour()]->GetPlayer();
	assert(player);

	ChessMoveResult const result = m_logic.Move(action.GetColour(), action.GetFromCoord(), action.GetToCoord());
	if (result < NoMoveResult)
	{
		SendMoveResponse(player, result);
		return;
	}

	m_moveResult = result;
	FinaliseWeenieMove(result);
}

void ChessMatch::MovePassDelayed(ChessDelayedAction const& action)
{
}

void ChessMatch::QuitDelayed(ChessColour const colour)
{
	switch (m_state)
	{
		case ChessStateWaitingForPlayers:
			Finish(CHESS_WINNER_END_GAME);
			break;
		case ChessStateInProgress:
			Finish(InverseColour(colour));
			break;
		default:
			break;
	}
}

void ChessMatch::StalemateDelayed(ChessDelayedAction const& action)
{
	ChessSide* side = m_side[action.GetColour()];
	ChessSide const* opSide = m_side[InverseColour(action.GetColour())];

	side->SetStalemate(action.GetStalemate());

	if (action.GetStalemate() && opSide->GetStalemate())
		Finish(CHESS_WINNER_STALEMATE);
	else if (!opSide->IsAi())
		SendOpponentStalemateState(opSide->GetPlayer(), side->GetColour(), action.GetStalemate());
}

void ChessMatch::CalculateWeeniePosition(ChessPieceCoord const& coord, ChessColour colour, Position& position) const
{
	uint32_t heading = static_cast<uint32_t>(m_position.frame.get_heading());
	heading += colour ? 180 : 0;
	heading = heading % 360;

	position.frame.m_origin += Vector(-3.5f + (coord.GetX() * 1.f), -3.5f + (coord.GetY() * 1.f), 0.f);
	position.frame.set_heading(heading);
}

void ChessMatch::AddWeeniePiece(BasePiece* piece) const
{
	Position weeniePosition = m_position;
	CalculateWeeniePosition(piece->GetCoord(), piece->GetColour(), weeniePosition);

	CWeenieObject* weeniePiece = nullptr;
	switch (piece->GetType())
	{
		case Pawn:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgepawn" : "mosswartpawn", &weeniePosition, true);
			break;
		case Rook:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgerook" : "mosswartrook", &weeniePosition, true);
			break;
		case Knight:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgeknight" : "mosswartknight", &weeniePosition, true);
			break;
		case Bishop:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgebishop" : "mosswartbishop", &weeniePosition, true);
			break;
		case Queen:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgequeen" : "mosswartqueen", &weeniePosition, true);
			break;
		case King:
			weeniePiece = g_pWeenieFactory->CreateWeenieByName(piece->GetColour() ? "drudgeking" : "mosswartking", &weeniePosition, true);
			break;
		default:
			break;
	}

	assert(weeniePiece);

	weeniePiece->AsGamePiece()->SetGuid(m_guid);
	piece->SetGuid(weeniePiece->GetID());
}

void ChessMatch::MoveWeeniePiece(BasePiece* piece)
{
	CWeenieObject* gamePiece = g_pWorld->FindObject(piece->GetGuid());
	assert(gamePiece);

	Position weeniePosition = m_position;
	CalculateWeeniePosition(piece->GetCoord(), piece->GetColour(), weeniePosition);
	gamePiece->AsGamePiece()->MoveEnqueue(weeniePosition);

	AddPendingWeenieMotion(piece);
}

void ChessMatch::AttackWeeniePiece(BasePiece* piece, uint32_t const victim)
{
	CWeenieObject* gamePiece = g_pWorld->FindObject(piece->GetGuid());
	assert(gamePiece);

	Position weeniePosition = m_position;
	CalculateWeeniePosition(piece->GetCoord(), piece->GetColour(), weeniePosition);

	gamePiece->AsGamePiece()->AttackEnqueue(weeniePosition, victim);

	AddPendingWeenieMotion(piece);
}

void ChessMatch::RemoveWeeniePiece(BasePiece* piece) const
{
	uint32_t const guid = piece->GetGuid();
	if (!guid)
		return;

	CWeenieObject* gamePiece = g_pWorld->FindObject(guid);
	assert(gamePiece);

	piece->SetGuid(0);
	gamePiece->Remove();
}

void ChessMatch::UpgradeWeeniePiece(BasePiece* piece) const
{
	RemoveWeeniePiece(piece);

	// AC's Chess implementation doesn't support underpromotion
	piece->SetType(Queen);
	AddWeeniePiece(piece);
}

void ChessMatch::AddPendingWeenieMotion(BasePiece* piece)
{
	m_weenieMotion.insert(piece->GetGuid());
	m_waitingForWeenieMotion = true;
}

void ChessMatch::SendStartGame(CPlayerWeenie* player, ChessColour colour) const
{
	BinaryWriter startGame;
	startGame.Write<uint32_t>(0x0282);
	startGame.Write<uint32_t>(m_guid);
	startGame.Write<uint32_t>(colour);
	player->SendNetMessage(startGame.GetData(), startGame.GetSize(), PRIVATE_MSG);
}

void ChessMatch::SendMoveResponse(CPlayerWeenie* player, ChessMoveResult const result) const
{
	BinaryWriter moveResponse;
	moveResponse.Write<uint32_t>(0x0283);
	moveResponse.Write<uint32_t>(m_guid);
	moveResponse.Write<int32_t>(result);
	player->SendNetMessage(moveResponse.GetData(), moveResponse.GetSize(), PRIVATE_MSG);
}

void ChessMatch::SendOpponentTurn(CPlayerWeenie* player, ChessColour const colour, GameMoveData& move) const
{
	BinaryWriter opponentTurn;
	opponentTurn.Write<uint32_t>(0x0284);
	opponentTurn.Write<uint32_t>(m_guid);
	opponentTurn.Write<uint32_t>(colour);
	opponentTurn.Write(&move);
	player->SendNetMessage(opponentTurn.GetData(), opponentTurn.GetSize(), PRIVATE_MSG);
}

void ChessMatch::SendOpponentStalemateState(CPlayerWeenie* player, ChessColour const colour, bool const on) const
{
	BinaryWriter opponentStalemate;
	opponentStalemate.Write<uint32_t>(0x0285);
	opponentStalemate.Write<uint32_t>(m_guid);
	opponentStalemate.Write<uint32_t>(colour);
	opponentStalemate.Write<uint32_t>(on);
	player->SendNetMessage(opponentStalemate.GetData(), opponentStalemate.GetSize(), PRIVATE_MSG);
}

void ChessMatch::SendGameOver(CPlayerWeenie* player, int32_t winner) const
{
	BinaryWriter gameOver;
	gameOver.Write<uint32_t>(0x028C);
	gameOver.Write<uint32_t>(m_guid);
	gameOver.Write<int32_t>(winner);
	player->SendNetMessage(gameOver.GetData(), gameOver.GetSize(), PRIVATE_MSG);
}

} // GDLE::Chess
