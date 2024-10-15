#!/bin/bash

# Function that generates the completions
_image_processing_completions() {
    local cur prev opts algorithms directions

    # Current word the user is trying to complete
    cur="${COMP_WORDS[COMP_CWORD]}"
    
    # Previous word typed
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # Options available for the user
    opts="-i --input -o --output -a --algo -s --strength --sr --sigma_range --sp --sigma_space -d --direction -h --help"

    # Available algorithms
    algorithms="gaussian box bilateral median motion"

    # Available directions for motion blur
    directions="horizontal vertical diagonal"

    # Completing options after the command
    if [[ ${cur} == -* ]] ; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
    fi

    # Completing the algorithms after -a or --algo
    if [[ ${prev} == "-a" || ${prev} == "--algo" ]] ; then
        COMPREPLY=( $(compgen -W "${algorithms}" -- ${cur}) )
        return 0
    fi

    # Completing the directions after -d or --direction
    if [[ ${prev} == "-d" || ${prev} == "--direction" ]] ; then
        COMPREPLY=( $(compgen -W "${directions}" -- ${cur}) )
        return 0
    fi
}

# Registering the completion function for the program (replace 'image_processing' with your actual program name)
complete -F _image_processing_completions image_processing
