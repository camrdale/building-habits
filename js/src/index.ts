import { ChessBoardElement, validSquare } from 'chessboard-element';

const board = document.querySelector<ChessBoardElement>('chess-board');
if (!board) {
  throw new Error('Failed to find <chess-board>');
}

function isCustomEvent(event: Event): event is CustomEvent {
  return 'detail' in event;
}

interface GameState {
  fen: string;
  turn: string;
  legal: { [key: string]: string[] };
  in_check: boolean;
  in_checkmate: boolean;
  in_draw: boolean;
}

//board.setPosition(fen, false);
let state: GameState = {
  fen: new URLSearchParams(window.location.search).get('fen') || 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1',
  turn: 'w',
  legal: {},
  in_check: false,
  in_checkmate: false,
  in_draw: false
};
let pgn = '';

const highlightStyles = document.createElement('style');
document.head.append(highlightStyles);
const whiteSquareGrey = '#a9a9a9';
const blackSquareGrey = '#696969';

function removeGreySquares() {
  highlightStyles.textContent = '';
}

function greySquare(square: string) {
  const highlightColor = (square.charCodeAt(0) % 2) ^ (square.charCodeAt(1) % 2)
    ? whiteSquareGrey
    : blackSquareGrey;

  highlightStyles.textContent += `
    chess-board::part(${square}) {
      background-color: ${highlightColor};
    }
  `;
}

board.addEventListener('drag-start', (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  const { piece } = e.detail;

  // do not pick up pieces if the game is over
  if (state.in_checkmate || state.in_draw) {
    e.preventDefault();
    return;
  }

  // only pick up pieces for the side to move
  if ((state.turn === 'w' && piece.search(/^b/) !== -1) ||
    (state.turn === 'b' && piece.search(/^w/) !== -1)) {
    e.preventDefault();
    return;
  }
});

board.addEventListener('drop', async (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  const { source, target, piece, setAction } = e.detail;

  removeGreySquares();

  if (!validSquare(target)) {
    setAction('snapback');
    return;
  }

  // Check for illegal moves
  if (!(source in state.legal && state.legal[source].includes(target))) {
    setAction('snapback');
    return;
  }

  let move = source + target;
  if ((piece === 'wP' && target.charAt(0) === 'h') || (piece === 'bP' && target.charAt(0) === 'a')) {
    // Always promote to queen for now.
    move += 'q';
  }

  const params = new URLSearchParams({ fen: state.fen });
  try {
    const response = await fetch('http://localhost:8080/engine/move/' + move + '?' + params.toString());
    if (response.status !== 200) {
      console.log(response);
      setAction('snapback');
      return;
    }
    const json = await response.json();
    console.log(json);
    state = json;

    board.setPosition(state.fen);
    if (pgn != '') {
      pgn += ' ';
    }
    pgn += move;
    updateStatus();
  } catch (e) {
    console.log(e);
    setAction('snapback');
    return;
  }
});

board.addEventListener('mouseover-square', (e) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  const { square } = e.detail;

  if (state.in_checkmate || state.in_draw) {
    return;
  }

  // get list of possible moves for this square
  const moves = state.legal[square];

  // exit if there are no moves available for this square
  if (!moves || moves.length === 0) {
    return;
  }

  // highlight the square they moused over
  greySquare(square);

  // highlight the possible squares for this piece
  for (const move of moves) {
    greySquare(move);
  }
});

board.addEventListener('mouseout-square', (_e: Event) => {
  removeGreySquares();
});

async function initializeBoard() {
  const params = new URLSearchParams({ fen: state.fen });
  try {
    const response = await fetch('http://localhost:8080/engine/newgame?' + params.toString());
    if (response.status === 200) {
      const json = await response.json();
      console.log(json);
      state = json;
      board!.setPosition(state.fen, false);
      updateStatus();
    } else {
      console.log(response);
    }
  } catch (e) {
    console.log(e);
  }
}

// update the board position after the piece snap
// for castling, en passant, pawn promotion
board.addEventListener('snap-end', (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  board.setPosition(state.fen);
});

function updateStatus() {
  let status = '';

  let moveColor = 'White';
  if (state.turn === 'b') {
    moveColor = 'Black';
  }

  if (state.in_checkmate) {
    // checkmate?
    status = `Game over, ${moveColor} is in checkmate.`;
  } else if (state.in_draw) {
    // draw?
    status = 'Game over, drawn position';
  } else {
    // game still on
    status = `${moveColor} to move`;

    // check?
    if (state.in_check) {
      status += `, ${moveColor} is in check`;
    }
  }

  document.querySelector('#status')!.innerHTML = status;
  document.querySelector('#fen')!.innerHTML = state.fen || '';
  document.querySelector('#pgn')!.innerHTML = pgn;
}

initializeBoard();
