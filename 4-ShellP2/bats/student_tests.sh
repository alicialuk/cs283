#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Basic pipe"{
    run "./dsh" <<EOF
ls | grep ".c"
EOF
    
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    
    [ "$status" -eq 0 ]
    [[ "$output" == *"dsh_cli.c"* ]]
    [[ "$output" == *"dshlib.c"* ]]
}
