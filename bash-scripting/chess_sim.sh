#!/bin/bash


# Validate that exactly one argument is provided
validate_arguments() {
    if test $# -ne 1; then
        echo "Usage: $0 <pgn_file>"
        exit 1
    fi
}

# Check if the provided file exists
check_file_exists() {
    if ! test -f "$1"; then
        echo "File does not exist: $1"
        exit 1
    fi
}

# Print metadata lines (those starting with [) from the PGN file
show_metadata() {
    echo "Metadata from PGN file:"
    grep '^\[' "$pgn_file"
}


# Initialize the chess board to the standard starting position
reset_board() {
    board=(
        r n b q k b n r  # rank 8 (Black back row)
        p p p p p p p p  # rank 7 (Black pawns)
        . . . . . . . .  # rank 6
        . . . . . . . .  # rank 5
        . . . . . . . .  # rank 4
        . . . . . . . .  # rank 3
        P P P P P P P P  # rank 2 (White pawns)
        R N B Q K B N R  # rank 1 (White back row)
    )
}

# Print the current board state
print_board() {
    echo "  a b c d e f g h"
    # Loop through each row and column
    for ((i=0; i<8; i++)); do
        echo -n "$((8 - i)) "
        for ((j=0; j<8; j++)); do
            index=$((i * 8 + j))
            echo -n "${board[$index]} "
        done
        echo "$((8 - i))"
    done
    echo "  a b c d e f g h"
}

# Convert str to board index 
str_to_index() {
    # Extract col_letter and row_number
    local col_letter=${1:0:1}
    local row_number=${1:1:1}
    # Convert col_letter to column number
    local col_index=$(( $(printf "%d" "'$col_letter") - 97 ))
    # Convert row_number to row number
    local row_index=$((8 - row_number))
    echo $((row_index * 8 + col_index))
}

# Apply UCI move to board
apply_move() {
    # Extract source and destination squares and promotion piece
    from_square=${1:0:2}
    to_square=${1:2:2}
    promotion_piece=${1:4:1}

    # Convert squares to indices
    from_index=$(str_to_index "$from_square")
    to_index=$(str_to_index "$to_square")
    piece="${board[$from_index]}"
    # Calculate horizontal movement for en passant
    diff=$(( to_index % 8 - from_index % 8 ))
    
    # Check for promotion
    if test ${#1} -eq 5; then
        promotion_case
        return
    # Check for white castling
    elif [[ "$piece" == "K" && "$from_square" == "e1" && ( "$to_square" == "g1" || "$to_square" == "c1" ) ]]; then
        castling_case
        return
    # Check for black castling
    elif [[ "$piece" == "k" && "$from_square" == "e8" && ( "$to_square" == "g8" || "$to_square" == "c8" ) ]]; then
        castling_case
        return
    # Check for en passant capture
    elif (test "$piece" = "P" -o "$piece" = "p") && test "${board[$to_index]}" = "." && test "$diff" -ne 0; then
        en_passant_case
        return
    fi

    # Standard move
    board[$to_index]="$piece"
    board[$from_index]="."
}

# Handle pawn promotion
promotion_case() {
    if test "$piece" != "${piece,,}"; then
        board[$to_index]=$(echo "$promotion_piece" | tr 'a-z' 'A-Z')  # White promotes
    else
        board[$to_index]="$promotion_piece"  # Black promotes
    fi
    board[$from_index]="."
}

# Handle castling move for both white and black
castling_case() {
    # Move the king
    board[$(str_to_index "$to_square")]="$piece"
    board[$(str_to_index "$from_square")]="."
    
    # White castling
    if test "$piece" = "K"; then
        rook_from="h1"; rook_to="f1"
        test "$to_square" = "c1" && rook_from="a1" && rook_to="d1"
        board[$(str_to_index "$rook_to")]="R"
        board[$(str_to_index "$rook_from")]="."
    # Black castling
    elif test "$piece" = "k"; then
        rook_from="h8"; rook_to="f8"
        test "$to_square" = "c8" && rook_from="a8" && rook_to="d8"
        board[$(str_to_index "$rook_to")]="r"
        board[$(str_to_index "$rook_from")]="."
    fi
}

# Handle en passant captures
en_passant_case() {
    if test "$piece" = "P"; then
        captured_index=$((to_index + 8))
    else
        captured_index=$((to_index - 8))
    fi
    board[$captured_index]="."
}

# Load the board state from saved_boards at current_move
load_board() {
    IFS=' ' read -r -a board <<< "${saved_boards[$current_move]}"
}

# Parse PGN file to extract UCI moves and reset board
parse_moves() {
    # Extract all non-metadata lines and convert to single line
    pgn_moves=$(grep -v '^\[' "$pgn_file" | tr '\n' ' ')
    # Call Python script to get UCI moves
    uci_moves_str=$(python3 parse_moves.py "$pgn_moves")
    # Split into array
    IFS=' ' read -r -a moves <<< "$uci_moves_str"
    total_moves=${#moves[@]}
    current_move=0
    saved_boards=()
    # Reset board and save initial state
    reset_board
    save_board
}

# Move one step forward in the game
move_forward() {
    apply_move "${moves[$current_move]}"
    current_move=$((current_move + 1))
    save_board
}

# Move one step back in the game
move_backward() {
    current_move=$((current_move - 1))
    load_board
}

# Play the game until the end
skip_to_end(){
    reset_board
    current_move=0
    saved_boards=()
    save_board
    while test $current_move -lt $total_moves; do
        move_forward
    done
}

# Save the current board state into saved_boards
save_board() {
    saved_boards[$current_move]="${board[*]}"
}

# Read and process a user key input
user_input() {
    echo -n "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit: "
    read -r key

    case "$key" in
        d)
            if test $current_move -lt $total_moves; then
                move_forward
            else
                echo "No more moves available."
                return
            fi
            ;;
        a)
            if test $current_move -gt 0; then
                move_backward
            fi
            ;;
        w)
            current_move=0
            load_board
            ;;
        s)
            if test $current_move -lt $total_moves; then
                skip_to_end
            fi
            ;;
        q)
            echo "Exiting."
            echo "End of game."
            exit 0
            ;;
        *)
            echo "Invalid key pressed: $key"
            return
            ;;
    esac
    echo "Move $current_move/$total_moves"
    print_board
}

# Main function to run the simulator
main() {
    # Call check functions
    validate_arguments "$@"
    check_file_exists "$1"
    pgn_file="$1"
    # Start the game
    show_metadata
    parse_moves
    
    echo "Move $current_move/$total_moves"
    print_board

    while true; do
        user_input
    done
}

main "$@"
