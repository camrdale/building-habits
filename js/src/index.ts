import { ChessBoardElement, validSquare } from 'chessboard-element';

const board = document.querySelector<ChessBoardElement>('chess-board');
if (!board) {
  throw new Error('Failed to find <chess-board>');
}

const startpos = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1';

function isCustomEvent(event: Event): event is CustomEvent {
  return 'detail' in event;
}

interface GameState {
  fen: string;
  last_move: string;
  turn: string;
  legal: { [key: string]: string[] };
  control: { [key: string]: number };
  in_check: boolean;
  in_checkmate: boolean;
  in_draw: boolean;
}

const queryParams = new URLSearchParams(window.location.search);
const showControl = queryParams.has('control');
const flip = queryParams.has('flip');
const engine = flip ? 'w' : 'b';

let state: GameState = {
  fen: queryParams.get('fen') || startpos,
  last_move: '',
  turn: 'w',
  legal: {},
  control: {},
  in_check: false,
  in_checkmate: false,
  in_draw: false
};
let pgn = '';

const controlStyles = document.createElement('style');
document.head.append(controlStyles);
const highlightStyles = document.createElement('style');
document.head.append(highlightStyles);
const whiteSquareGrey = '#a9a9a9';
const blackSquareGrey = '#696969';
const lastMoveYellow = 'rgba(255, 255, 51, 0.5)';
const inCheckRed = '#f25c61';

function initialHighlightStyles() {
  let initial = '';
  if (state.last_move) {
    let startingSquare = state.last_move.substring(0, 2);
    let endingSquare = state.last_move.substring(2, 4);
    initial += `
      chess-board::part(${startingSquare}) {
        background-color: ${lastMoveYellow};
      }
      chess-board::part(${endingSquare}) {
        background-color: ${lastMoveYellow};
      }
    `;
  }
  if (state.in_check && board?.position) {
    let piece = state.turn + 'K';
    let square = Object.keys(board.position).find(square => board.position[square] === piece);
    initial += `
      chess-board::part(${square}) {
        background-color: ${inCheckRed};
      }
    `;
  }
  return initial;
}

function removeGreySquares() {
  highlightStyles.textContent = initialHighlightStyles();
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

  // only pick up pieces when it is their turn to move
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
    updateState(json);
  } catch (e) {
    console.log(e);
    setAction('snapback');
    return;
  }

  if (state.turn == engine && !state.in_checkmate && !state.in_draw) {
    // Make a move from the engine.
    nextMove();
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

  // highlight the possible squares for this piece
  for (const move of moves) {
    greySquare(move);
  }
});

board.addEventListener('mouseout-square', (_e: Event) => {
  removeGreySquares();
});

// update the board position after the piece snap
// for castling, en passant, pawn promotion
board.addEventListener('snap-end', (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  board.setPosition(state.fen);
});

async function initializeBoard() {
  const params = new URLSearchParams({ fen: state.fen });
  try {
    const response = await fetch('http://localhost:8080/engine/newgame?' + params.toString());
    if (response.status === 200) {
      const json = await response.json();
      console.log(json);
      updateState(json);
    } else {
      console.log(response);
    }
  } catch (e) {
    console.log(e);
  }

  if (board && flip) {
    board.orientation = 'black';
  }

  if (state.turn == engine && !state.in_checkmate && !state.in_draw) {
    // Make a move from the engine.
    nextMove();
  }
}

async function nextMove() {
  const params = new URLSearchParams({ fen: state.fen });
  try {
    const response = await fetch('http://localhost:8080/engine/search' + '?' + params.toString());
    if (response.status !== 200) {
      console.log(response);
      return;
    }
    const json = await response.json();
    console.log(json);
    updateState(json);
  } catch (e) {
    console.log(e);
    return;
  }
}

function updateState(newState: GameState) {
  state = newState;
  board!.setPosition(state.fen, false);

  highlightStyles.textContent = initialHighlightStyles();
  controlStyles.textContent = '';
  if (showControl) {
    for (const [square, control] of Object.entries(state.control)) {
      if (control == 0) { continue; }
      let redColor = '255';
      let blueColor = '0';
      if (control > 0) {
        redColor = '0';
        blueColor = '255';
      }
      const opacity = Math.abs(control) / 10.0;

      controlStyles.textContent += `
      chess-board::part(${square}) {
        background-color: rgba(${redColor}, 0, ${blueColor}, ${opacity});
      }
    `;
    }
  }

  if (pgn != '') {
    pgn += ' ';
  }
  pgn += state.last_move;

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
  document.querySelector('#fen')!.innerHTML = state.fen;
  document.querySelector('#pgn')!.innerHTML = pgn;
}

initializeBoard();
