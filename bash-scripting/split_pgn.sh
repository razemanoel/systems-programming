#!/bin/bash


# Validate that exactly two arguments are provided
validate_arguments() {
    if test "$#" -ne 2; then
        echo "Usage: $0 <source_pgn_file> <destination_directory>"
        exit 1
    fi
}

# Check if the provided file exists
check_file_exists() {
    if test ! -f "$1"; then
        echo "Error: File '$1' does not exist."
        exit 1
    fi
}

# Directory Creation if necessary
check_directory_exists() {
    if test ! -d "$1"; then
        mkdir -p "$1"
        echo "Created directory '$1'."
    fi
}

# Save a game to file from game_block
save_file() {
    split_count=$((split_count + 1))
    output_file="$dest_dir/${file_name}_${split_count}.pgn"
    # Write the game_block content to the output file and print
    echo -e "$game_block" > "$output_file"
    echo "Saved game to $output_file"
    game_block=""
}

main() {
    # Extract input
    input_file="$1"
    dest_dir="$2"
    # Call check functions
    validate_arguments "$@"
    check_file_exists "$input_file"
    check_directory_exists "$dest_dir"

    # Extract file name
    file_name=$(basename "$input_file" .pgn)
    # Init to count how many splits done, and game_block
    split_count=0
    game_block=""

    while IFS= read -r line || test -n "$line"; do
        # Check if line starts with '[Event "' and game_block is not empty
        if echo "$line" | grep -q '^\[Event "' && test -n "$game_block"; then
            save_file
        fi
        # Add the line to the game_block
        game_block+="$line"$'\n'
    done < "$input_file"

    # Save the last game if game_block contains content
    if test -n "$game_block"; then
        save_file
    fi

    echo "All games have been split and saved to '$dest_dir'."

}
main "$@"



