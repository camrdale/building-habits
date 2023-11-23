import { ChessBoardElement, validSquare } from 'chessboard-element';

const board = document.querySelector<ChessBoardElement>('chess-board');
if (!board) {
  throw new Error('Failed to find <chess-board>');
}

function isCustomEvent(event: Event): event is CustomEvent {
  return 'detail' in event;
}

interface Moves {
  [key: string]: string[];
}

let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
let turn = 'w';
let pgn = '';
let game_over = false;
let in_checkmate = false;
let in_draw = false;
let in_check = false;
let legal_moves: Moves = {};

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
  // const {source, piece, position, orientation} = e.detail;

  // do not pick up pieces if the game is over
  if (game_over) {
    e.preventDefault();
    return;
  }

  // only pick up pieces for the side to move
  if ((turn === 'w' && piece.search(/^b/) !== -1) ||
    (turn === 'b' && piece.search(/^w/) !== -1)) {
    e.preventDefault();
    return;
  }
});

board.addEventListener('drop', async (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  const { source, target, setAction } = e.detail;

  removeGreySquares();

  if (!validSquare(target)) {
    setAction('snapback');
    return;
  }

  // Check for illegal moves
  if (!(source in legal_moves && legal_moves[source].includes(target))) {
    setAction('snapback');
    return;
  }

  const params = new URLSearchParams({ fen: fen });
  try {
    const response = await fetch('http://localhost:8080/engine/move/' + source + target + 'q?' + params.toString());
    if (response.status !== 200) {
      console.log(response);
      setAction('snapback');
      return;
    }
    const json = await response.json();
    legal_moves = json["legal"];
    fen = json["fen"];
    board.setPosition(fen);
  } catch (e) {
    console.log(e);
    setAction('snapback');
    return;
  }

  if (pgn != '') {
    pgn += ' ';
  }
  pgn += source + target;

  if (turn === 'w') {
    turn = 'b';
  } else {
    turn = 'w';
  }

  updateStatus(fen);
});

board.addEventListener('mouseover-square', (e) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  const { square } = e.detail;

  // get list of possible moves for this square
  const moves = legal_moves[square];

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

// board.addEventListener('change', (e: Event) => {
//   if (!isCustomEvent(e))
//     throw new Error('not a custom event');

//   const { value } = e.detail;

//   legalMoves(objToFen(value) || "");
//   updateStatus(objToFen(value) || "");
// });

function legalMoves(fen: string) {
  fen += " " + turn + " KQkq - 0 1";
  const params = new URLSearchParams({ fen: fen });
  try {
    fetch('http://localhost:8080/engine/legal?' + params.toString())
      .then(response => {
        if (response.status === 200) {
          response.json().then((json: any) => legal_moves = json["legal"]).catch(e => console.log(e));
        } else {
          console.log(response);
        }
      })
      .catch(e => console.log(e));
  } catch (e) {
    console.log(e);
  }
}

// update the board position after the piece snap
// for castling, en passant, pawn promotion
board.addEventListener('snap-end', (e: Event) => {
  if (!isCustomEvent(e))
    throw new Error('not a custom event');

  board.setPosition(fen);
});

function updateStatus(fen: string) {
  let status = '';

  let moveColor = 'White';
  if (turn === 'b') {
    moveColor = 'Black';
  }

  if (in_checkmate) {
    // checkmate?
    status = `Game over, ${moveColor} is in checkmate.`;
  } else if (in_draw) {
    // draw?
    status = 'Game over, drawn position';
  } else {
    // game still on
    status = `${moveColor} to move`;

    // check?
    if (in_check) {
      status += `, ${moveColor} is in check`;
    }
  }

  document.querySelector('#status')!.innerHTML = status;
  document.querySelector('#fen')!.innerHTML = fen || '';
  document.querySelector('#pgn')!.innerHTML = pgn;
}

legalMoves(fen);
updateStatus(fen);
